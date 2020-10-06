#!/bin/bash

./waf

k=0
rate="10e5"
mcs=4
note="4"
packetsize="4e3"
tcp=0
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

# NS_LOG="MacLow=level_function|time|node|func:WifiPhy=level_function|time|node|func:ChannelAccessManager=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=10.5 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true" &> debug-ofdma.txt

# QOS OFDMA
# ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=true --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false" &> debug-ofdma2.txt

# NQOS CSMA
# ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true"

# NS_LOG="MacLow=level_function|time|node|func:WifiPhy=level_function|time|node|func:ChannelAccessManager=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=10.5 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=true" &> debug-csma.txt

# QOS CSMA
# ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=20 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false"

# NS_LOG="MacLow=level_function|time|node|func" ./waf --run "${codefile} --ofdmaEnabled=false --rtscts=1 --startTime=10 --totalTime=11 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} ${appcommand} --tcp=${tcp} --qosDisabled=false" &> debug-csma2.txt





# NS_LOG="MacLow=level_function|time|node|func:WifiPhy=level_function|time|node|func" ./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=60 --totalTime=120 --packetSize=4e3 --dataMode=HeMcs4 --datarate=25e5 --ratio=0.375 --RngRun=11 --locationFile=./locationtxt/locationFile23.txt --routingFile=./locationtxt/routingFile23.txt --tcp=1 --qosDisabled=true" &> debug-ofdma2.txt

./waf --run "simple-ofdma-adhoc4 --ofdmaEnabled=true --startTime=60 --totalTime=120 --packetSize=4e3 --dataMode=HeMcs4 --datarate=25e5 --ratio=0.375 --RngRun=11 --locationFile=./locationtxt/locationFile23.txt --routingFile=./locationtxt/routingFile23.txt --tcp=1 --qosDisabled=true"