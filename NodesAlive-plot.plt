set terminal pdf
set output "ns3/NodesAlive-plot.pdf"
set title "Nodes Alive"
set xlabel "Time (seconds)"
set ylabel "Number of nodes alive"

set datafile missing "-nan"
plot "ns3/WdsrAlive-plot.dat" index 0 title "WDSR: " with steps, "ns3/DsrAlive-plot.dat" index 0 title "DSR: " with steps
