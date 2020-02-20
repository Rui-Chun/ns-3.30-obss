#!/bin/bash

outpath="./my-simulations/8_ofdma/$1/"

for (( i=$2 ; i<=$3 ; i++ ))
do
    mcs=$i
    case "$mcs" in
    0)  rate="2e6"
        ;;
    1)  rate="3e6"
        ;;
    2)  rate="4e6"
        ;;
    3)  rate="5e6"
        ;;
    4)  rate="6e6"
        ;;
    5)  rate="7e6"
        ;;
    6)  rate="8e6"
        ;;
    7)  rate="9e6"
        ;;
    8)  rate="10e6"
        ;;
    9)  rate="11e6"
        ;;
    esac

    ./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --RngRun=$1" &> $outpath/apsta-ofdma-mcs$mcs-$1.out
    ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --RngRun=$1" &> $outpath/apsta-csma-mcs$mcs-$1.out
done