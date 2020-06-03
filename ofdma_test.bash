#!/bin/bash

./waf

k=1
rate="7e5"
mcs=0
note="22"
locationfile="locationtxt/locationFile${note}.txt"
routingfile="locationtxt/routingFile${note}.txt"

./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"

# NS_LOG="WifiPhy=level_function|time|node|func:InterferenceHelper=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=14.2 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}" &> debug-ofdma.txt


# ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"

# NS_LOG="MacLow=level_function|time|node|func:InterferenceHelper=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=10.2 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}" &> debug-csma.txt