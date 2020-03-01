#include <iostream>
#include <vector>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

/// throughput monitor
std::vector<uint64_t> firstTotalRx;
std::vector<uint64_t> lastTotalRx;
std::vector<double> throughput;
std::vector<double> delay;
std::vector<double> loss;
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
      throughput[i] = CalculateSingleStreamThroughput (packetSink[i], lastTotalRx[i], monitorInterval);
      if (firstTotalRx[i] == 0)
        {
          firstTotalRx[i] = lastTotalRx[i];
        }
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

void
CalculateAverageThroughput (double duration)
{
  std::cout << "avg";
  for (uint32_t i = 0; i < packetSink.size (); ++i)
    {
      std::cout << '\t' << (lastTotalRx[i]-firstTotalRx[i]) * (double) 8/1e3 / duration;
    }
  std::cout << std::endl;
}

int main (int argc, char **argv)
{
  CommandLine cmd;

  bool ofdmaEnabled = false;
  uint32_t clNum = 2;
  double startTime = 10.0;
  double totalTime = 11.0;
  double monitorInterval = 1.0;
  std::string dataMode = "HeMcs0";
  std::string routing = "none";
  double datarate = 1e6;
  double packetSize = 1e3;
  uint32_t adhoc = 0;
  uint32_t tcp = 0;
  uint32_t rtscts = 0;

  cmd.AddValue ("ofdmaEnabled", "", ofdmaEnabled);
  cmd.AddValue ("clNum", "", clNum);
  cmd.AddValue ("startTime", "Application start time, s.", startTime);
  cmd.AddValue ("totalTime", "Simulation time, s.", totalTime);
  cmd.AddValue ("monitorInterval", "Monitor interval, s.", monitorInterval);
  cmd.AddValue ("dataMode", "Mode for data frames.", dataMode);
  cmd.AddValue ("routing", "Routing algo, none/aodv/olsr", routing);
  cmd.AddValue ("datarate", "Datarate, bps.", datarate);
  cmd.AddValue ("packetSize", "Packet size, bytes.", packetSize);
  cmd.AddValue ("adhoc", "mac protocol, 0 = apsta, 1 = adhoc", adhoc);
  cmd.AddValue ("tcp", "tcp protocl, 0 = udp, 1 = tcp", tcp);
  cmd.AddValue ("rtscts", "rtscts for csma, 0 = disable, 1 = enable", rtscts);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::RegularWifiMac::VO_MaxAmsduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::VI_MaxAmsduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BE_MaxAmsduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BK_MaxAmsduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::VO_MaxAmpduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::VI_MaxAmpduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BE_MaxAmpduSize", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BK_MaxAmpduSize", UintegerValue (0));

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
                                 "Rho", StringValue ("ns3::UniformRandomVariable[Min=15.0|Max=15.0]"),
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
                                "DataMode", StringValue (dataMode),
                                "RtsCtsThreshold", rtscts ? UintegerValue (0) : UintegerValue (4294967295));

  WifiMacHelper mac;
  if (adhoc)
    {
      mac.SetType ("ns3::AdhocWifiMac",
                   "OfdmaSupported", BooleanValue (ofdmaEnabled));
      apDevices = wifi.Install (phy, mac, apNodes);
      clDevices = wifi.Install (phy, mac, clNodes);
    }
  else
    {
      Ssid ssid = Ssid ("ofdma");
      mac.SetType ("ns3::ApWifiMac",
                  "Ssid", SsidValue (ssid),
                  "OfdmaSupported", BooleanValue (ofdmaEnabled));
      apDevices = wifi.Install (phy, mac, apNodes);
      mac.SetType ("ns3::StaWifiMac",
                  "Ssid", SsidValue (ssid),
                  "OfdmaSupported", BooleanValue (ofdmaEnabled));
      clDevices = wifi.Install (phy, mac, clNodes);
    }

  // Install Internet stack
  InternetStackHelper stack;
  if (routing == std::string ("aodv"))
    {
      AodvHelper aodv;
      stack.SetRoutingHelper (aodv);
    }
  else if (routing == std::string ("olsr"))
    {
      OlsrHelper olsr;
      stack.SetRoutingHelper (olsr);
    }
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
  if (tcp)
    {
      for (uint32_t i = 0; i < clNum; i++)
        {
          uint16_t port = 5000+i;
          Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
          PacketSinkHelper server ("ns3::TcpSocketFactory", localAddress);
          clApplications.Add (server.Install (clNodes.Get (i)));
          packetSink.push_back (StaticCast<PacketSink> (clApplications.Get (i)));
          firstTotalRx.push_back (0);
          lastTotalRx.push_back (0);
          throughput.push_back (0);
          delay.push_back (0);
          loss.push_back (0);

          OnOffHelper client ("ns3::TcpSocketFactory", Address ());
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
    }
  else
    {
      for (uint32_t i = 0; i < clNum; i++)
        {
          uint16_t port = 5000+i;
          Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
          PacketSinkHelper server ("ns3::UdpSocketFactory", localAddress);
          clApplications.Add (server.Install (clNodes.Get (i)));
          packetSink.push_back (StaticCast<PacketSink> (clApplications.Get (i)));
          firstTotalRx.push_back (0);
          lastTotalRx.push_back (0);
          throughput.push_back (0);
          delay.push_back (0);
          loss.push_back (0);

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
    }
  
  apApplications.Start (Seconds (startTime));
  apApplications.Stop (Seconds (totalTime));
  clApplications.Start (Seconds (0.0));
  clApplications.Stop (Seconds (totalTime + 0.1));
  trApplications.Start (Seconds (startTime * 0.3));
  trApplications.Stop (Seconds (startTime * 0.8));

  Simulator::Schedule (Seconds (startTime), &PrintThroughputTitle, clNum);
  Simulator::Schedule (Seconds (startTime), &CalculateThroughput, monitorInterval);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Stop (Seconds (totalTime + 0.1));
  Simulator::Run ();

  // Throughput
  CalculateAverageThroughput (totalTime - startTime);

  // Delay
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if ((t.destinationPort >= 5000) && (t.destinationPort < 5000+clNum))
        {
          uint32_t index = t.destinationPort - 5000;
          if (!t.destinationAddress.IsEqual (clInterfaces.GetAddress (index)))
            {
              continue;
            }
          if (i->second.rxPackets > 0)
            {
              delay[index] = (double)i->second.delaySum.GetMilliSeconds () / (double)i->second.rxPackets;
              loss[index] = (double)i->second.lostPackets / (double)(i->second.lostPackets + i->second.rxPackets);
            }
          else
            {
              delay[index] = 0;
              loss[index] = 0;
            }
        }
    }
  std::cout << "delay";
  for (uint32_t i = 0; i < packetSink.size (); ++i)
    {
      std::cout << '\t' << delay[i];
    }
  std::cout << std::endl;
  std::cout << "loss";
  for (uint32_t i = 0; i < packetSink.size (); ++i)
    {
      std::cout << '\t' << loss[i];
    }
  std::cout << std::endl;

  Simulator::Destroy ();

  return true;
}
