help:
	cat Makefile

all: setup build netanim/NetAnim

setup: ns3/ns3

ns3/ns3:
	git submodule update --init --recursive
	rm -rd ns3/scratch/ ns3/contrib/
	ln -s $(shell pwd)/contrib ns3/
	ln -s $(shell pwd)/scratch ns3/

build:
	ns3/ns3 configure --enable-examples --enable-tests
	ns3/ns3 build

run:
	ns3/ns3 run "wdsr-sim --fixed=1"

anim: netanim/NetAnim
	netanim/NetAnim&

netanim/NetAnim:
	git submodule update --init --recursive
	cd netanim; qmake NetAnim.pro; make

plot: run
	cd ns3;./Wdsr-plot.sh && xdg-open Wdsr-plot.pdf; cd ..

plotComp:
	ns3/ns3 run "wdsr-sim --comp=1"
	cd ns3;./Wdsr-plot.sh && ./Dsr-plot.sh; cd ..
	gnuplot NodesAlive-plot.plt

avg:
	gcc avg.c -o avg -O2

summed: avg plotComp
	cat ns3/Wdsr-plot.dat | ./avg | sort -snk 1,1 | ./avg > ns3/Wdsr-sum.dat
	cat ns3/Dsr-plot.dat | ./avg | sort -snk 1,1 | ./avg > ns3/Dsr-sum.dat
	gnuplot summed-plot.plt
	xdg-open ns3/total-plot.pdf

ns3/src/patch:
	cp patch ns3/src/patch
	cd ns3/src; git apply patch; cd ../../
