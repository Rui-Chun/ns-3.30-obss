#!/bin/bash

./waf

# udp 2x3 change ystep

for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --apXStep=20 --apYStep=$ys --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=14e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --gridSize=3 --apNum=6 --gateways=0+3 --appl=2+5" | tee ./obss-bash-2x3-y/grid2x3-${ys}-${cnt}.txt
done

# udp 2x3 change ystep
for ((ys=72;ys<=90;ys+=3))
do
    for ((cnt=$1;cnt<=$2;cnt++))
    do
        ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --apXStep=20 --apYStep=$ys --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=14e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --gridSize=3 --apNum=6 --gateways=0+3 --appl=2+5 --recordPath=./obss-bash-2x3-y/grid3o-${ys}-${cnt}-record.txt" | tee ./obss-bash-2x3-y/grid2x3o-${ys}-${cnt}.txt
    done
done