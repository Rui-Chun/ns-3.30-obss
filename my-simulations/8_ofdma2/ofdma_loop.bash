#!/bin/bash

outpath="./my-simulations/8_ofdma2/$1"
mkdir -p $outpath

for (( i=$2 ; i<=$3 ; i++ ))
do
    mcs=$i
    for (( j=2; j<=14; j=j+2 ))
    do
        rate="${j}e6"
        echo $outpath/apsta-ofdma-mcs$mcs-$rate-$1.out
        ./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --RngRun=$1" &> $outpath/apsta-ofdma-mcs$mcs-$rate-$1.out
        echo $outpath/apsta-csma-mcs$mcs-$rate-$1.out
        ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --RngRun=$1" &> $outpath/apsta-csma-mcs$mcs-$rate-$1.out
    done
done
