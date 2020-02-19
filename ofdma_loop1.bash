#!/bin/bash

outpath="./my-simulations/8_ofdma"

./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs0 --datarate=1e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs0-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs1 --datarate=2e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs1-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs2 --datarate=3e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs2-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs3 --datarate=4e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs3-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs4 --datarate=5e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs4-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs5 --datarate=7e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs5-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs6 --datarate=8e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs6-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs7 --datarate=9e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs7-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs8 --datarate=10e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs8-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --dataMode=HeMcs9 --datarate=12e5 --RngRun=$1" &> $outpath/adhoc-csma-mcs9-$1.out


./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs0 --datarate=1e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs0-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs1 --datarate=2e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs1-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs2 --datarate=3e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs2-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs3 --datarate=4e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs3-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs4 --datarate=5e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs4-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs5 --datarate=7e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs5-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs6 --datarate=8e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs6-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs7 --datarate=9e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs7-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs8 --datarate=10e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs8-$1.out
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --startTime=60 --totalTime=120 --dataMode=HeMcs9 --datarate=12e5 --RngRun=$1" &> $outpath/adhoc-ofdma-mcs9-$1.out
