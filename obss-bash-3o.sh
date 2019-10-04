#!/bin/bash

./waf

# udp
for ((cnt=1;cnt<=0;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=3 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=4e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash/grid3o-${cnt}.txt
    cp /home/jiaming/TransRecords.txt ./obss-bash/grid3o-${cnt}-record.txt
done

# tcp
for ((cnt=1;cnt<=0;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=3 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=2e6 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash-tcp/grid3o-${cnt}.txt
    cp /home/jiaming/TransRecords.txt ./obss-bash-tcp/grid3o-${cnt}-record.txt
done

# tcp-1 
for ((thr=1;thr<=6;thr++))
do
    for ((cnt=1;cnt<=100;cnt++))
    do
        ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=3 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=${thr}e6 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8 --recordPath=./obss-bash-tcp-1/grid3o-${thr}-${cnt}-record.txt" | tee ./obss-bash-tcp-1/grid3o-${thr}-${cnt}.txt
        # cp /home/synrg/TransRecords.txt ./obss-bash-tcp-1/grid3o-${cnt}-record.txt
    done
done