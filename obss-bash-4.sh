#!/bin/bash

./waf

for ((cnt=51;cnt<=100;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=4 --apXStep=35 --apYStep=35 --RngSeed=${cnt} --startTime=10 --totalTime=120 --route=static --datarate=1e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15" | tee ./obss-bash/grid4-${cnt}.txt
done