#!/bin/bash
  echo "" > nodesalive.dat
  echo "" > y.dat
  ../../ns3/ns3 run "wdsr-sim"

for i in {0..30}
do
  # echo "$i" >> timeofdeath.dat
  ../../ns3/build/scratch/ns3-dev-wdsr-sim-default --fixed=0 --alpha="$i" >> timeofdeath.dat && echo "$i" >> y.dat
done
paste y.dat timeofdeath.dat > xy.dat

gnuplot -persist <<-EOFMarker
    set terminal pdf
    set output "alpha-plot.pdf"
    set title "Alpha test"
    set xlabel "Alpha (S)"
    set ylabel "Time of first depletion (S)"
    plot "xy.dat" using 1:2 with lines notitle
EOFMarker

