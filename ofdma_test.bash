#!/bin/bash

./waf

k=1
rate="10e5"
mcs=4
note="2"
packetsize="1e2"
locationfile="locationtxt/locationFile${note}.txt"
routingfile="locationtxt/routingFile${note}.txt"

# ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"

NS_LOG="MacLow=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}" &> debug-ofdma.txt


# ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"

NS_LOG="MacLow=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}" &> debug-csma.txt