/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Kirill Andreev <andreev@iitp.ru>
 *         Pavel Boyko <boyko@iitp.ru>
 */
 
#include "mesh-helper.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/mesh-point-device.h"
#include "ns3/wifi-net-device.h"
#include "ns3/minstrel-wifi-manager.h"
#include "ns3/minstrel-ht-wifi-manager.h"
#include "ns3/mesh-wifi-interface-mac.h"
#include "ns3/wifi-helper.h"
#include "ns3/ht-configuration.h"
#include "ns3/vht-configuration.h"
#include "ns3/he-configuration.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/qos-utils.h"

namespace ns3
{
MeshHelper::MeshHelper () :
  m_nInterfaces (1),
  m_spreadChannelPolicy (ZERO_CHANNEL),
  m_stack (0),
  m_standard (WIFI_PHY_STANDARD_80211a),
  m_selectQueueCallback (&SelectQueueByDSField)
{
}
MeshHelper::~MeshHelper ()
{
  m_stack = 0;
}
void
MeshHelper::SetSpreadInterfaceChannels (enum ChannelPolicy policy)
{
  m_spreadChannelPolicy = policy;
}
void
MeshHelper::SetStackInstaller (std::string type,
                               std::string n0, const AttributeValue &v0,
                               std::string n1, const AttributeValue &v1,
                               std::string n2, const AttributeValue &v2,
                               std::string n3, const AttributeValue &v3,
                               std::string n4, const AttributeValue &v4,
                               std::string n5, const AttributeValue &v5,
                               std::string n6, const AttributeValue &v6,
                               std::string n7, const AttributeValue &v7)
{
  m_stackFactory.SetTypeId (type);
  m_stackFactory.Set (n0, v0);
  m_stackFactory.Set (n1, v1);
  m_stackFactory.Set (n2, v2);
  m_stackFactory.Set (n3, v3);
  m_stackFactory.Set (n4, v4);
  m_stackFactory.Set (n5, v5);
  m_stackFactory.Set (n6, v6);
  m_stackFactory.Set (n7, v7);

  m_stack = m_stackFactory.Create<MeshStack> ();
  if (m_stack == 0)
    {
      NS_FATAL_ERROR ("Stack has not been created: " << type);
    }
}

void
MeshHelper::SetNumberOfInterfaces (uint32_t nInterfaces)
{
  m_nInterfaces = nInterfaces;
}
NetDeviceContainer
MeshHelper::Install (const WifiPhyHelper &phyHelper, NodeContainer c) const
{
  NetDeviceContainer devices;
  NS_ASSERT (m_stack != 0);
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      // Create a mesh point device
      Ptr<MeshPointDevice> mp = CreateObject<MeshPointDevice> ();
      node->AddDevice (mp);
      // Create wifi interfaces (single interface by default)
      for (uint32_t i = 0; i < m_nInterfaces; ++i)
        {
          uint32_t channel = 0;
          if (m_spreadChannelPolicy == ZERO_CHANNEL)
            {
              channel = 0;
            }
          if (m_spreadChannelPolicy == SPREAD_CHANNELS)
            {
              channel = i * 5;
            }
          Ptr<WifiNetDevice> iface = CreateInterface (phyHelper, node, channel);
          mp->AddInterface (iface);
        }
      if (!m_stack->InstallStack (mp))
        {
          NS_FATAL_ERROR ("Stack is not installed!");
        }
      devices.Add (mp);
    }
  return devices;
}
MeshHelper
MeshHelper::Default (void)
{
  MeshHelper helper;
  helper.SetMacType ();
  helper.SetRemoteStationManager ("ns3::ArfWifiManager");
  helper.SetSpreadInterfaceChannels (SPREAD_CHANNELS);
  return helper;
}

void
MeshHelper::SetMacType (std::string n0, const AttributeValue &v0,
                        std::string n1, const AttributeValue &v1,
                        std::string n2, const AttributeValue &v2,
                        std::string n3, const AttributeValue &v3,
                        std::string n4, const AttributeValue &v4,
                        std::string n5, const AttributeValue &v5,
                        std::string n6, const AttributeValue &v6,
                        std::string n7, const AttributeValue &v7)
{
  m_mac.SetTypeId ("ns3::MeshWifiInterfaceMac");
  m_mac.Set (n0, v0);
  m_mac.Set (n1, v1);
  m_mac.Set (n2, v2);
  m_mac.Set (n3, v3);
  m_mac.Set (n4, v4);
  m_mac.Set (n5, v5);
  m_mac.Set (n6, v6);
  m_mac.Set (n7, v7);
}
void
MeshHelper::SetRemoteStationManager (std::string type,
                                     std::string n0, const AttributeValue &v0,
                                     std::string n1, const AttributeValue &v1,
                                     std::string n2, const AttributeValue &v2,
                                     std::string n3, const AttributeValue &v3,
                                     std::string n4, const AttributeValue &v4,
                                     std::string n5, const AttributeValue &v5,
                                     std::string n6, const AttributeValue &v6,
                                     std::string n7, const AttributeValue &v7)
{
  m_stationManager = ObjectFactory ();
  m_stationManager.SetTypeId (type);
  m_stationManager.Set (n0, v0);
  m_stationManager.Set (n1, v1);
  m_stationManager.Set (n2, v2);
  m_stationManager.Set (n3, v3);
  m_stationManager.Set (n4, v4);
  m_stationManager.Set (n5, v5);
  m_stationManager.Set (n6, v6);
  m_stationManager.Set (n7, v7);
}
void 
MeshHelper::SetStandard (enum WifiPhyStandard standard)
{
  m_standard = standard;
}

Ptr<WifiNetDevice>
MeshHelper::CreateInterface (const WifiPhyHelper &phyHelper, Ptr<Node> node, uint16_t channelId) const
{
  Ptr<WifiNetDevice> device = CreateObject<WifiNetDevice> ();
  if (m_standard >= WIFI_PHY_STANDARD_80211n_2_4GHZ)
    {
      Ptr<HtConfiguration> htConfiguration = CreateObject<HtConfiguration> ();
      device->SetHtConfiguration (htConfiguration);
    }
  if ((m_standard == WIFI_PHY_STANDARD_80211ac) || (m_standard == WIFI_PHY_STANDARD_80211ax_5GHZ))
    {
      Ptr<VhtConfiguration> vhtConfiguration = CreateObject<VhtConfiguration> ();
      device->SetVhtConfiguration (vhtConfiguration);
    }
  if (m_standard >= WIFI_PHY_STANDARD_80211ax_2_4GHZ)
    {
      Ptr<HeConfiguration> heConfiguration = CreateObject<HeConfiguration> ();
      device->SetHeConfiguration (heConfiguration);
    }

  Ptr<MeshWifiInterfaceMac> mac = m_mac.Create<MeshWifiInterfaceMac> ();
  NS_ASSERT (mac != 0);
  mac->SetSsid (Ssid ());
  mac->SetDevice (device);
  Ptr<WifiRemoteStationManager> manager = m_stationManager.Create<WifiRemoteStationManager> ();
  NS_ASSERT (manager != 0);
  Ptr<WifiPhy> phy = phyHelper.Create (node, device);
  mac->SetAddress (Mac48Address::Allocate ());
  mac->ConfigureStandard (m_standard);
  phy->ConfigureStandard (m_standard);
  device->SetMac (mac);
  device->SetPhy (phy);
  device->SetRemoteStationManager (manager);
  node->AddDevice (device);
  // mac->SwitchFrequencyChannel (channelId);
  // Aggregate a NetDeviceQueueInterface object if a RegularWifiMac is installed
  Ptr<RegularWifiMac> rmac = DynamicCast<RegularWifiMac> (mac);
  if (rmac)
    {
      Ptr<NetDeviceQueueInterface> ndqi;
      BooleanValue qosSupported;
      PointerValue ptr;
      Ptr<WifiMacQueue> wmq;

      rmac->GetAttributeFailSafe ("QosSupported", qosSupported);
      if (qosSupported.Get ())
        {
          ndqi = CreateObjectWithAttributes<NetDeviceQueueInterface> ("NTxQueues",
                                                                      UintegerValue (4));

          rmac->GetAttributeFailSafe ("BE_Txop", ptr);
          wmq = ptr.Get<QosTxop> ()->GetWifiMacQueue ();
          ndqi->GetTxQueue (0)->ConnectQueueTraces (wmq);

          rmac->GetAttributeFailSafe ("BK_Txop", ptr);
          wmq = ptr.Get<QosTxop> ()->GetWifiMacQueue ();
          ndqi->GetTxQueue (1)->ConnectQueueTraces (wmq);

          rmac->GetAttributeFailSafe ("VI_Txop", ptr);
          wmq = ptr.Get<QosTxop> ()->GetWifiMacQueue ();
          ndqi->GetTxQueue (2)->ConnectQueueTraces (wmq);

          rmac->GetAttributeFailSafe ("VO_Txop", ptr);
          wmq = ptr.Get<QosTxop> ()->GetWifiMacQueue ();
          ndqi->GetTxQueue (3)->ConnectQueueTraces (wmq);
          ndqi->SetSelectQueueCallback (m_selectQueueCallback);
        }
      else
        {
          ndqi = CreateObject<NetDeviceQueueInterface> ();

          rmac->GetAttributeFailSafe ("Txop", ptr);
          wmq = ptr.Get<Txop> ()->GetWifiMacQueue ();
          ndqi->GetTxQueue (0)->ConnectQueueTraces (wmq);
        }
      device->AggregateObject (ndqi);
    }
  return device;
}
void
MeshHelper::Report (const ns3::Ptr<ns3::NetDevice>& device, std::ostream& os)
{
  NS_ASSERT (m_stack != 0);
  Ptr<MeshPointDevice> mp = device->GetObject<MeshPointDevice> ();
  NS_ASSERT (mp != 0);
  std::vector<Ptr<NetDevice> > ifaces = mp->GetInterfaces ();
  os << "<MeshPointDevice time=\"" << Simulator::Now ().GetSeconds () << "\" address=\""
     << Mac48Address::ConvertFrom (mp->GetAddress ()) << "\">\n";
  m_stack->Report (mp, os);
  os << "</MeshPointDevice>\n";
}
void
MeshHelper::ResetStats (const ns3::Ptr<ns3::NetDevice>& device)
{
  NS_ASSERT (m_stack != 0);
  Ptr<MeshPointDevice> mp = device->GetObject<MeshPointDevice> ();
  NS_ASSERT (mp != 0);
  m_stack->ResetStats (mp);
}
int64_t
MeshHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<MeshPointDevice> mpd = DynamicCast<MeshPointDevice> (netDevice);
      Ptr<WifiNetDevice> wifi;
      Ptr<MeshWifiInterfaceMac> mac;
      if (mpd)
        {
          // To access, we need the underlying WifiNetDevices
          std::vector<Ptr<NetDevice> > ifaces = mpd->GetInterfaces ();
          for (std::vector<Ptr<NetDevice> >::iterator i = ifaces.begin (); i != ifaces.end (); i++)
            {
              wifi = DynamicCast<WifiNetDevice> (*i);
           
              // Handle any random numbers in the PHY objects.
              currentStream += wifi->GetPhy ()->AssignStreams (currentStream);

              // Handle any random numbers in the station managers.
              Ptr<WifiRemoteStationManager> manager = wifi->GetRemoteStationManager ();
              Ptr<MinstrelWifiManager> minstrel = DynamicCast<MinstrelWifiManager> (manager);
              if (minstrel)
                {
                  currentStream += minstrel->AssignStreams (currentStream);
                }

              Ptr<MinstrelHtWifiManager> minstrelHt = DynamicCast<MinstrelHtWifiManager> (manager);
              if (minstrelHt)
                {
                  currentStream += minstrelHt->AssignStreams (currentStream);
                }

              // Handle any random numbers in the mesh mac and plugins
              mac = DynamicCast<MeshWifiInterfaceMac> (wifi->GetMac ());
              if (mac)
                {
                  currentStream += mac->AssignStreams (currentStream);
                }
              Ptr<RegularWifiMac> rmac = DynamicCast<RegularWifiMac> (mac);
              if (rmac)
                {
                  PointerValue ptr;
                  rmac->GetAttribute ("Txop", ptr);
                  Ptr<Txop> txop = ptr.Get<Txop> ();
                  currentStream += txop->AssignStreams (currentStream);

                  rmac->GetAttribute ("VO_Txop", ptr);
                  Ptr<QosTxop> vo_txop = ptr.Get<QosTxop> ();
                  currentStream += vo_txop->AssignStreams (currentStream);

                  rmac->GetAttribute ("VI_Txop", ptr);
                  Ptr<QosTxop> vi_txop = ptr.Get<QosTxop> ();
                  currentStream += vi_txop->AssignStreams (currentStream);

                  rmac->GetAttribute ("BE_Txop", ptr);
                  Ptr<QosTxop> be_txop = ptr.Get<QosTxop> ();
                  currentStream += be_txop->AssignStreams (currentStream);

                  rmac->GetAttribute ("BK_Txop", ptr);
                  Ptr<QosTxop> bk_txop = ptr.Get<QosTxop> ();
                  currentStream += bk_txop->AssignStreams (currentStream);
               }
            }
        }
    }
  return (currentStream - stream);
}

void
MeshHelper::SetSelectQueueCallback (SelectQueueCallback f)
{
  m_selectQueueCallback = f;
}

} // namespace ns3

