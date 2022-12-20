#!/bin/bash
  echo "" > timeofdeath.dat
  echo "" > y.dat
  ../../ns3/ns3 run "wdsr-sim"

for i in {0..60}
do
  # echo "$i" >> timeofdeath.dat
  ../../ns3/build/scratch/ns3-dev-wdsr-sim-default --fixed=1 --gamma=44 --alpha="$i" >> timeofdeath.dat && echo "$i" >> y.dat
done
paste y.dat timeofdeath.dat > xy.dat

gnuplot -persist <<-EOFMarker
    set terminal pdf
    set output "alpha-plot.pdf"
    set title "Alpha test"
    set xlabel "Alpha (S)"
    set ylabel "Time of first depletion (S)"
    set xtics 6
    set mxtics 2
    set mytics 2
    set style line 81 lt 0 lc rgb "#808080" lw 0.5
    set grid xtics
    set grid ytics
    set grid mxtics
    set grid mytics
    set grid back ls 81
    plot "xy.dat" using 1:2 with lines notitle
EOFMarker

