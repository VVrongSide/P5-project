# Weighted DSR algorithm in ns3

This project uses ns3 as a base for testing weighted routing for mesh networks.

## building
Dependecies: Cmake, gnuplot  
To build this project run:
```
make all
```
running `make` will show what the makefile contains, this is helpfull to get an overview of its functions.
## Plotting
There is a few plots that can be made. Wdsr can be plottet with: `make plot`  
Otherwise scripts for plots can be found in /test/*
## Using
All simulations are done in `scratch/wdsr.cc`

The test and plots can be found in the test folder.

## DISCLAIMER
Please dont use any of this for as a base for your simulation, the examples ns3 comes with are much better and less buggy.  
The patch might make you able to come further in your simulation if you want to delplete an energy source, but its not fully testet and experimental at best
