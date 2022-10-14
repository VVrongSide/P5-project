setup:
	git submodule update --init --recursive
	rm -rd ns3/scratch/ ns3/contrib/
	ln -s $(shell pwd)/contrib ns3/
	ln -s $(shell pwd)/scratch ns3/


build:
	ns3/ns3 configure --enable-examples --enable-tests
	ns3/ns3 build
