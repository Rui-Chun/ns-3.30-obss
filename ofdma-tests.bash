#!/bin/bash

unset NS_LOG
./waf

# udp
# echo 01
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --datarate=2e6 --clNum=4 --startTime=60 --totalTime=120" &> log01.out
# echo 02
# ./waf --run "simple-ofdma --ofdmaEnabled=true --datarate=2e6 --clNum=4 --startTime=60 --totalTime=120" &> log02.out
# echo 03
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --dataMode=HeMcs4 --datarate=5e6 --clNum=4 --startTime=60 --totalTime=120" &> log03.out
# echo 04
# ./waf --run "simple-ofdma --ofdmaEnabled=true --dataMode=HeMcs4 --datarate=5e6 --clNum=4 --startTime=60 --totalTime=120" &> log04.out

# echo 05
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --datarate=2e6 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log05.out
# echo 06
# ./waf --run "simple-ofdma --ofdmaEnabled=true --datarate=2e6 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log06.out
# echo 07
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --dataMode=HeMcs4 --datarate=5e6 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log07.out
# echo 08
# ./waf --run "simple-ofdma --ofdmaEnabled=true --dataMode=HeMcs4 --datarate=5e6 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log08.out

# echo 09
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --datarate=2e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120" &> log09.out
# echo 10
# ./waf --run "simple-ofdma --ofdmaEnabled=true --datarate=2e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120" &> log10.out
# echo 11
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --dataMode=HeMcs4 --datarate=5e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120" &> log11.out
# echo 12
# ./waf --run "simple-ofdma --ofdmaEnabled=true --dataMode=HeMcs4 --datarate=5e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120" &> log12.out

# echo 13
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --datarate=2e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log13.out
# echo 14
# ./waf --run "simple-ofdma --ofdmaEnabled=true --datarate=2e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log14.out
# echo 15
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --dataMode=HeMcs4 --datarate=5e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log15.out
# echo 16
# ./waf --run "simple-ofdma --ofdmaEnabled=true --dataMode=HeMcs4 --datarate=5e6 --adhoc=1 --clNum=4 --startTime=60 --totalTime=120 --routing=olsr" &> log16.out

echo 17
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=false --rtscts=1 --datarate=3e4 --clNum=20 --startTime=60 --totalTime=120" &> log17.out
echo 18
./waf --run "simple-ofdma-adhoc --ofdmaEnabled=true --datarate=3e4 --clNum=20 --startTime=60 --totalTime=120" &> log18.out








# tcp
# echo 51
# ./waf --run "simple-ofdma --ofdmaEnabled=false --rtscts=1 --datarate=2e6 --clNum=4 --startTime=60 --totalTime=120 --tcp=1" &> log51.out
# echo 52
# ./waf --run "simple-ofdma --ofdmaEnabled=true --datarate=2e6 --clNum=4 --startTime=60 --totalTime=120 --tcp=1" &> log52.out