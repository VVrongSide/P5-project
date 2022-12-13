set terminal pdf
set output "ns3/total-plot.pdf"
set title "Energy remaining pr. node"
set xlabel "Time (seconds)"
set ylabel "Energy"

plot "ns3/Wdsr-sum.dat" title "WDSR" with lines, "ns3/Dsr-sum.dat" title "DSR" with lines
