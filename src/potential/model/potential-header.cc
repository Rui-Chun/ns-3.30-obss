/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c)
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
 * Author:
 */

#include "potential-header.h"
#include "ns3/log.h"

namespace ns3 {

/*
 * PotentialHeader
 */
NS_LOG_COMPONENT_DEFINE ("PotentialHeader");
NS_OBJECT_ENSURE_REGISTERED (PotentialHeader);


PotentialHeader::PotentialHeader ()
  : m_command (0)
{
}

TypeId
PotentialHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PotentialHeader")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<PotentialHeader> ();
  return tid;
}

TypeId
PotentialHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
PotentialHeader::Print (std::ostream & os) const
{
  os << "command " << int(m_command);
  os << " | potential " << m_potential;
}

uint32_t
PotentialHeader::GetSerializedSize () const
{
  return 4 + sizeof (m_potential);
}

void
PotentialHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (uint8_t (m_command));
  i.WriteU8 (2);
  i.WriteU16 (0);

  i.WriteU32 (m_potential);
}

uint32_t
PotentialHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  uint8_t temp;
  temp = i.ReadU8 ();
  if ((temp == REQUEST) || (temp == RESPONSE))
    {
      m_command = temp;
    }
  else
    {
      return 0;
    }

  if (i.ReadU8 () != 2)
    {
      NS_LOG_LOGIC ("POTENTIAL received a message with mismatch version, ignoring.");
      return 0;
    }

  if (i.ReadU16 () != 0)
    {
      NS_LOG_LOGIC ("POTENTIAL received a message with invalid filled flags, ignoring.");
      return 0;
    }

  m_potential = i.ReadU32 ();

  return GetSerializedSize ();
}

void
PotentialHeader::SetCommand (PotentialHeader::Command_e command)
{
  m_command = command;
}

PotentialHeader::Command_e
PotentialHeader::GetCommand () const
{
  return PotentialHeader::Command_e (m_command);
}

void
PotentialHeader::SetPotential (uint32_t potential)
{
  m_potential = uint16_t (potential);
}

uint32_t
PotentialHeader::GetPotential (void)
{
  return uint16_t (m_potential);
}


std::ostream & operator << (std::ostream & os, const PotentialHeader & h)
{
  h.Print (os);
  return os;
}


}

