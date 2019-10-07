#!/bin/bash

./waf

# udp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=udp --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-udp/${cnt}.txt
done

# # udp
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=udp --rndSeed=${cnt} --route=static" | tee ./mesh-bash-udps/${cnt}.txt
# done

# tcp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=tcpr --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-tcp/${cnt}.txt
done

# tcp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=tcpr --rndSeed=${cnt} --route=static" | tee ./mesh-bash-tcps/${cnt}.txt
done

# # udp
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=udp --rndSeed=${cnt} --route=static" | tee ./mesh-bash-udps/${cnt}.txt
# done

rm minstrel-*.txt

# # tcp
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=tcp --RngSeed=${cnt}" | tee ./mesh-bash-tcp/${cnt}.txt
# done