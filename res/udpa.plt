set terminal postscript landscape
set nolabel
set xlabel "drop rate %"
set xrange [0:10]
set ylabel "usec"
set yrange [0:8000000]
set output "udpa.ps"
plot "window-1.dat" title "slinding window (window size 1)" with linespoints, "window-30.dat" title "sliding window (window-size 30)" with line