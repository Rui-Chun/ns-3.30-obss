#!/bin/bash

./waf

for k in {1..1..1}
do
for mcs in {3..6..1}
do
for note in "42"
do
for note2 in "a" "b1" "b2" "c" "d" "e1" "e2"
do
for packetsize in "4e3"
do
    locationfile="locationtxt/locationFile${note}${note2}.txt"
    routingfile="locationtxt/routingFile${note}.txt"
    appfile="locationtxt/appFile.txt"
    folder="my-simulations/8_ofdma8/unbalanced/topo${note}${note2}-mcs${mcs}-size${packetsize}"
    if [ ! -d ${folder} ]; then
    mkdir -p ${folder}
    fi
for rate in "30e6" "20e6" "10e6" "25e6" "15e6" "5e6"
do
for tcp in 0
do
    outfile="${folder}/ofdma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc6 --ofdmaEnabled=true --startTime=60 --totalTime=90 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --tcp=${tcp} --randomTime=true --qosDisabled=true" &> ${outfile}
    fi

    outfile="${folder}/csma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc6 --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=90 --packetSize=${packetsize} --dataMode=HeMcs$mcs --datarate=$rate --ratio=0.375 --RngRun=$k --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --tcp=${tcp} --randomTime=true --qosDisabled=true" &> ${outfile}
    fi
done
done
done
done
done
done
done
