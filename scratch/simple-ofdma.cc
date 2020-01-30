#include <iostream>
#include <vector>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

/// throughput monitor
std::vector<uint64_t> lastTotalRx;
std::vector<double> throughput;
std::vector<Ptr<PacketSink>> packetSink;

double
CalculateSingleStreamThroughput (Ptr<PacketSink> sink, uint64_t &lastTotalRx, double monitorInterval)
{
  double thr = (sink->GetTotalRx () - lastTotalRx) * (double) 8/1e3 / monitorInterval;
  lastTotalRx = sink->GetTotalRx ();
  return thr;
}

void
CalculateThroughput (double monitorInterval)
{
  std::cout << Simulator::Now ().GetSeconds ();
  for (uint32_t i = 0; i < packetSink.size (); ++i)
    {
      if (packetSink[i] == NULL)
        throughput[i] = -1;
      else
        throughput[i] = CalculateSingleStreamThroughput (packetSink[i], lastTotalRx[i], monitorInterval);
      std::cout << '\t' << throughput[i];
    }
  std::cout << std::endl;
  Simulator::Schedule (Seconds (monitorInterval), &CalculateThroughput, monitorInterval);
}

void
PrintThroughputTitle (uint32_t clNum)
{
  std::cout << "--------------------------Rate[Kbps]--------------------------\n";
  std::cout << "Time[s]";
  for (uint32_t i = 0; i < clNum; ++i)
      std::cout << '\t' << "cl-" << i;
  std::cout << std::endl;
}

int main (int argc, char **argv)
{
  CommandLine cmd;

  bool ofdmaEnabled = false;
  uint32_t clNum = 4;
  double startTime = 60.0;
  double totalTime = 120.0;
  double monitorInterval = 1.0;
  double datarate = 1e5;
  double packetSize = 1e3;

  cmd.AddValue ("ofdmaEnabled", "", ofdmaEnabled);
  cmd.AddValue ("clNum", "", clNum);
  cmd.AddValue ("startTime", "Application start time, s.", startTime);
  cmd.AddValue ("totalTime", "Simulation time, s.", totalTime);
  cmd.AddValue ("monitorInterval", "Monitor interval, s.", monitorInterval);
  cmd.AddValue ("datarate", "Datarate, bps.", datarate);
  cmd.AddValue ("packetSize", "Packet size, bytes.", packetSize);
  cmd.Parse (argc, argv);

  // Create nodes and set mobility
  NodeContainer apNodes;
  apNodes.Create (1);
  NodeContainer clNodes;
  clNodes.Create (clNum);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (apNodes);
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0),
                                 "Rho", StringValue ("ns3::ConstantRandomVariable[Constant=10]"),
                                 "Theta", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.2830]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (clNodes);

  // Install mac and phy
  NetDeviceContainer apDevices;
  NetDeviceContainer clDevices;

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper chan = YansWifiChannelHelper::Default ();
  phy.SetChannel (chan.Create ());
  phy.Set ("ChannelNumber", UintegerValue (36));

  WifiHelper wifi;
  wifi.SetStandard (ns3::WIFI_PHY_STANDARD_80211ax_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "ControlMode", StringValue ("HeMcs0"),
                                "DataMode", StringValue ("HeMcs7"),
                                "RtsCtsThreshold", UintegerValue (65535));

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ofdma");
  mac.SetType ("ns3::ApWifiMac",
               "BeaconInterval", TimeValue (MicroSeconds (65535 * 1024)),
               "Ssid", SsidValue (ssid),
               "OfdmaSupported", BooleanValue (ofdmaEnabled));
  apDevices = wifi.Install (phy, mac, apNodes);
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "OfdmaSupported", BooleanValue (ofdmaEnabled));
  clDevices = wifi.Install (phy, mac, clNodes);

  // Install Internet stack
  InternetStackHelper stack;
  stack.Install (apNodes);
  stack.Install (clNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterfaces = address.Assign (apDevices);
  Ipv4InterfaceContainer clInterfaces = address.Assign (clDevices);

  // Install application
  ApplicationContainer apApplications;
  ApplicationContainer clApplications;
  ApplicationContainer trApplications;
  for (uint32_t i = 0; i < clNum; i++)
    {
      uint16_t port = 5000+i;
      Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper server ("ns3::UdpSocketFactory", localAddress);
      clApplications.Add (server.Install (clNodes.Get (i)));
      packetSink.push_back (StaticCast<PacketSink> (clApplications.Get (i)));
      lastTotalRx.push_back (0);
      throughput.push_back (0);

      OnOffHelper client ("ns3::UdpSocketFactory", Address ());
      AddressValue remoteAddress (InetSocketAddress (clInterfaces.GetAddress (i), port));
      client.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      client.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      client.SetAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) (datarate))));
      client.SetAttribute ("PacketSize", UintegerValue (packetSize));
      client.SetAttribute ("Remote", remoteAddress);
      apApplications.Add (client.Install (apNodes));

      client.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1|Bound=2]"));
      client.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1|Bound=2]"));
      client.SetAttribute ("DataRate", DataRateValue (DataRate (1e3)));
      client.SetAttribute ("PacketSize", UintegerValue (25));
      trApplications.Add (client.Install (apNodes));
    }
  apApplications.Start (Seconds (startTime));
  apApplications.Stop (Seconds (totalTime));
  clApplications.Start (Seconds (0.0));
  clApplications.Stop (Seconds (totalTime + 0.1));
  trApplications.Start (Seconds (startTime * 0.3));
  trApplications.Stop (Seconds (startTime * 0.8));

  Simulator::Schedule (Seconds (startTime), &PrintThroughputTitle, clNum);
  Simulator::Schedule (Seconds (startTime), &CalculateThroughput, monitorInterval);

  // Config::Set ("/NodeLst/*/DeviceList/*/$ns3::WifiNetDevice/Mac/QosSupported", BooleanValue (false));
  Config::Set ("/NodeLst/*/DeviceList/*/$ns3::WifiNetDevice/Mac/VI_MaxAmpduSize", UintegerValue (0));
  Config::Set ("/NodeLst/*/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize", UintegerValue (0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (totalTime + 0.1));
  Simulator::Run ();
  Simulator::Destroy ();

  return true;
}