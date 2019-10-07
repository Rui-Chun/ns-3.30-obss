#!/bin/bash

./waf

# udp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=8e5 --mac=adhoc --app=udp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15 --recordPath=./obss-bash-udp9/grid4o-${cnt}-record.txt" | tee ./obss-bash-udp9/grid4o-${cnt}.txt
done

# tcp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=4e5 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15 --recordPath=./obss-bash-tcp9/grid4o-${cnt}-record.txt" | tee ./obss-bash-tcp9/grid4o-${cnt}.txt
done

# # tcp-1 
# for ((thr=2;thr<=1;thr+=2))
# do
#     for ((cnt=1;cnt<=0;cnt++))
#     do
#         ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=4 --apXStep=30 --apYStep=30 --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=${thr}e5 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15 --recordPath=./obss-bash-tcp-1/grid4o-${thr}-${cnt}-record.txt" | tee ./obss-bash-tcp-1/grid4o-${thr}-${cnt}.txt
#         # cp /home/jiaming/TransRecords.txt ./obss-bash-tcp/grid4o-${cnt}-record.txt
#     done
# done

# # tcp-2
# for ((xs=24;xs<=36;xs+=3))
# do
#     for ((ys=24;ys<=36;ys+=3))
#     do
#         for ((cnt=1;cnt<=100;cnt++))
#         do
#             ./waf --run "mesh-loc-jw --rateControl=obss --datarateUp=false --gridSize=4 --apXStep=${xs} --apYStep=${ys} --RngSeed=${cnt} --startTime=10 --totalTime=40 --route=static --datarate=8e5 --mac=adhoc --app=tcp --constantRate=HeMcs4 --apNum=16 --gateways=0+4+8+12 --appl=3+7+11+15 --recordPath=./obss-bash-tcp-2/grid4o-${xs}-${ys}-${cnt}-record.txt" | tee ./obss-bash-tcp-2/grid4o-${xs}-${ys}-${cnt}.txt
#         done
#     done
# done