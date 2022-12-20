#!/bin/bash
  echo "" > timeofdeath.dat
  echo "" > y.dat
  ../../ns3/ns3 run "wdsr-sim --fixed=1"

for i in {63..0}
do
  ../../ns3/build/scratch/ns3-dev-wdsr-sim-default --fixed=1 --gamma="$i" --alpha=1 >> timeofdeath.dat && echo "$i" >> y.dat
done
paste y.dat timeofdeath.dat > xy.dat

gnuplot plt
