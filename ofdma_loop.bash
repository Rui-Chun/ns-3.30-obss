#!/bin/bash

outpath="./my-simulations/8_ofdma"

./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs0 --datarate=2e6 --RngRun=$1" &> $outpath/apsta-csma-mcs0-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs1 --datarate=4e6 --RngRun=$1" &> $outpath/apsta-csma-mcs1-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs2 --datarate=5.5e6 --RngRun=$1" &> $outpath/apsta-csma-mcs2-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs3 --datarate=7e6 --RngRun=$1" &> $outpath/apsta-csma-mcs3-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs4 --datarate=10.5e6 --RngRun=$1" &> $outpath/apsta-csma-mcs4-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs5 --datarate=14e6 --RngRun=$1" &> $outpath/apsta-csma-mcs5-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs6 --datarate=16e6 --RngRun=$1" &> $outpath/apsta-csma-mcs6-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs7 --datarate=18e6 --RngRun=$1" &> $outpath/apsta-csma-mcs7-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs8 --datarate=20e6 --RngRun=$1" &> $outpath/apsta-csma-mcs8-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs9 --datarate=24e6 --RngRun=$1" &> $outpath/apsta-csma-mcs9-$1.out


./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs0 --datarate=2e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs0-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs1 --datarate=4e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs1-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs2 --datarate=5.5e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs2-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs3 --datarate=7e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs3-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs4 --datarate=10.5e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs4-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs5 --datarate=14e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs5-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs6 --datarate=16e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs6-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs7 --datarate=18e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs7-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs8 --datarate=20e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs8-$1.out
./waf --run "simple-ofdma --ofdmaEnabled=true --clNum=4 --startTime=60 --totalTime=120 --dataMode=HeMcs9 --datarate=24e6 --RngRun=$1" &> $outpath/apsta-ofdma-mcs9-$1.out
