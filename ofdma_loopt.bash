#!/bin/bash

for (( k=$1; k<=$2; k=k+1 ))
do
    outpath="./my-simulations/8_ofdma6/$k"
    mkdir -p $outpath
    for (( j=2; j<=9; j=j+1 ))
    do
        rate="${j}e5"
        for (( i=0 ; i<=7 ; i++ ))
        do
            mcs=$i

            filename="$outpath/adhoc-ofdma-mcs$mcs-$rate-$k.out"
            if [ ! -f $filename ]
            then
                echo $filename
                touch $filename
                ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --ratio=0.375 --RngRun=$k" &> $filename
            fi

            filename="$outpath/adhoc-csma-mcs$mcs-$rate-$k.out"
            if [ ! -f $filename ]
            then
                echo $filename
                touch $filename
                ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --clNum=20 --ratio=0.375 --RngRun=$k" &> $filename
            fi
        done
    done
done
