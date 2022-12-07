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
	ns3/ns3 run wdsr-sim

anim: netanim/NetAnim
	netanim/NetAnim&

netanim/NetAnim:
	git submodule update --init --recursive
	cd netanim; qmake NetAnim.pro; make

plot:
	ns3/ns3 run wdsr-sim
	cd ns3;./Wdsr-plot.sh && xdg-open Wdsr-plot.png; cd ..