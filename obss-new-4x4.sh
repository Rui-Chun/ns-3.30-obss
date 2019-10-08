#!/bin/bash

./waf

# udp 4x4

for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --apXStep=20 --apYStep=75 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=14e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --gridSize=4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15" | tee ./obss-bash-4x4/grid4x4-${cnt}.txt
done

# udp 4x4
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --apXStep=20 --apYStep=75 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=14e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --gridSize=4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15 --recordPath=./obss-bash-4x4/grid4x4o-${cnt}-record.txt" | tee ./obss-bash-4x4/grid4x4o-${cnt}.txt
done