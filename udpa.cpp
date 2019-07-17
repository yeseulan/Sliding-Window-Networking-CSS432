/* Yeseul An
 * CSS 432
 * Assignment 3
 * The program that implements functions such as clientSlidingWindow, serverEarlyRetrans with dropping rate
 * which are used in the hw3a.cpp main program
 */

#include "UdpSocket.h"
#include "Timer.h"
#include <vector>
#include <random>

void serverEarlyRetrans(UdpSocket &sock, const int max, int message[], int windowSize, int dropRate);
bool returnRandomNumber(int dropRate);

/*
 * returnRandomNumber
 * The parameter dropRate is the dropping rate given from the main program
 * Returns true if randomly selected N(between 0 to 99) is smaller or equal than the drop rate
 * Otherwise, return false
 */
bool returnRandomNumber(int dropRate) {
    int n = rand() % 100;
    if (n <= dropRate) {
        return true;
    }
    return false;
}

/*
 * serverEarlyRetrans
 * The parameter sock is UdpSocket, max is maximum number of messages,
 * message is the message sent from client to server, windowSize is the size of the window
 * Tracks received message's sequence number in its array and returns a cumulative acknowledgement
 * to the client
 */
void serverEarlyRetrans(UdpSocket &sock, const int max, int *message, int windowSize, int dropRate) {
    cerr << "server early retrans test with drop rate:" << endl;

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

            bool dropFlag = returnRandomNumber(dropRate);

            if (!dropFlag) {
                // send next sequence back to the client
                sock.ackTo((char *) &nextSequence, sizeof(nextSequence));
            }
        }
    } while (base < max);
}