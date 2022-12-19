#!/bin/bash
../../ns3/ns3 run "wdsr-sim --fixed=1"
echo "" > ./AllDsrAlive-plot.dat
echo "" > ./AllWdsrAlive-plot.dat
echo "" > ./AllWdsr-plot.dat
echo "" > ./AllDsr-plot.dat

for i in {100..105}
do
  # echo "$i" >> timeofdeath.dat
  echo "$i" >> y.dat
  ../../ns3/build/scratch/ns3-dev-wdsr-sim-default --comp=1 --seed="$i"
  cat ./DsrAlive-plot.dat >> ./AllDsrAlive-plot.dat 
  cat ./Dsr-plot.dat >> ./AllDsr-plot.dat
  cat ./WdsrAlive-plot.dat >> ./AllWdsrAlive-plot.dat 
  cat ./Wdsr-plot.dat >> ./AllWdsr-plot.dat
done
cat AllDsrAlive-plot.dat | ../../add > DsrAlive-plot.dat
# cat AllDsr-plot.dat | ../../add > Dsr-plot.dat
cat AllWdsrAlive-plot.dat | ../../add > WdsrAlive-plot.dat
# cat AllWdsr-plot.dat | ../../add > Wdsr-plot.dat
 ./DsrAlive-plot.sh
 ./WdsrAlive-plot.sh
gnuplot NodesAlive-plot.plt
