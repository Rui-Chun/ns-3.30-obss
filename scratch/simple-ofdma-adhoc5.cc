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
PrintThroughputTitle (uint32_t apNum)
{
  std::cout << "--------------------------Rate[Kbps]--------------------------\n";
  std::cout << "Time[s]";
  for (uint32_t i = 0; i < packetSink.size (); ++i)
    {
      std::cout << '\t' << "cl-" << i+apNum;
    }
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
  bool qosDisabled = false;
  uint32_t apNum = 1;
  uint32_t clNum = 2;
  double startTime = 10.0;
  double totalTime = 11.0;
  double monitorInterval = 1.0;
  std::string dataMode = "HeMcs0";
  double datarate = 1e6;
  double packetSize = 1e3;
  uint32_t tcp = 0;
  uint32_t rtscts = 0;
  double ratio = 1.0;
  bool randomTime = false;

  std::string routing = "static";
  std::string locationFile = "locationFile.txt";
  std::string routingFile = "routingFile.txt";
  std::string appFile = "appFile.txt";

  cmd.AddValue ("ofdmaEnabled", "", ofdmaEnabled);
  cmd.AddValue ("qosDisabled", "", qosDisabled);
  cmd.AddValue ("apNum", "", apNum);
  cmd.AddValue ("clNum", "", clNum);
  cmd.AddValue ("startTime", "Application start time, s.", startTime);
  cmd.AddValue ("totalTime", "Simulation time, s.", totalTime);
  cmd.AddValue ("monitorInterval", "Monitor interval, s.", monitorInterval);
  cmd.AddValue ("dataMode", "Mode for data frames.", dataMode);
  cmd.AddValue ("datarate", "Datarate, bps.", datarate);
  cmd.AddValue ("packetSize", "Packet size, bytes.", packetSize);
  cmd.AddValue ("tcp", "tcp protocl, 0 = udp, 1 = tcp", tcp);
  cmd.AddValue ("rtscts", "rtscts for csma, 0 = disable, 1 = enable", rtscts);
  cmd.AddValue ("routing", "Routing algo, none/aodv/olsr", routing);
  cmd.AddValue ("locationFile", "Location file", locationFile);
  cmd.AddValue ("routingFile", "Routing file for static routing", routingFile);
  cmd.AddValue ("appFile", "App file for static routing", appFile);
  cmd.AddValue ("ratio", "Ratio of location file inputs.", ratio);
  cmd.AddValue ("randomTime", "Randomness in application traffic", randomTime);
  cmd.Parse (argc, argv);

  // Config::SetDefault ("ns3::RegularWifiMac::VO_MaxAmsduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::VI_MaxAmsduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::BE_MaxAmsduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::BK_MaxAmsduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::VO_MaxAmpduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::VI_MaxAmpduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::BE_MaxAmpduSize", UintegerValue (0));
  // Config::SetDefault ("ns3::RegularWifiMac::BK_MaxAmpduSize", UintegerValue (0));

  std::vector<std::vector<double>> locations;
  if (!locationFile.empty ())
    {
      clNum = 0;
      double locationX = 0;
      double locationY = 0;
      double locationZ = 0;
      double inject = 0;
      std::ifstream fin (locationFile);
      if (!fin.is_open ())
        {
          NS_ASSERT_MSG (false, "missing location file");
        }
      while (fin >> locationX)
        {
          clNum++;
          fin >> locationY >> locationZ >> inject;
          locations.push_back (std::vector<double> {locationX * ratio, locationY * ratio, locationZ * ratio, inject});
        }
      fin.close ();
      clNum -= apNum;
    }

  // Create nodes and set mobility
  NodeContainer apNodes;
  apNodes.Create (apNum);
  NodeContainer clNodes;
  clNodes.Create (clNum);
  NodeContainer nodes;
  nodes.Add (apNodes);
  nodes.Add (clNodes);

  MobilityHelper mobility;
  if (locationFile.empty ())
    {
      mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (0.0),
                                    "MinY", DoubleValue (0.0));
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (apNodes);
      mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (40.0),
                                    "MinY", DoubleValue (0.0),
                                    "DeltaX", DoubleValue (40.0),
                                    "GridWidth", UintegerValue (clNum));
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (clNodes);
    }
  else
    {
      if (locations.size () < apNum+clNum)
        {
          NS_ASSERT_MSG (false, "not enough input locations");
        }
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      for (uint32_t i = 0; i < apNum+clNum; i++)
        {
          positionAlloc->Add (Vector (locations[i][0], locations[i][1], locations[i][2]));
        }
      mobility.SetPositionAllocator (positionAlloc);
      mobility.Install (nodes);
    }  

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
  mac.SetType ("ns3::AdhocWifiMac",
               "QosDisabled", BooleanValue (qosDisabled),
               "OfdmaTested", BooleanValue (true),
               "OfdmaSupported", BooleanValue (ofdmaEnabled));
  apDevices = wifi.Install (phy, mac, apNodes);
  clDevices = wifi.Install (phy, mac, clNodes);

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
  else if (routing == std::string ("static"))
    {
      Ipv4StaticRoutingHelper staticroute;
      stack.SetRoutingHelper (staticroute);
    }
  stack.Install (apNodes);
  stack.Install (clNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterfaces = address.Assign (apDevices);
  Ipv4InterfaceContainer clInterfaces = address.Assign (clDevices);
  Ipv4InterfaceContainer interfaces;
  interfaces.Add (apInterfaces);
  interfaces.Add (clInterfaces);

  std::vector<std::vector<uint32_t>> routes;
  if (routing == std::string ("static"))
    {
      NS_ASSERT_MSG (!routingFile.empty (), "missing routing file name");
      uint32_t nodeId = 0;
      uint32_t dstId = 0;
      uint32_t nextId = 0;
      std::ifstream fin (routingFile);
      if (!fin.is_open ())
        {
          NS_ASSERT_MSG (false, "missing routing file");
        }
      while (fin >> nodeId)
        {
          fin >> dstId >> nextId;
          routes.push_back (std::vector<uint32_t> {nodeId, dstId, nextId});
        }
      fin.close ();
      
      Ptr<Ipv4StaticRouting> staticRouting;
      Ipv4Address dstAdd = Ipv4Address ();
      Ipv4Address nextAdd = Ipv4Address ();
      for (uint32_t i = 0; i < routes.size (); i++)
        {
          nodeId = routes[i][0];
          dstId = routes[i][1];
          nextId = routes[i][2];
          if (i < clNum)
            {
              NS_ASSERT_MSG (nodeId < apNum, "wrong routing file format");
              NS_ASSERT_MSG (dstId >= apNum, "wrong routing file format");
              NS_ASSERT_MSG (dstId < apNum+clNum, "wrong routing file format");
            }
          staticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting> (nodes.Get (nodeId)->GetObject<Ipv4> ()->GetRoutingProtocol ());
          dstAdd = interfaces.GetAddress (dstId);
          nextAdd = interfaces.GetAddress (nextId);
          staticRouting->AddHostRouteTo (dstAdd, nextAdd, 1);
        }
    }

  // Install application
  ApplicationContainer apApplications;
  ApplicationContainer clApplications;
  ApplicationContainer trApplications;
  std::string appName = "ns3::UdpSocketFactory";
  if (tcp)
    {
      appName = "ns3::TcpSocketFactory";
    }

  std::vector<std::vector<double>> appInfo;
  if (!appFile.empty ())
    {
      double tempInject = 0;
      double tempSize = 0;
      std::ifstream fin (appFile);
      if (!fin.is_open ())
        {
          NS_ASSERT_MSG (false, "missing app file");
        }
      while (fin >> tempInject)
        {
          fin >> tempSize;
          appInfo.push_back (std::vector<double> {tempInject, tempSize});
        }
      fin.close ();
    }
  else
    {
      appInfo.push_back (std::vector<double> {1.0, 1.0});
    }

  for (uint32_t i = 0; i < clNum; i++)
    {
      uint32_t sinkId = i;
      uint32_t srcId = 0;
      uint16_t port = 5000+i;
      Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper server (appName.c_str (), localAddress);
      clApplications.Add (server.Install (clNodes.Get (sinkId)));
      packetSink.push_back (StaticCast<PacketSink> (clApplications.Get (i)));
      firstTotalRx.push_back (0);
      lastTotalRx.push_back (0);
      throughput.push_back (0);
      delay.push_back (0);
      loss.push_back (0);

      OnOffHelper client (appName.c_str (), Address ());
      AddressValue remoteAddress (InetSocketAddress (clInterfaces.GetAddress (sinkId), port));
      if (randomTime)
        {
          client.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.1|Bound=0.3]"));
          client.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.1|Bound=0.3]"));
        }
      else
        {
          client.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.1]"));
          client.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.1]"));
        }
      client.SetAttribute ("Remote", remoteAddress);
      for (uint32_t j = 0; j < appInfo.size (); j++)
        {
          client.SetAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) (datarate * appInfo[j][0] * locations[sinkId+apNum][3] + 1))));
          client.SetAttribute ("PacketSize", UintegerValue (packetSize * appInfo[j][1]));
          apApplications.Add (client.Install (apNodes.Get (srcId)));
        }

      client.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1|Bound=2]"));
      client.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1|Bound=2]"));
      client.SetAttribute ("DataRate", DataRateValue (DataRate (1e3)));
      client.SetAttribute ("PacketSize", UintegerValue (25));
      trApplications.Add (client.Install (apNodes.Get (srcId)));
    }
  
  apApplications.Start (Seconds (startTime));
  apApplications.Stop (Seconds (totalTime));
  clApplications.Start (Seconds (0.0));
  clApplications.Stop (Seconds (totalTime + 0.1));
  trApplications.Start (Seconds (startTime * 0.3));
  trApplications.Stop (Seconds (startTime * 0.8));

  Simulator::Schedule (Seconds (startTime), &PrintThroughputTitle, apNum);
  Simulator::Schedule (Seconds (startTime), &CalculateThroughput, monitorInterval);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Stop (Seconds (totalTime + 0.1));
  Simulator::Run ();

  for (uint32_t i = 0; i < apNum; i++)
    {
      Ptr<NetDevice> nd = apDevices.Get (i);
      Ptr<WifiNetDevice> wnd = nd->GetObject<WifiNetDevice> ();
      Ptr<WifiMac> wm = wnd->GetMac ();
      Ptr<RegularWifiMac> rwm = wm->GetObject<RegularWifiMac> ();
      rwm->PrintRuSentNum ();
    }
  for (uint32_t i = 0; i < clNum; i++)
    {
      Ptr<NetDevice> nd = clDevices.Get (i);
      Ptr<WifiNetDevice> wnd = nd->GetObject<WifiNetDevice> ();
      Ptr<WifiMac> wm = wnd->GetMac ();
      Ptr<RegularWifiMac> rwm = wm->GetObject<RegularWifiMac> ();
      rwm->PrintRuSentNum ();
    }

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
