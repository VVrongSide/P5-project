#!/bin/bash
  echo "" > timeofdeath.dat
  echo "" > y.dat
  ../../ns3/ns3 run "wdsr-sim --fixed=1"

for i in {126..2}
do
  # echo "$i" >> timeofdeath.dat
  ../../ns3/build/scratch/ns3-dev-wdsr-sim-default --fixed=1 --gamma="$i" >> timeofdeath.dat
  echo "$i" >> y.dat
done
paste y.dat timeofdeath.dat > xy.dat

gnuplot -persist <<-EOFMarker
    set terminal pdf
    set output "gamma-plot.pdf"
    set title "Gamma test"
    set xlabel "Gamma"
    set ylabel "Time of first depletion (s)"
    plot "xy.dat" using 1:2 with lines notitle
EOFMarker

# set datafile missing "-nan"
# plot "Wdsr-plot.dat" index 0 title "Node: 0" with lines, "Wdsr-plot.dat" index 1 title "Node: 1" with lines, "Wdsr-plot.dat" index 2 title "Node: 2" with lines, "Wdsr-plot.dat" index 3 title "Node: 3" with lines, "Wdsr-plot.dat" index 4 title "Node: 4" with lines, "Wdsr-plot.dat" index 5 title "Node: 5" with lines, "Wdsr-plot.dat" index 6 title "Node: 6" with lines, "Wdsr-plot.dat" index 7 title "Node: 7" with lines, "Wdsr-plot.dat" index 8 title "Node: 8" with lines

