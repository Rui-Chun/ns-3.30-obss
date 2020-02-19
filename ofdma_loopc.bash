#!/bin/bash

outpath="./my-simulations/8_ofdma"

mcs=$2
case "$mcs" in
0)  rate="2e6"
    ;;
1)  rate="4e6"
    ;;
2)  rate="6e6"
    ;;
3)  rate="7e6"
    ;;
4)  rate="10e6"
    ;;
5)  rate="14e6"
    ;;
6)  rate="16e6"
    ;;
7)  rate="18e6"
    ;;
8)  rate="20e6"
    ;;
9)  rate="24e6"
    ;;
esac

./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --RngRun=$1" &> $outpath/apsta-ofdma-mcs$mcs-$1.out
