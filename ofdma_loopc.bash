#!/bin/bash

outpath="./my-simulations/8_ofdma/$1"
mkdir -p $outpath

for (( i=$2 ; i<=$3 ; i++ ))
do
    mcs=$i
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
    5)  rate="6e5"
        ;;
    6)  rate="7e5"
        ;;
    7)  rate="7e5"
        ;;
    8)  rate="8e5"
        ;;
    9)  rate="8e5"
        ;;
    esac

    ./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --ratio=0.375 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs$mcs-$1.out
    ./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --ratio=0.375 --RngRun=$1" &> $outpath/adhoc-csma-mcs$mcs-$1.out
done