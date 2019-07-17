/*
 * Yeseul An
 * CSS 432
 * Assignment 3
 * The program that implements algorithms such as stop-and-wait algorithm and sliding window algorithm
 *  which are used in the hw2.cpp main function
 */

#define TIME_OUT 1500                // set timeout 1500 usec
#include <vector>
#include "UdpSocket.h"
#include "Timer.h"

using namespace std;

int clientStopWait(UdpSocket &sock, const int max, int message[]);
void serverReliable(UdpSocket &sock, const int max, int message[]);
int clientSlidingWindow(UdpSocket &sock, const int max, int message[], int windowSize);
void serverEarlyRetrans(UdpSocket &sock, const int max, int message[], int windowSize);

/*
 * clientStopWait
 * The parameter sock is a UdpSocket object, max is maximum number of messages,
 * message is the message sent from client to server
 * Returns the number of retransmissions to the main program
 */
int clientStopWait(UdpSocket &sock, const int max, int *message) {
    cerr << "client: stop and wait test:" << endl;
    int retransmit = 0;

    for (int i = 0; i < max; i++) {
        message[0] = i;                            // message[0] has a sequence #
        sock.sendTo( ( char * )message, MSGSIZE ); // udp message send to server

        Timer timer;        // after sending, client starts the timer
        timer.start();

        // If there is no ack received from the server,
        // resend the packet after TIME_OUT
        while (sock.pollRecvFrom() < 1)  {
            if (timer.lap() > TIME_OUT) {
                sock.sendTo( (char * )message, MSGSIZE);
                retransmit++;
                timer.start();
            }
        }

        // Otherwise, receives acknowledgement from the server
        sock.recvFrom( (char *) message, MSGSIZE);
    }
    return retransmit;
}


/*
 * serverReliable
 * The parameter sock is UdpSocket, max is maximum number of messages,
 * message is the message sent from client to server
 * The server sends acknowledgement to the client so that the client knows
 *  which packet is successfully received by the server.
 */
void serverReliable(UdpSocket &sock, const int max, int *message) {
    cerr << "server reliable test:" << endl;

    // receive message[] max times
    for (int i = 0; i < max; i++) {
        while (true) {
            // if the socket object contains message,
            // server receives message from the client
            if (sock.pollRecvFrom() > 0) {
                sock.recvFrom((char *) message, MSGSIZE);
                if (message[0] == i) {      // checks whether the sequence number equals to i
                    sock.ackTo((char *) &i, sizeof(i)); // send ack to the client
                    break;
                }
            }
        }
    }
}

/*
 * clientSlidingWindow
 * The parameter sock is UdpSocket, max is maximum number of messages,
 * message is the message sent from client to server, windowSize is the size of the window
 * Returns number of retransmission times to the main function depending on the windowSize
 */
int clientSlidingWindow(UdpSocket &sock, const int max, int *message, int windowSize) {
    cerr << "client: sliding window test:" << endl;

    int retransmit = 0;
    int sequence = 0;
    int ackSequence = 0;
    int lastSeqReceived = -1;

    while (sequence < max || ackSequence < max) {
        // if number of in-transit messages is smaller than the window size
        // send multiple messages to the server
        if (ackSequence + windowSize > sequence && sequence < max) {
            message[0] = sequence;
            sock.sendTo( (char *) message, MSGSIZE);
            sequence++;
        }

        // receives last sequence from the server and compare whether it is equal
        // to the acknowledged sequence
        if (sock.pollRecvFrom() > 0) {
            sock.recvFrom(( char *) &lastSeqReceived, sizeof(lastSeqReceived));
            if (lastSeqReceived == ackSequence)
            {
                ackSequence++;
            }
        } else {
            Timer timer;
            timer.start();
            // if the socket doesn't contain any data
            while (sock.pollRecvFrom() < 1) {
                // if time out, retransmit the message right after the currently
                // acknowledged message
                if (timer.lap() > TIME_OUT)  {
                    retransmit = retransmit + (sequence - ackSequence);

                    if (lastSeqReceived >= ackSequence && lastSeqReceived <= sequence) {
                        ackSequence = lastSeqReceived + 1;
                    } else {
                        // reset the sequence to the currently acknowledged sequence
                        sequence = ackSequence;
                    }
                    break;
                }
            }
        }
    }
    return retransmit;
}

/*
 * serverEarlyRetrans
 * The parameter sock is UdpSocket, max is maximum number of messages,
 * message is the message sent from client to server, windowSize is the size of the window
 * Tracks received message's sequence number in its array and returns a cumulative acknowledgement
 * to the client
 */
void serverEarlyRetrans(UdpSocket &sock, const int max, int *message, int windowSize) {
    cerr << "server early retrans test:" << endl;

    int receivedMsg = 0;
    int base = 0;
    int nextSequence = -1;

    // window that keeps track of sequence number of received messages
    vector<bool> window(max, false);

    do {
        if (sock.pollRecvFrom() > 0) {
            sock.recvFrom((char *) message, MSGSIZE);
            receivedMsg = message[0];

            // if unacknowledged message received are bigger than size of the window
            // drop the packet
            if (receivedMsg - base > windowSize) {
                continue;
            } else if (receivedMsg > base) {
                window[receivedMsg] = true; // acknowledge messages

                // track whether the contents in the vector are contiguous
                while (window[base]) {
                    nextSequence = base;
                    base++;
                }
            } else {
                window[receivedMsg] = true;
                nextSequence = base; // get the next sequence
            }
            // send next sequence back to the client
            sock.ackTo((char *) &nextSequence, sizeof(nextSequence));
        }
    } while (base < max);
}


