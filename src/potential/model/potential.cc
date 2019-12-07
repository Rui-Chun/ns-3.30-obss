/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Universita' di Firenze, Italy
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
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include <iomanip>
#include <algorithm>
#include <functional>
#include <map>
#include "potential.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/unused.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-route.h"
#include "ns3/node.h"
#include "ns3/names.h"
#include "ns3/potential-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-packet-info-tag.h"
#include "ns3/loopback-net-device.h"
#include "ns3/double.h"
#include "ns3/boolean.h"

#define POTENTIAL_ALL_NODE "224.0.0.19"
#define POTENTIAL_PORT 1520

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Potential");

NS_OBJECT_ENSURE_REGISTERED (Potential);

Potential::Potential ()
  : m_ipv4 (0),
    m_initialized (false)
{
  m_rng = CreateObject<UniformRandomVariable> ();
}

Potential::~Potential ()
{
}

TypeId
Potential::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Potential")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Internet")
    .AddConstructor<Potential> ()
    .AddAttribute ("UnsolicitedRoutingUpdate", "The time between two Unsolicited Routing Updates.",
                   TimeValue (Seconds(5)),
                   MakeTimeAccessor (&Potential::m_unsolicitedUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("StartupDelay", "Maximum random delay for protocol startup (send route requests).",
                   TimeValue (Seconds(1)),
                   MakeTimeAccessor (&Potential::m_startupDelay),
                   MakeTimeChecker ())
    .AddAttribute ("TimeoutDelay", "The delay to invalidate a route.",
                   TimeValue (Seconds(180)),
                   MakeTimeAccessor (&Potential::m_timeoutDelay),
                   MakeTimeChecker ())
    .AddAttribute ("GarbageCollectionDelay", "The delay to delete an expired route.",
                   TimeValue (Seconds(120)),
                   MakeTimeAccessor (&Potential::m_garbageCollectionDelay),
                   MakeTimeChecker ())
    .AddAttribute ("MinTriggeredCooldown", "Min cooldown delay after a Triggered Update.",
                   TimeValue (Seconds(1)),
                   MakeTimeAccessor (&Potential::m_minTriggeredUpdateDelay),
                   MakeTimeChecker ())
    .AddAttribute ("MaxTriggeredCooldown", "Max cooldown delay after a Triggered Update.",
                   TimeValue (Seconds(5)),
                   MakeTimeAccessor (&Potential::m_maxTriggeredUpdateDelay),
                   MakeTimeChecker ())
    .AddAttribute ("LinkDownValue", "Value for link down in count to infinity.",
                   UintegerValue (16),
                   MakeUintegerAccessor (&Potential::m_linkDown),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("FixedPotential", "Updatable potential or not",
                   BooleanValue (true),
                   MakeBooleanAccessor (&Potential::m_fixedPotential),
                   MakeBooleanChecker ())
    .AddAttribute ("Potential", "Initial potential value",
                   UintegerValue (0),
                   MakeUintegerAccessor (&Potential::m_potential),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Conductivity", "Potential conductivity value",
                   DoubleValue (0.25),
                   MakeDoubleAccessor (&Potential::m_conductivity),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

int64_t
Potential::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);

  m_rng->SetStream (stream);
  return 1;
}

void
Potential::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  bool addedGlobal = false;

  m_initialized = true;

  Time delay = m_unsolicitedUpdate + Seconds (m_rng->GetValue (0, 0.5*m_unsolicitedUpdate.GetSeconds ()) );
  m_nextUnsolicitedUpdate = Simulator::Schedule (delay, &Potential::SendUnsolicitedRouteUpdate, this);


  for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces (); i++)
    {
      Ptr<LoopbackNetDevice> check = DynamicCast<LoopbackNetDevice> (m_ipv4->GetNetDevice (i));
      if (check)
        {
          continue;
        }

      bool activeInterface = false;
      if (m_interfaceExclusions.find (i) == m_interfaceExclusions.end ())
        {
          activeInterface = true;
          m_ipv4->SetForwarding (i, true);
        }

      for (uint32_t j = 0; j < m_ipv4->GetNAddresses (i); j++)
        {
          Ipv4InterfaceAddress address = m_ipv4->GetAddress (i, j);
          if (address.GetScope() != Ipv4InterfaceAddress::HOST && activeInterface == true)
            {
              NS_LOG_LOGIC ("POTENTIAL: adding socket to " << address.GetLocal ());
              TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
              Ptr<Node> theNode = GetObject<Node> ();
              Ptr<Socket> socket = Socket::CreateSocket (theNode, tid);
              InetSocketAddress local = InetSocketAddress (address.GetLocal (), POTENTIAL_PORT);
              socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
              int ret = socket->Bind (local);
              NS_ASSERT_MSG (ret == 0, "Bind unsuccessful");
              socket->SetIpRecvTtl (true);
              m_sendSocketList[socket] = i;
            }
          else if (m_ipv4->GetAddress (i, j).GetScope() == Ipv4InterfaceAddress::GLOBAL)
            {
              addedGlobal = true;
            }
        }
    }

  if (!m_recvSocket)
    {
      NS_LOG_LOGIC ("POTENTIAL: adding receiving socket");
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Node> theNode = GetObject<Node> ();
      m_recvSocket = Socket::CreateSocket (theNode, tid);
      InetSocketAddress local = InetSocketAddress (POTENTIAL_ALL_NODE, POTENTIAL_PORT);
      m_recvSocket->Bind (local);
      m_recvSocket->SetRecvCallback (MakeCallback (&Potential::Receive, this));
      m_recvSocket->SetIpRecvTtl (true);
      m_recvSocket->SetRecvPktInfo (true);
    }


  if (addedGlobal)
    {
      Time delay = Seconds (m_rng->GetValue (m_minTriggeredUpdateDelay.GetSeconds (), m_maxTriggeredUpdateDelay.GetSeconds ()));
      m_nextTriggeredUpdate = Simulator::Schedule (delay, &Potential::DoSendRouteUpdate, this, false);
    }

  delay = Seconds (m_rng->GetValue (0.01, m_startupDelay.GetSeconds ()));
  m_nextTriggeredUpdate = Simulator::Schedule (delay, &Potential::SendRouteRequest, this);

  Ipv4RoutingProtocol::DoInitialize ();
}

Ptr<Ipv4Route>
Potential::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << header << oif);

  Ipv4Address destination = header.GetDestination ();
  Ptr<Ipv4Route> rtentry = 0;

  if (destination.IsMulticast ())
    {
      // Note:  Multicast routes for outbound packets are stored in the
      // normal unicast table.  An implication of this is that it is not
      // possible to source multicast datagrams on multiple interfaces.
      // This is a well-known property of sockets implementation on
      // many Unix variants.
      // So, we just log it and fall through to LookupStatic ()
      NS_LOG_LOGIC ("RouteOutput (): Multicast destination");
    }

  rtentry = Lookup (destination, oif);
  if (rtentry)
    {
      sockerr = Socket::ERROR_NOTERROR;
    }
  else
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
    }
  return rtentry;
}

bool
Potential::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                        UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                        LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header << header.GetSource () << header.GetDestination () << idev);

  NS_ASSERT (m_ipv4 != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  Ipv4Address dst = header.GetDestination ();

  if (m_ipv4->IsDestinationAddress (header.GetDestination (), iif))
    {
      if (!lcb.IsNull ())
        {
          NS_LOG_LOGIC ("Local delivery to " << header.GetDestination ());
          lcb (p, header, iif);
          return true;
        }
      else
        {
          // The local delivery callback is null.  This may be a multicast
          // or broadcast packet, so return false so that another
          // multicast routing protocol can handle it.  It should be possible
          // to extend this to explicitly check whether it is a unicast
          // packet, and invoke the error callback if so
          return false;
        }
    }

  if (dst.IsMulticast ())
    {
      NS_LOG_LOGIC ("Multicast route not supported by POTENTIAL");
      return false; // Let other routing protocols try to handle this
    }

  if (header.GetDestination ().IsBroadcast ())
    {
      NS_LOG_LOGIC ("Dropping packet not for me and with dst Broadcast");
      if (!ecb.IsNull ())
        {
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
      return false;
    }

  // Check if input device supports IP forwarding
  if (m_ipv4->IsForwarding (iif) == false)
    {
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      if (!ecb.IsNull ())
        {
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
      return true;
    }
  // Next, try to find a route
  NS_LOG_LOGIC ("Unicast destination");
  Ptr<Ipv4Route> rtentry = Lookup (header.GetDestination ());

  if (rtentry != 0)
    {
      NS_LOG_LOGIC ("Found unicast destination - calling unicast callback");
      ucb (rtentry, p, header);  // unicast forwarding callback
      return true;
    }
  else
    {
      NS_LOG_LOGIC ("Did not find unicast destination - returning false");
      return false; // Let other routing protocols try to handle this
    }
}

void
Potential::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);

  Ptr<LoopbackNetDevice> check = DynamicCast<LoopbackNetDevice> (m_ipv4->GetNetDevice (i));
  if (check)
    {
      return;
    }

  for (uint32_t j = 0; j < m_ipv4->GetNAddresses (i); j++)
    {
      Ipv4InterfaceAddress address = m_ipv4->GetAddress (i, j);
      Ipv4Mask networkMask = address.GetMask ();
      Ipv4Address networkAddress = address.GetLocal ().CombineMask (networkMask);

      if (address.GetScope () == Ipv4InterfaceAddress::GLOBAL)
        {
          AddNetworkRouteTo (networkAddress, networkMask, i);
        }
    }

  if (!m_initialized)
    {
      return;
    }


  bool sendSocketFound = false;
  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
    {
      if (iter->second == i)
        {
          sendSocketFound = true;
          break;
        }
    }

  bool activeInterface = false;
  if (m_interfaceExclusions.find (i) == m_interfaceExclusions.end ())
    {
      activeInterface = true;
      m_ipv4->SetForwarding (i, true);
    }

  for (uint32_t j = 0; j < m_ipv4->GetNAddresses (i); j++)
    {
      Ipv4InterfaceAddress address = m_ipv4->GetAddress (i, j);

      if (address.GetScope() != Ipv4InterfaceAddress::HOST && sendSocketFound == false && activeInterface == true)
        {
          NS_LOG_LOGIC ("POTENTIAL: adding sending socket to " << address.GetLocal ());
          TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
          Ptr<Node> theNode = GetObject<Node> ();
          Ptr<Socket> socket = Socket::CreateSocket (theNode, tid);
          InetSocketAddress local = InetSocketAddress (address.GetLocal (), POTENTIAL_PORT);
          socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
          socket->Bind (local);
          socket->SetIpRecvTtl (true);
          m_sendSocketList[socket] = i;
        }
      if (address.GetScope () == Ipv4InterfaceAddress::GLOBAL)
        {
          SendTriggeredRouteUpdate ();
        }
    }

  if (!m_recvSocket)
    {
      NS_LOG_LOGIC ("POTENTIAL: adding receiving socket");
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Node> theNode = GetObject<Node> ();
      m_recvSocket = Socket::CreateSocket (theNode, tid);
      InetSocketAddress local = InetSocketAddress (POTENTIAL_ALL_NODE, POTENTIAL_PORT);
      m_recvSocket->Bind (local);
      m_recvSocket->SetRecvCallback (MakeCallback (&Potential::Receive, this));
      m_recvSocket->SetIpRecvTtl (true);
      m_recvSocket->SetRecvPktInfo (true);
    }
}

void
Potential::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);

  /* remove all routes that are going through this interface */
  for (RoutesI it = m_routes.begin (); it != m_routes.end (); it++)
    {
      if (it->first->GetInterface () == interface)
        {
          InvalidateRoute (it->first);
        }
    }

  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
    {
      NS_LOG_INFO ("Checking socket for interface " << interface);
      if (iter->second == interface)
        {
          NS_LOG_INFO ("Removed socket for interface " << interface);
          iter->first->Close ();
          m_sendSocketList.erase (iter);
          break;
        }
    }

  if (m_interfaceExclusions.find (interface) == m_interfaceExclusions.end ())
    {
      SendTriggeredRouteUpdate ();
    }
}

void
Potential::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);

  if (!m_ipv4->IsUp (interface))
    {
      return;
    }

  if (m_interfaceExclusions.find (interface) != m_interfaceExclusions.end ())
    {
      return;
    }

  Ipv4Address networkAddress = address.GetLocal ().CombineMask (address.GetMask ());
  Ipv4Mask networkMask = address.GetMask ();

  if (address.GetScope () == Ipv4InterfaceAddress::GLOBAL)
    {
      AddNetworkRouteTo (networkAddress, networkMask, interface);
    }

  SendTriggeredRouteUpdate ();
}

void
Potential::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);

  if (!m_ipv4->IsUp (interface))
    {
      return;
    }

  if (address.GetScope() != Ipv4InterfaceAddress::GLOBAL)
    {
      return;
    }

  Ipv4Address networkAddress = address.GetLocal ().CombineMask (address.GetMask ());
  Ipv4Mask networkMask = address.GetMask ();

  // Remove all routes that are going through this interface
  // which reference this network
  for (RoutesI it = m_routes.begin (); it != m_routes.end (); it++)
    {
      if (it->first->GetInterface () == interface
          && it->first->IsNetwork ()
          && it->first->GetDestNetwork () == networkAddress
          && it->first->GetDestNetworkMask () == networkMask)
        {
          InvalidateRoute (it->first);
        }
    }

  if (m_interfaceExclusions.find (interface) == m_interfaceExclusions.end ())
    {
      SendTriggeredRouteUpdate ();
    }

}

void
Potential::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);

  NS_ASSERT (m_ipv4 == 0 && ipv4 != 0);
  uint32_t i = 0;
  m_ipv4 = ipv4;

  for (i = 0; i < m_ipv4->GetNInterfaces (); i++)
    {
      if (m_ipv4->IsUp (i))
        {
          NotifyInterfaceUp (i);
        }
      else
        {
          NotifyInterfaceDown (i);
        }
    }
}

void
Potential::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  NS_LOG_FUNCTION (this << stream);

  std::ostream* os = stream->GetStream ();

  *os << "Node: " << m_ipv4->GetObject<Node> ()->GetId ()
      << ", Time: " << Now().As (unit)
      << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
      << ", Fixed: " << m_fixedPotential
      << ", Potential: " << m_potential << std::endl;

  for (auto it : m_neighbors)
    {
      *os << it.first << '-' << std::get<0> (it.second) << '\t';
    }
    
  *os << ", IPv4 POTENTIAL table" << std::endl;

  if (!m_routes.empty ())
    {
      *os << "Destination     Gateway         Genmask         Flags Metric Ref    Use Iface" << std::endl;
      for (RoutesCI it = m_routes.begin (); it != m_routes.end (); it++)
        {
          PotentialRoutingTableEntry* route = it->first;
          PotentialRoutingTableEntry::Status_e status = route->GetRouteStatus();

          if (status == PotentialRoutingTableEntry::POTENTIAL_VALID)
            {
              std::ostringstream dest, gw, mask, flags;
              dest << route->GetDest ();
              *os << std::setiosflags (std::ios::left) << std::setw (16) << dest.str ();
              gw << route->GetGateway ();
              *os << std::setiosflags (std::ios::left) << std::setw (16) << gw.str ();
              mask << route->GetDestNetworkMask ();
              *os << std::setiosflags (std::ios::left) << std::setw (16) << mask.str ();
              flags << "U";
              if (route->IsHost ())
                {
                  flags << "HS";
                }
              else if (route->IsGateway ())
                {
                  flags << "GS";
                }
              *os << std::setiosflags (std::ios::left) << std::setw (6) << flags.str ();
              *os << std::setiosflags (std::ios::left) << std::setw (7) << int(route->GetRouteMetric ());
              // Ref ct not implemented
              *os << "-" << "      ";
              // Use not implemented
              *os << "-" << "   ";
              if (Names::FindName (m_ipv4->GetNetDevice (route->GetInterface ())) != "")
                {
                  *os << Names::FindName (m_ipv4->GetNetDevice (route->GetInterface ()));
                }
              else
                {
                  *os << route->GetInterface ();
                }
              *os << std::endl;
            }
        }
    }
  *os << std::endl;
}

void
Potential::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  for (RoutesI j = m_routes.begin ();  j != m_routes.end (); j = m_routes.erase (j))
    {
      delete j->first;
    }
  m_routes.clear ();

  m_nextTriggeredUpdate.Cancel ();
  m_nextUnsolicitedUpdate.Cancel ();
  m_nextTriggeredUpdate = EventId ();
  m_nextUnsolicitedUpdate = EventId ();

  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
    {
      iter->first->Close ();
    }
  m_sendSocketList.clear ();

  m_recvSocket->Close ();
  m_recvSocket = 0;

  m_ipv4 = 0;

  Ipv4RoutingProtocol::DoDispose ();
}


Ptr<Ipv4Route>
Potential::Lookup (Ipv4Address dst, Ptr<NetDevice> interface)
{
  NS_LOG_FUNCTION (this << dst << interface);

  Ptr<Ipv4Route> rtentry = 0;
  uint16_t longestMask = 0;

  /* when sending on local multicast, there have to be interface specified */
  if (dst.IsLocalMulticast ())
    {
      NS_ASSERT_MSG (interface, "Try to send on local multicast address, and no interface index is given!");
      rtentry = Create<Ipv4Route> ();
      rtentry->SetSource (m_ipv4->SourceAddressSelection (m_ipv4->GetInterfaceForDevice (interface), dst));
      rtentry->SetDestination (dst);
      rtentry->SetGateway (Ipv4Address::GetZero ());
      rtentry->SetOutputDevice (interface);
      return rtentry;
    }

  for (RoutesI it = m_routes.begin (); it != m_routes.end (); it++)
    {
      PotentialRoutingTableEntry* j = it->first;

      if (j->GetRouteStatus () == PotentialRoutingTableEntry::POTENTIAL_VALID)
        {
          Ipv4Mask mask = j->GetDestNetworkMask ();
          uint16_t maskLen = mask.GetPrefixLength ();
          Ipv4Address entry = j->GetDestNetwork ();

          NS_LOG_LOGIC ("Searching for route to " << dst << ", mask length " << maskLen);

          if (mask.IsMatch (dst, entry))
            {
              NS_LOG_LOGIC ("Found global network route " << j << ", mask length " << maskLen);

              /* if interface is given, check the route will output on this interface */
              if (!interface || interface == m_ipv4->GetNetDevice (j->GetInterface ()))
                {
                  if (maskLen < longestMask)
                    {
                      NS_LOG_LOGIC ("Previous match longer, skipping");
                      continue;
                    }

                  longestMask = maskLen;

                  Ipv4RoutingTableEntry* route = j;
                  uint32_t interfaceIdx = route->GetInterface ();
                  rtentry = Create<Ipv4Route> ();

                  if (route->GetDest ().IsAny ()) /* default route */
                    {
                      rtentry->SetSource (m_ipv4->SourceAddressSelection (interfaceIdx, route->GetGateway ()));
                    }
                  else
                    {
                      rtentry->SetSource (m_ipv4->SourceAddressSelection (interfaceIdx, route->GetDest ()));
                    }

                  rtentry->SetDestination (route->GetDest ());
                  rtentry->SetGateway (route->GetGateway ());
                  rtentry->SetOutputDevice (m_ipv4->GetNetDevice (interfaceIdx));
                }
            }
        }
    }

  if (rtentry)
    {
      NS_LOG_LOGIC ("Matching route via " << rtentry->GetDestination () << " (through " << rtentry->GetGateway () << ") at the end");
    }
  return rtentry;
}

void
Potential::AddNetworkRouteTo (Ipv4Address network, Ipv4Mask networkPrefix, Ipv4Address nextHop, uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkPrefix << nextHop << interface);

  PotentialRoutingTableEntry* route = new PotentialRoutingTableEntry (network, networkPrefix, nextHop, interface);
  route->SetRouteMetric (1);
  route->SetRouteStatus (PotentialRoutingTableEntry::POTENTIAL_VALID);
  route->SetRouteChanged (true);

  for (auto it = m_routes.begin (); it != m_routes.end (); )
    {
      if (it->first->GetDest () == network)
        {
          it = m_routes.erase (it);
        }
      else
        {
          it++;
        }
    }

  m_routes.push_back (std::make_pair (route, EventId ()));
}

void
Potential::AddNetworkRouteTo (Ipv4Address network, Ipv4Mask networkPrefix, uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkPrefix << interface);

  PotentialRoutingTableEntry* route = new PotentialRoutingTableEntry (network, networkPrefix, interface);
  route->SetRouteMetric (1);
  route->SetRouteStatus (PotentialRoutingTableEntry::POTENTIAL_VALID);
  route->SetRouteChanged (true);

  for (auto it = m_routes.begin (); it != m_routes.end (); )
    {
      if (it->first->GetDest () == network)
        {
          it = m_routes.erase (it);
        }
      else
        {
          it++;
        }
    }

  m_routes.push_back (std::make_pair (route, EventId ()));
}

void
Potential::InvalidateRoute (PotentialRoutingTableEntry *route)
{
  NS_LOG_FUNCTION (this << *route);

  for (RoutesI it = m_routes.begin (); it != m_routes.end (); it++)
    {
      if (it->first == route)
        {
          route->SetRouteStatus (PotentialRoutingTableEntry::POTENTIAL_INVALID);
          route->SetRouteMetric (m_linkDown);
          route->SetRouteChanged (true);
          if (it->second.IsRunning ())
            {
              it->second.Cancel ();
            }
          it->second = Simulator::Schedule (m_garbageCollectionDelay, &Potential::DeleteRoute, this, route);
          return;
        }
    }
  NS_ABORT_MSG ("POTENTIAL::InvalidateRoute - cannot find the route to update");
}

void
Potential::DeleteRoute (PotentialRoutingTableEntry *route)
{
  NS_LOG_FUNCTION (this << *route);

  for (RoutesI it = m_routes.begin (); it != m_routes.end (); it++)
    {
      if (it->first == route)
        {
          delete route;
          m_routes.erase (it);
          return;
        }
    }
  NS_ABORT_MSG ("POTENTIAL::DeleteRoute - cannot find the route to delete");
}


void
Potential::Receive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Address sender;
  Ptr<Packet> packet = socket->RecvFrom (sender);
  InetSocketAddress senderAddr = InetSocketAddress::ConvertFrom (sender);
  NS_LOG_INFO ("Received " << *packet << " from " << senderAddr);

  Ipv4Address senderAddress = senderAddr.GetIpv4 ();
  uint16_t senderPort = senderAddr.GetPort ();

  Ipv4PacketInfoTag interfaceInfo;
  if (!packet->RemovePacketTag (interfaceInfo))
    {
      NS_ABORT_MSG ("No incoming interface on POTENTIAL message, aborting.");
    }
  uint32_t incomingIf = interfaceInfo.GetRecvIf ();
  Ptr<Node> node = this->GetObject<Node> ();
  Ptr<NetDevice> dev = node->GetDevice (incomingIf);
  uint32_t ipInterfaceIndex = m_ipv4->GetInterfaceForDevice (dev);

  SocketIpTtlTag hoplimitTag;
  if (!packet->RemovePacketTag (hoplimitTag))
    {
      NS_ABORT_MSG ("No incoming Hop Count on POTENTIAL message, aborting.");
    }
  uint8_t hopLimit = hoplimitTag.GetTtl ();

  int32_t interfaceForAddress = m_ipv4->GetInterfaceForAddress (senderAddress);
  if (interfaceForAddress != -1)
    {
      NS_LOG_LOGIC ("Ignoring a packet sent by myself.");
      return;
    }

  PotentialHeader hdr;
  packet->RemoveHeader (hdr);

  if (hdr.GetCommand () == PotentialHeader::RESPONSE)
    {
      HandleResponses (hdr, senderAddress, ipInterfaceIndex, hopLimit);
    }
  else if (hdr.GetCommand () == PotentialHeader::REQUEST)
    {
      HandleRequests (hdr, senderAddress, senderPort, ipInterfaceIndex, hopLimit);
    }
  else
    {
      NS_LOG_LOGIC ("Ignoring message with unknown command: " << int (hdr.GetCommand ()));
    }
  return;
}

void
Potential::HandleRequests (PotentialHeader requestHdr, Ipv4Address senderAddress, uint16_t senderPort, uint32_t incomingInterface, uint8_t hopLimit)
{
  NS_LOG_FUNCTION (this << senderAddress << int (senderPort) << incomingInterface << int (hopLimit) << requestHdr);

  if (m_interfaceExclusions.find (incomingInterface) != m_interfaceExclusions.end ())
    {
      NS_LOG_LOGIC ("Ignoring an update message from an excluded interface: " << incomingInterface);
      return;
    }

  Ptr<Packet> p = Create<Packet> ();
  SocketIpTtlTag tag;
  p->RemovePacketTag (tag);
  tag.SetTtl (1);
  p->AddPacketTag (tag);

  PotentialHeader hdr;
  hdr.SetCommand (PotentialHeader::RESPONSE);
  hdr.SetPotential (m_potential);
  p->AddHeader (hdr);
  NS_LOG_DEBUG ("SendTo: " << *p);
  m_recvSocket->SendTo (p, 0, InetSocketAddress (senderAddress, senderPort));

  UpdateNeighbor (senderAddress, requestHdr.GetPotential (), incomingInterface);
}

void
Potential::HandleResponses (PotentialHeader hdr, Ipv4Address senderAddress, uint32_t incomingInterface, uint8_t hopLimit)
{
  NS_LOG_FUNCTION (this << senderAddress << incomingInterface << int (hopLimit) << hdr);

  if (m_interfaceExclusions.find (incomingInterface) != m_interfaceExclusions.end ())
    {
      NS_LOG_LOGIC ("Ignoring an update message from an excluded interface: " << incomingInterface);
      return;
    }

  UpdateNeighbor (senderAddress, hdr.GetPotential (), incomingInterface);
}

void
Potential::UpdateNeighbor (Ipv4Address neighbor, uint32_t potential, uint32_t incomingInterface)
{
  NS_LOG_FUNCTION (this);
  auto it = m_neighbors.find (neighbor);
  if (it == m_neighbors.end ())
    {
      m_neighbors[neighbor] = std::make_tuple (potential, incomingInterface, Simulator::Now ());
      UpdatePotential ();
    }
  else
    {
      std::get<2> (it->second) = Simulator::Now ();
      if (std::get<0> (it->second) != potential)
        {
          std::get<0> (it->second) = potential;
          UpdatePotential ();
        }      
    }
}

void
Potential::UpdatePotential ()
{
  if (m_fixedPotential)
    {
      return;
    }

  // delete outdated

  NeighborCList clist;
  for (auto it : m_neighbors)
    {
      clist.push_back (std::make_tuple (std::get<0> (it.second), it.first, std::get<1> (it.second)));
    }

  NeighborListComparator cmp =
    [](std::tuple<uint32_t, Ipv4Address, uint32_t> a, std::tuple<uint32_t, Ipv4Address, uint32_t> b)
      {
        return std::get<0> (a) <= std::get<0> (b);
      };

  clist.sort (cmp);

  double potential = 0;
  for (auto it : clist)
    {
      if (potential > (double) std::get<0> (it))
        {
          break;
        }
      potential += ((double) std::get<0> (it) - potential) * m_conductivity;
    }
  m_potential = (uint32_t) potential;

  AddDefaultRouteTo (std::get<1> (clist.back ()), std::get<2> (clist.back ()));
}

void
Potential::DoSendRouteUpdate (bool periodic)
{
  NS_LOG_FUNCTION (this << (periodic ? " periodic" : " triggered"));

  Ptr<Packet> p = Create<Packet> ();
  SocketIpTtlTag tag;
  p->RemovePacketTag (tag);
  tag.SetTtl (1);
  p->AddPacketTag (tag);

  PotentialHeader hdr;
  hdr.SetCommand (PotentialHeader::RESPONSE);
  hdr.SetPotential (m_potential);
  p->AddHeader (hdr);
  NS_LOG_DEBUG ("Potential: " << m_potential << ", SendTo: " << *p);

  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
    {
      uint32_t interface = iter->second;

      if (m_interfaceExclusions.find (interface) == m_interfaceExclusions.end ())
        {
          iter->first->SendTo (p, 0, InetSocketAddress (POTENTIAL_ALL_NODE, POTENTIAL_PORT));
        }
    }

  for (RoutesI rtIter = m_routes.begin (); rtIter != m_routes.end (); rtIter++)
    {
      rtIter->first->SetRouteChanged (false);
    }
}

void
Potential::SendTriggeredRouteUpdate ()
{
  NS_LOG_FUNCTION (this);

  if (m_nextTriggeredUpdate.IsRunning())
    {
      NS_LOG_LOGIC ("Skipping Triggered Update due to cooldown");
      return;
    }

  Time delay = Seconds (m_rng->GetValue (m_minTriggeredUpdateDelay.GetSeconds (), m_maxTriggeredUpdateDelay.GetSeconds ()));
  m_nextTriggeredUpdate = Simulator::Schedule (delay, &Potential::DoSendRouteUpdate, this, false);
}

void
Potential::SendUnsolicitedRouteUpdate ()
{
  NS_LOG_FUNCTION (this);

  if (m_nextTriggeredUpdate.IsRunning())
    {
      m_nextTriggeredUpdate.Cancel ();
    }

  DoSendRouteUpdate (true);

  Time delay = m_unsolicitedUpdate + Seconds (m_rng->GetValue (0, 0.5*m_unsolicitedUpdate.GetSeconds ()) );
  m_nextUnsolicitedUpdate = Simulator::Schedule (delay, &Potential::SendUnsolicitedRouteUpdate, this);
}

std::set<uint32_t>
Potential::GetInterfaceExclusions () const
{
  return m_interfaceExclusions;
}

void
Potential::SetInterfaceExclusions (std::set<uint32_t> exceptions)
{
  NS_LOG_FUNCTION (this);

  m_interfaceExclusions = exceptions;
}

uint8_t
Potential::GetInterfaceMetric (uint32_t interface) const
{
  NS_LOG_FUNCTION (this << interface);

  std::map<uint32_t, uint8_t>::const_iterator iter = m_interfaceMetrics.find (interface);
  if (iter != m_interfaceMetrics.end ())
    {
      return iter->second;
    }
  return 1;
}

void
Potential::SetInterfaceMetric (uint32_t interface, uint8_t metric)
{
  NS_LOG_FUNCTION (this << interface << int (metric));

  if (metric < m_linkDown)
    {
      m_interfaceMetrics[interface] = metric;
    }
}

void
Potential::SendRouteRequest ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> p = Create<Packet> ();
  SocketIpTtlTag tag;
  p->RemovePacketTag (tag);
  tag.SetTtl (1);
  p->AddPacketTag (tag);

  PotentialHeader hdr;
  hdr.SetCommand (PotentialHeader::REQUEST);
  hdr.SetPotential (m_potential);
  p->AddHeader (hdr);

  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
    {
      uint32_t interface = iter->second;

      if (m_interfaceExclusions.find (interface) == m_interfaceExclusions.end ())
        {
          NS_LOG_DEBUG ("SendTo: " << *p);
          iter->first->SendTo (p, 0, InetSocketAddress (POTENTIAL_ALL_NODE, POTENTIAL_PORT));
        }
    }
}

void
Potential::AddDefaultRouteTo (Ipv4Address nextHop, uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);

  AddNetworkRouteTo (Ipv4Address ("0.0.0.0"), Ipv4Mask::GetZero (), nextHop, interface);
}


/*
 * PotentialRoutingTableEntry
 */

PotentialRoutingTableEntry::PotentialRoutingTableEntry ()
  : m_tag (0),
    m_metric (0),
    m_status (POTENTIAL_INVALID),
    m_changed (false)
{
}

PotentialRoutingTableEntry::PotentialRoutingTableEntry (Ipv4Address network, Ipv4Mask networkPrefix, Ipv4Address nextHop, uint32_t interface)
  : Ipv4RoutingTableEntry ( Ipv4RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, nextHop, interface) ),
    m_tag (0),
    m_metric (0),
    m_status (POTENTIAL_INVALID),
    m_changed (false)
{
}

PotentialRoutingTableEntry::PotentialRoutingTableEntry (Ipv4Address network, Ipv4Mask networkPrefix, uint32_t interface)
  : Ipv4RoutingTableEntry ( Ipv4RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface) ),
    m_tag (0),
    m_metric (0),
    m_status (POTENTIAL_INVALID),
    m_changed (false)
{
}

PotentialRoutingTableEntry::~PotentialRoutingTableEntry ()
{
}


void
PotentialRoutingTableEntry::SetRouteTag (uint16_t routeTag)
{
  if (m_tag != routeTag)
    {
      m_tag = routeTag;
      m_changed = true;
    }
}

uint16_t
PotentialRoutingTableEntry::GetRouteTag () const
{
  return m_tag;
}

void
PotentialRoutingTableEntry::SetRouteMetric (uint8_t routeMetric)
{
  if (m_metric != routeMetric)
    {
      m_metric = routeMetric;
      m_changed = true;
    }
}

uint8_t
PotentialRoutingTableEntry::GetRouteMetric () const
{
  return m_metric;
}

void
PotentialRoutingTableEntry::SetRouteStatus (Status_e status)
{
  if (m_status != status)
    {
      m_status = status;
      m_changed = true;
    }
}

PotentialRoutingTableEntry::Status_e
PotentialRoutingTableEntry::GetRouteStatus (void) const
{
  return m_status;
}

void
PotentialRoutingTableEntry::SetRouteChanged (bool changed)
{
  m_changed = changed;
}

bool
PotentialRoutingTableEntry::IsRouteChanged (void) const
{
  return m_changed;
}


std::ostream & operator << (std::ostream& os, const PotentialRoutingTableEntry& rte)
{
  os << static_cast<const Ipv4RoutingTableEntry &>(rte);
  os << ", metric: " << int (rte.GetRouteMetric ()) << ", tag: " << int (rte.GetRouteTag ());

  return os;
}


}

