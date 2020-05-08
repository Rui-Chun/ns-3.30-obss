#!/bin/bash

./waf

k=1
rate="10e5"
mcs=4
note="22"
locationfile="locationtxt/locationFile${note}.txt"
routingfile="locationtxt/routingFile${note}.txt"

# ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"
# ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"

NS_LOG="Txop=level_function|time|node|func:MacLow=level_function|time|node|func:WifiMacQueue=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}" &> debug.txt
# ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile}"