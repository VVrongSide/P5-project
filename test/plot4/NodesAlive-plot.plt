set terminal pdf
set output "NodesAlive-plot.pdf"
set title "Nodes Alive"
set xlabel "Time (seconds)"
set ylabel "Number of nodes alive"

set datafile missing "-nan"
plot "WdsrAlive-plot.dat" index 0 title "WDSR: " with steps, "DsrAlive-plot.dat" index 0 title "DSR: " with steps
