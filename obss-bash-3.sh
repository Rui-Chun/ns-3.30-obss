#!/bin/bash

./waf

# udp
for ((cnt=1;cnt<=0;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=3 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=4e6 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash/grid3-${cnt}.txt
done

# tcp
for ((cnt=1;cnt<=0;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=3 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=2e6 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash-tcp/grid3-${cnt}.txt
done

# tcp-1 
for ((thr=1;thr<=0;thr++))
do
    for ((cnt=1;cnt<=0;cnt++))
    do
        ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=3 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=${thr}e6 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash-tcp-1/grid3-${thr}-${cnt}.txt
    done
done

# tcp-2
for ((xs=24;xs<=36;xs+=3))
do
    for ((ys=24;ys<=36;ys+=3))
    do
        ./waf --run "mesh-loc-jw --rateControl=constant --datarateUp=false --gridSize=3 --apXStep=${xs} --apYStep=${ys} --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=3e6 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=9 --gateways=0+3+6 --appl=2+5+8" | tee ./obss-bash-tcp-2/grid3-${xs}-${ys}-${cnt}.txt
    done
done