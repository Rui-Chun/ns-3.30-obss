#!/bin/bash

./waf

for ((cnt=1;cnt<=50;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=8e5 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15" | tee ./obss-bash/grid4o-${cnt}.txt
    cp TransRecords.txt ./obss-bash/grid4o-${cnt}-record.txt
done
