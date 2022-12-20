#!/bin/bash
../../ns3/ns3 run "wdsr-sim --fixed=1"
echo "" > ./AllDsrAlive-plot.dat
echo "" > ./AllWdsrAlive-plot.dat

for i in {888..938}
do
  # echo "$i" >> timeofdeath.dat
  echo "$i" >> y.dat
  ../../ns3/build/scratch/ns3-dev-wdsr-sim-default --comp=1 --seed="$i" --alpha=10 --gamma=44
  cat ./DsrAlive-plot.dat >> ./AllDsrAlive-plot.dat 
  cat ./Dsr-plot.dat > ./"$i"AllDsr-plot.dat
  cat ./WdsrAlive-plot.dat >> ./AllWdsrAlive-plot.dat 
  cat ./Wdsr-plot.dat > ./"$i"AllWdsr-plot.dat
  echo "im at run $i/938"
done
cat AllDsrAlive-plot.dat | LC_ALL=C sort -g -k 1,1  | ../../avg > DsrAlive-plot.dat
cat AllWdsrAlive-plot.dat | LC_ALL=C sort -g -k 1,1  | ../../avg > WdsrAlive-plot.dat
./DsrAlive-plot.sh
./WdsrAlive-plot.sh
gnuplot NodesAlive-plot.plt
