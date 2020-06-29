#!/bin/bash

./waf

for k in {1..50..1}
do
for mcs in {3..5..1}
do
for note in "4" "22" "23" "24" "33" "222"
do
for packetsize in "4e3"
do
    locationfile="locationtxt/locationFile${note}.txt"
    routingfile="locationtxt/routingFile${note}.txt"
    appfile="locationtxt/appFile.txt"
    folder="my-simulations/8_ofdma8/onoffmodify/topo${note}-mcs${mcs}-size${packetsize}"
    if [ ! -d ${folder} ]; then
    mkdir -p ${folder}
    fi
for rate in "40e6"
do
for tcp in 0
do
    outfile="${folder}/ofdma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc6 --ofdmaEnabled=true --startTime=60 --totalTime=120 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --tcp=${tcp} --randomTime=true --qosDisabled=true" &> ${outfile}
    fi

    outfile="${folder}/csma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc6 --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --tcp=${tcp} --randomTime=true --qosDisabled=true" &> ${outfile}
    fi
done
done
done
done
done
done
