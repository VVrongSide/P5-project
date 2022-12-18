#!/bin/bash
  echo "" > timeofdeath.dat
  echo "" > y.dat
  ../../ns3/ns3 run "wdsr-sim --fixed=1"

for i in {126..2}
do
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
