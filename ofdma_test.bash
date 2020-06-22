#!/bin/bash

./waf

k=1
rate="20e5"
mcs=4
note="2"
packetsize="1e2"
locationfile="locationtxt/locationFile${note}.txt"
routingfile="locationtxt/routingFile${note}.txt"

# NQOS OFDMA
./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=true"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=true" &> debug-ofdma.txt

# QOS OFDMA
./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=false"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=false" &> debug-ofdma2.txt

# NQOS CSMA
./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=true"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=true" &> debug-csma.txt

# QOS CSMA
./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=false"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --qosDisabled=false" &> debug-csma2.txt