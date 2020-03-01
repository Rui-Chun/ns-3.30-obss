#!/bin/bash

outpath="./my-simulations/8_ofdma2/$1"
mkdir -p $outpath

for (( i=$2 ; i<=$3 ; i++ ))
do
    mcs=$i
    for (( j=2; j<=12; j=j+2 ))
    do
        rate="${j}e5"
        echo $outpath/adhoc-ofdma-mcs$mcs-$rate-$1.out
        ./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --ratio=0.375 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs$mcs-$rate-$1.out
        echo $outpath/adhoc-csma-mcs$mcs-$rate-$1.out
        ./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --ratio=0.375 --RngRun=$1" &> $outpath/adhoc-csma-mcs$mcs-$rate-$1.out
    done
done
