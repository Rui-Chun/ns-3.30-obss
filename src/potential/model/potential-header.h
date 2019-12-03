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

#ifndef POTENTIAL_HEADER_H
#define POTENTIAL_HEADER_H

#include <list>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"


namespace ns3 {

/**
 * \ingroup potential
 * \brief PotentialHeader
 */
class PotentialHeader : public Header
{
public:
  PotentialHeader (void);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Return the instance type identifier.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId (void) const;

  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size of the packet.
   * \return size
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * \brief Serialize the packet.
   * \param start Buffer iterator
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start Buffer iterator
   * \return size of the packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Commands to be used in Potential headers
   */
  enum Command_e
  {
    REQUEST = 0x1,
    RESPONSE = 0x2,
  };

  /**
   * \brief Set the command
   * \param command the command
   */
  void SetCommand (Command_e command);

  /**
   * \brief Get the command
   * \returns the command
   */
  Command_e GetCommand (void) const;

  void SetPotential (uint32_t potential);
  uint32_t GetPotential (void);

private:
  uint8_t m_command; //!< command type
  uint32_t m_potential;
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the reference to the output stream
 * \param h the Potential header
 * \returns the reference to the output stream
 */
std::ostream & operator << (std::ostream & os, const PotentialHeader & h);

}

#endif /* Potential_HEADER_H */

