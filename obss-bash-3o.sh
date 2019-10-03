#!/bin/bash

./waf

for ((cnt=1;cnt<=100;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=3 --apXStep=35 --apYStep=35 --RngSeed=${cnt} --startTime=10 --totalTime=120 --route=static --datarate=5e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash/grid3o-${cnt}.txt
    cp /home/jiaming/TransRecords.txt ./obss-bash/grid3o-${cnt}-record.txt
done