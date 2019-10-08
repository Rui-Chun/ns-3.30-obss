#!/bin/bash

./waf

# udp 3x3 change 

for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --apXStep=20 --apYStep=75 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=14e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --gridSize=3 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash-3x3/grid3x3-${cnt}.txt
done

# udp 3x3 change 
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --apXStep=20 --apYStep=75 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=14e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --gridSize=3 --apNum=9 --gateways=0+3+6 --appl=2+5+8 --recordPath=./obss-bash-3x3/grid3x3o-${cnt}-record.txt" | tee ./obss-bash-3x3/grid3x3o-${cnt}.txt
done