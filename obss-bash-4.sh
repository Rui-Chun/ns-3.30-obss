#!/bin/bash

./waf

# udp
for ((cnt=1;cnt<=0;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=8e5 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15" | tee ./obss-bash/grid4-${cnt}.txt
done

# tcp
for ((cnt=1;cnt<=0;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=4e5 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15" | tee ./obss-bash-tcp/grid4-${cnt}.txt
done

# tcp-1 
for ((thr=2;thr<=12;thr+=2))
do
    for ((cnt=1;cnt<=100;cnt++))
    do
        ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=${thr}e5 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15" | tee ./obss-bash-tcp-1/grid4-${thr}-${cnt}.txt
    done
done