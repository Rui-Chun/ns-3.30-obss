#!/bin/bash

for (( k=$1; k<=$2; k=k+1 ))
do
    outpath="./my-simulations/8_ofdma8/$k"
    mkdir -p $outpath
    for (( j=2; j<=10; j=j+2 ))
    do
        rate="${j}e5"
        for (( i=1 ; i<=7 ; i=i+2 ))
        do
            mcs=$i

            for note in "22" "23" "24" "25" "32" "33" "52" "222"
            do
                locationfile="my-simulations/8_ofdma7/locationFile${note}.txt"
                routingfile="my-simulations/8_ofdma7/routingFile${note}.txt"
                filename="$outpath/adhoc-ofdma-mcs$mcs-$rate-$k-${note}.out"
                if [ ! -f $filename ]
                then
                    echo $filename
                    touch $filename
                    ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --tcp=1 --locationFile=${locationfile} --routingFile=${routingfile}" &> $filename
                fi

                filename="$outpath/adhoc-csma-mcs$mcs-$rate-$k-${note}.out"
                if [ ! -f $filename ]
                then
                    echo $filename
                    touch $filename
                    ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false -rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --tcp=1 --locationFile=${locationfile} --routingFile=${routingfile}" &> $filename
                fi
            done
        done
    done
done
