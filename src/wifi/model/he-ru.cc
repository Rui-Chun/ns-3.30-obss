/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018
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
 * Author: Stefano Avallone <stavallo@unina.it>
 */

#include "he-ru.h"
#include "ns3/abort.h"

namespace ns3 {

const std::map<HeRu::BwTonesPair,std::vector<HeRu::Indices>> HeRu::m_heRuSubcarrierRanges =
{
  // RUs in a 20 MHz HE PPDU (Table 28-6)
  { {20, HeRu::RU_26_TONE}, { /* 1 */ {{-121,-96}},
                              /* 2 */ {{-95,-70}},
                              /* 3 */ {{-68,-43}},
                              /* 4 */ {{-42,-17}},
                              /* 5 */ {{-16,-4}, {4,16}},
                              /* 6 */ {{17,42}},
                              /* 7 */ {{43,68}},
                              /* 8 */ {{70,95}},
                              /* 9 */ {{96,121}} } },
  { {20, HeRu::RU_52_TONE}, { /* 1 */ {{-121,-70}},
                              /* 2 */ {{-68,-17}},
                              /* 3 */ {{17,68}},
                              /* 4 */ {{70,121}} } },
  { {20, HeRu::RU_106_TONE}, { /* 1 */ {{-122,-17}},
                               /* 2 */ {{17,122}} } },
  { {20, HeRu::RU_242_TONE}, { /* 1 */ {{-122,-2}, {2,122}} } },
  // RUs in a 40 MHz HE PPDU (Table 28-7)
  { {40, HeRu::RU_26_TONE}, { /* 1 */ {{-243,-218}},
                              /* 2 */ {{-217,-192}},
                              /* 3 */ {{-189,-164}},
                              /* 4 */ {{-163,-138}},
                              /* 5 */ {{-136,-111}},
                              /* 6 */ {{-109,-84}},
                              /* 7 */ {{-83,-58}},
                              /* 8 */ {{-55,-30}},
                              /* 9 */ {{-29,-4}},
                              /* 10 */ {{4,29}},
                              /* 11 */ {{30,55}},
                              /* 12 */ {{58,83}},
                              /* 13 */ {{84,109}},
                              /* 14 */ {{111,136}},
                              /* 15 */ {{138,163}},
                              /* 16 */ {{164,189}},
                              /* 17 */ {{192,217}},
                              /* 18 */ {{218,243}} } },
  { {40, HeRu::RU_52_TONE}, { /* 1 */ {{-243,-192}},
                              /* 2 */ {{-189,-138}},
                              /* 3 */ {{-109,-58}},
                              /* 4 */ {{-55,-4}},
                              /* 5 */ {{4,55}},
                              /* 6 */ {{58,109}},
                              /* 7 */ {{138,189}},
                              /* 8 */ {{192,243}} } },
  { {40, HeRu::RU_106_TONE}, { /* 1 */ {{-243,-138}},
                               /* 2 */ {{-109,-4}},
                               /* 3 */ {{4,109}},
                               /* 4 */ {{138,243}} } },
  { {40, HeRu::RU_242_TONE}, { /* 1 */ {{-244,-3}},
                               /* 2 */ {{3,244}} } },
  { {40, HeRu::RU_484_TONE}, { /* 1 */ {{-244,-3}, {3,244}} } },
  // RUs in an 80 MHz HE PPDU (Table 28-8)
  { {80, HeRu::RU_26_TONE}, { /* 1 */ {{-499,-474}},
                              /* 2 */ {{-473,-448}},
                              /* 3 */ {{-445,-420}},
                              /* 4 */ {{-419,-394}},
                              /* 5 */ {{-392,-367}},
                              /* 6 */ {{-365,-340}},
                              /* 7 */ {{-339,-314}},
                              /* 8 */ {{-311,-286}},
                              /* 9 */ {{-285,-260}},
                              /* 10 */ {{-257,-232}},
                              /* 11 */ {{-231,-206}},
                              /* 12 */ {{-203,-178}},
                              /* 13 */ {{-177,-152}},
                              /* 14 */ {{-150,-125}},
                              /* 15 */ {{-123,-98}},
                              /* 16 */ {{-97,-72}},
                              /* 17 */ {{-69,-44}},
                              /* 18 */ {{-43,-18}},
                              /* 19 */ {{-16,-4}, {4,16}},
                              /* 20 */ {{18,43}},
                              /* 21 */ {{44,69}},
                              /* 22 */ {{72,97}},
                              /* 23 */ {{98,123}},
                              /* 24 */ {{125,150}},
                              /* 25 */ {{152,177}},
                              /* 26 */ {{178,203}},
                              /* 27 */ {{206,231}},
                              /* 28 */ {{232,257}},
                              /* 29 */ {{260,285}},
                              /* 30 */ {{286,311}},
                              /* 31 */ {{314,339}},
                              /* 32 */ {{340,365}},
                              /* 33 */ {{367,392}},
                              /* 34 */ {{394,419}},
                              /* 35 */ {{420,445}},
                              /* 36 */ {{448,473}},
                              /* 37 */ {{474,499}} } },
  { {80, HeRu::RU_52_TONE}, { /* 1 */ {{-499,-448}},
                              /* 2 */ {{-445,-394}},
                              /* 3 */ {{-365,-314}},
                              /* 4 */ {{-311,-260}},
                              /* 5 */ {{-257,-206}},
                              /* 6 */ {{-203,-152}},
                              /* 7 */ {{-123,-72}},
                              /* 8 */ {{-69,-18}},
                              /* 9 */ {{18,69}},
                              /* 10 */ {{72,123}},
                              /* 11 */ {{152,203}},
                              /* 12 */ {{206,257}},
                              /* 13 */ {{260,311}},
                              /* 14 */ {{314,365}},
                              /* 15 */ {{394,445}},
                              /* 16 */ {{448,499}} } },
  { {80, HeRu::RU_106_TONE}, { /* 1 */ {{-499,-394}},
                               /* 2 */ {{-365,-260}},
                               /* 3 */ {{-257,-152}},
                               /* 4 */ {{-123,-18}},
                               /* 5 */ {{18,123}},
                               /* 6 */ {{152,257}},
                               /* 7 */ {{260,365}},
                               /* 8 */ {{394,499}} } },
  { {80, HeRu::RU_242_TONE}, { /* 1 */ {{-500,-259}},
                               /* 2 */ {{-258,-17}},
                               /* 3 */ {{17,258}},
                               /* 4 */ {{259,500}} } },
  { {80, HeRu::RU_484_TONE}, { /* 1 */ {{-500,-17}},
                               /* 2 */ {{17,500}} } },
  { {80, HeRu::RU_996_TONE}, { /* 1 */ {{-500,-3}, {3,500}} } }
};


std::size_t
HeRu::GetNRus (uint8_t bw, RuType ruType)
{
  if (bw == 160 && ruType == RU_2x996_TONE)
    {
      return 1;
    }

  // if the bandwidth is 160MHz, search for the number of RUs available
  // in 80MHz and double the result.
  auto it = m_heRuSubcarrierRanges.find ({(bw == 160 ? 80 : bw), ruType});

  if (it == m_heRuSubcarrierRanges.end ())
    {
      return 0;
    }

  return (bw == 160 ? 2 : 1) * it->second.size ();
}

HeRu::Indices
HeRu::GetSubcarrierRange (uint8_t bw, RuType ruType, std::size_t index)
{
  auto it = m_heRuSubcarrierRanges.find ({bw, ruType});

  NS_ABORT_MSG_IF (it == m_heRuSubcarrierRanges.end (), "RU not found");
  NS_ABORT_MSG_IF (!index || index > it->second.size (), "RU index not available");

  return it->second.at (index-1);
}

bool
HeRu::Overlap (uint8_t bw, RuSpec ru, const std::vector<RuSpec> &v)
{
  // A 2x996-tone RU spans 160 MHz, hence it overlaps with any other RU
  if (bw == 160 && ru.ruType == RU_2x996_TONE && !v.empty ())
    {
      return true;
    }

  Indices ranges1 = GetSubcarrierRange ((bw == 160 ? 80 : bw), ru.ruType, ru.index);

  for (auto& r1 : ranges1)
    {
      for (auto& p : v)
        {
          if (bw == 160 && p.ruType == RU_2x996_TONE)
            {
              return true;
            }

          if (ru.primary80MHz != p.primary80MHz)
            {
              // the two RUs are located in distinct 80MHz bands
              continue;
            }

          Indices ranges2 = GetSubcarrierRange ((bw == 160 ? 80 : bw), p.ruType, p.index);
          for (auto& r2 : ranges2)
            {
              if (r1.second >= r2.first && r2.second >= r1.first)
                {
                  return true;
                }
            }
        }
    }
  return false;
}

bool
HeRu::Overlap (uint8_t bw, RuSpec ru, RuSpec v)
{
  std::vector<RuSpec> preRu;
  preRu.push_back (v);
  return Overlap (bw, ru, preRu);
}

bool
HeRu::IsValid (uint8_t bw, RuSpec ru)
{
  if (ru.index < 1)
    {
      return false;
    }

  if (bw == 160 && ru.ruType == RU_2x996_TONE)
    {
      return ru.primary80MHz;
    }
  else
    {
      return (ru.index <= GetNRus (bw == 160 ? 80 : bw, ru.ruType));
    }
}

bool
HeRu::IsSame (RuSpec ru1, RuSpec ru2)
{
  if ((ru1.index == 0) && (ru2.index == 0))
    {
      return true;
    }
    
  return (ru1.primary80MHz == ru2.primary80MHz) && (ru1.ruType == ru2.ruType) && (ru1.index == ru2.index);
}

} //namespace ns3
