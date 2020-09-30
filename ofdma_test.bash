#!/bin/bash

./waf

k=0
rate="10e6"
mcs=4
note="4"
packetsize="4e3"
tcp=1
locationfile="locationtxt/locationFile${note}.txt"
routingfile="locationtxt/routingFile${note}.txt"
appfile="locationtxt/appFile.txt"

codefile="simple-ofdma-adhoc4"
if [ "${codefile}" == "simple-ofdma-adhoc5" ]; then
    appcommand="--appFile=${appfile}"
else
    appcommand=""
fi

# NQOS OFDMA
# ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true"

# NS_LOG="MacLow=level_function|time|node|func:WifiPhy=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=13 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true" &> debug-ofdma.txt

# QOS OFDMA
# ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false"

NS_LOG="MacLow=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false" &> debug-ofdma2.txt

# NQOS CSMA
# ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true" &> debug-csma.txt

# QOS CSMA
# ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false" &> debug-csma2.txt