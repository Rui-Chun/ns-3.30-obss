#!/bin/bash

outpath="./my-simulations/8_ofdma"

mcs=$2
case "$mcs" in
0)  rate="1e5"
    ;;
1)  rate="2e5"
    ;;
2)  rate="3e5"
    ;;
3)  rate="4e5"
    ;;
4)  rate="5e5"
    ;;
5)  rate="7e5"
    ;;
6)  rate="8e5"
    ;;
7)  rate="9e5"
    ;;
8)  rate="10e5"
    ;;
9)  rate="12e5"
    ;;
esac

./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs$mcs-$1.out