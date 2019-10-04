/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/log.h"
#include "obss-wifi-manager.h"
#include "wifi-phy.h"
#include "ns3/string.h"
// #include <fstream>

#define HEMCS0 18 // only consider HE Mcs
#define TRANLIMIT 500 // the initial transmission will be sent at lowest rate
#define SNRMARGIN 0
#define ACKSNRMARGIN 0
#define DEFAULTLOSS -120

namespace ns3 {
/**
 * \brief hold per-remote-station state for Ideal Wifi manager.
 *
 * This struct extends from WifiRemoteStation struct to hold additional
 * information required by the Ideal Wifi manager
 */
struct ObssWifiRemoteStation : public WifiRemoteStation
{
  double m_lastSnrObserved;  //!< SNR of most recently reported packet sent to the remote station
  double m_lastSnrCached;    //!< SNR most recently used to select a rate
  uint8_t m_nss;             //!< Number of spatial streams
  WifiMode m_lastMode;       //!< Mode most recently used to the remote station
};

/// To avoid using the cache before a valid value has been cached
static const double CACHE_INITIAL_VALUE = -100;

static ObssWifiManager::PathLossPairs globalLossPairs; // (dst+src, loss)

static ObssWifiManager::TransRecords globalTransRecords;

static ObssWifiManager::RxRecords globalRxRecords;

static uint64_t TransNum =0;

static uint64_t RxNum = 0;

static uint64_t ResetNum = 0;

NS_OBJECT_ENSURE_REGISTERED (ObssWifiManager);

NS_LOG_COMPONENT_DEFINE ("ObssWifiManager");

TypeId
ObssWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ObssWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<ObssWifiManager> ()
    .AddAttribute ("BerThreshold",
                   "The maximum Bit Error Rate acceptable at any transmission mode",
                   DoubleValue (1e-5),
                   MakeDoubleAccessor (&ObssWifiManager::m_ber),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ConstantMode", "The constant rate Mode when not restricted by obss",
                   StringValue ("HeMcs6"),
                   MakeWifiModeAccessor (&ObssWifiManager::m_constMode),
                   MakeWifiModeChecker ())
    .AddAttribute ("RecordPath", "The file path of records",
                   StringValue ("TransRecords.txt"),
                   MakeStringAccessor( &ObssWifiManager::m_recordPath),
                   MakeStringChecker())
    .AddTraceSource ("Rate",
                     "Traced value for rate changes (b/s)",
                     MakeTraceSourceAccessor (&ObssWifiManager::m_currentRate),
                     "ns3::TracedValueCallback::Uint64")
  ;
  return tid;
}

ObssWifiManager::ObssWifiManager ()
  : m_currentRate (0)
{
  NS_LOG_FUNCTION (this);
}

ObssWifiManager::~ObssWifiManager ()
{
  NS_LOG_FUNCTION (this);
}

void
ObssWifiManager::SetupPhy (const Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  WifiRemoteStationManager::SetupPhy (phy);
}

uint16_t
ObssWifiManager::GetChannelWidthForMode (WifiMode mode) const
{
  NS_ASSERT (mode.GetModulationClass () != WIFI_MOD_CLASS_HT
             && mode.GetModulationClass () != WIFI_MOD_CLASS_VHT
             && mode.GetModulationClass () != WIFI_MOD_CLASS_HE);
  if (mode.GetModulationClass () == WIFI_MOD_CLASS_DSSS
      || mode.GetModulationClass () == WIFI_MOD_CLASS_HR_DSSS)
    {
      return 22;
    }
  else
    {
      return 20;
    }
}

void
ObssWifiManager::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  
  //######
  // connect end of preamble trace source
  GetPhy()->TraceConnectWithoutContext ("EndOfHePreamble", MakeCallback (&ObssWifiManager::ReceiveHeSig, this));
  m_device = DynamicCast<WifiNetDevice> (GetPhy()->GetDevice());
  m_obssRestricted = false;
  uint8_t mymac[6];
  m_device->GetMac()->GetAddress().CopyTo(mymac);
  m_myMac = mymac[5];

}

double
ObssWifiManager::GetSnrThreshold (WifiTxVector txVector) const
{
  NS_LOG_FUNCTION (this << txVector.GetMode ().GetUniqueName ());
  for (Thresholds::const_iterator i = m_thresholds.begin (); i != m_thresholds.end (); i++)
    {
      NS_LOG_DEBUG ("Checking " << i->second.GetMode ().GetUniqueName () <<
                    " nss " << +i->second.GetNss () <<
                    " GI " << i->second.GetGuardInterval () <<
                    " width " << i->second.GetChannelWidth ());
      NS_LOG_DEBUG ("against TxVector " << txVector.GetMode ().GetUniqueName () <<
                    " nss " << +txVector.GetNss () <<
                    " GI " << txVector.GetGuardInterval () <<
                    " width " << txVector.GetChannelWidth ());
      if (txVector.GetMode () == i->second.GetMode ()
          && txVector.GetNss () == i->second.GetNss ()
          && txVector.GetChannelWidth () == i->second.GetChannelWidth ())
        {
          return i->first;
        }
    }
  NS_ASSERT (false);
  return 0.0;
}

void
ObssWifiManager::AddSnrThreshold (WifiTxVector txVector, double snr)
{
  NS_LOG_FUNCTION (this << txVector.GetMode ().GetUniqueName () << snr);
  m_thresholds.push_back (std::make_pair (snr, txVector));
}

WifiRemoteStation *
ObssWifiManager::DoCreateStation (void) const
{
  NS_LOG_FUNCTION (this);
  ObssWifiRemoteStation *station = new ObssWifiRemoteStation ();
  station->m_lastSnrObserved = 0.0;
  station->m_lastSnrCached = CACHE_INITIAL_VALUE;
  station->m_lastMode = GetDefaultMode ();
  station->m_nss = 1;
  return station;
}


void
ObssWifiManager::DoReportRxOk (WifiRemoteStation *station, double rxSnr, WifiMode txMode)
{
  NS_LOG_DEBUG("RX Ok!");
  RxNum++;
  NS_LOG_DEBUG("RxNum= "<<+RxNum);
  uint8_t staAddrs[6];
  station->m_state->m_address.CopyTo(staAddrs);
  if(RxNum % 1000 ==0 )
  {
    std::ofstream myfile;
    myfile.open ("RxRecords.txt");
    myfile<<"Dst\tSrc\tHeMcs\tRxSnr\tNum\n";
    for(uint16_t i=0; i<globalRxRecords.size(); i++)
    {
      uint8_t temp_dst = std::get<0>(globalRxRecords[i]);
      uint8_t temp_src = std::get<1>(globalRxRecords[i]);
      int temp_mcs = std::get<2>(globalRxRecords[i]);
      double temp_snr = std::get<3>(globalRxRecords[i]);
      uint64_t temp_num = std::get<4>(globalRxRecords[i]);
      myfile << (int)temp_dst<<" \t  "<<(int)temp_src<<"\t  "<<temp_mcs<<"\t  "<<(double)temp_snr<<"\t  "<<(long)temp_num<<"\n";
    }

    myfile<<"\n TotalRx Number= "<<+RxNum<<"\n";
    myfile.close();
  }

  bool rxFlag=false;

  for(uint16_t i=0;i<globalRxRecords.size(); i++)
  {
    uint8_t temp_dst = std::get<0>(globalRxRecords[i]);
    uint8_t temp_src = std::get<1>(globalRxRecords[i]);
    int temp_mcs = std::get<2>(globalRxRecords[i]);
    double temp_snr = std::get<3>(globalRxRecords[i]);
    if(temp_src==staAddrs[5] && temp_dst==m_myMac && temp_mcs==txMode.GetMcsValue() && std::abs(temp_snr-rxSnr) < 1e-3)
    {
      std::get<4>(globalRxRecords[i])++; // num ++
      rxFlag=true;
      break;
    }
  }
  if(!rxFlag)
  {
    RxRecord rx(m_myMac, staAddrs[5], txMode.GetMcsValue(), rxSnr, 1);
    globalRxRecords.push_back(rx);
    // std::cout<<"TransRecord pushed"<<std::endl;
  }
  NS_LOG_FUNCTION (this << station << rxSnr << txMode);


  
}

void
ObssWifiManager::DoReportRtsFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

void
ObssWifiManager::DoReportDataFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

void
ObssWifiManager::DoReportRtsOk (WifiRemoteStation *st,
                                 double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_FUNCTION (this << st << ctsSnr << ctsMode.GetUniqueName () << rtsSnr);
  ObssWifiRemoteStation *station = (ObssWifiRemoteStation *)st;
  station->m_lastSnrObserved = rtsSnr;
}

void
ObssWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                  double ackSnr, WifiMode ackMode, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << ackSnr << ackMode.GetUniqueName () << dataSnr);
  NS_LOG_DEBUG("Data Ok, ackMode: "<< ackMode);

  uint8_t addrs[6];
  st->m_state->m_address.CopyTo(addrs);

  // std::cout<<"myMac: "<< +m_myMac <<" staMac: "<< +addrs[5]<< " dataSnr:  "<< WToDbm(dataSnr) << std::endl;
  NS_LOG_DEBUG( "myMac: "<< +m_myMac <<" staMac: "<< +addrs[5]<< " dataSnr:  "<< WToDbm(dataSnr) );
  ObssWifiRemoteStation *station = (ObssWifiRemoteStation *)st;
  if (dataSnr == 0)
    {
      NS_LOG_WARN ("DataSnr reported to be zero; not saving this report.");
      return;
    }
  station->m_lastSnrObserved = dataSnr;
}

void
ObssWifiManager::DoReportAmpduTxStatus (WifiRemoteStation *st, uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus, double rxSnr, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << +nSuccessfulMpdus << +nFailedMpdus << rxSnr << dataSnr);
  ObssWifiRemoteStation *station = (ObssWifiRemoteStation *)st;
  if (dataSnr == 0)
    {
      NS_LOG_WARN ("DataSnr reported to be zero; not saving this report.");
      return;
    }
  station->m_lastSnrObserved = dataSnr;
}


void
ObssWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

void
ObssWifiManager::DoReportFinalDataFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

WifiTxVector
ObssWifiManager::DoGetDataTxVector (WifiRemoteStation *st)
{
  NS_LOG_FUNCTION (this << st);

  // std::cout<<"Obss GetDataVector !"<<std::endl;
  NS_LOG_DEBUG("\nObss GetDataVector !");
  CheckObssStatus();

  ObssWifiRemoteStation *station = (ObssWifiRemoteStation *)st;
  // we need have station mac address
  uint8_t staAddrs[6];
  station->m_state->m_address.CopyTo(staAddrs);

  //We search within the Supported rate set the mode with the
  //highest data rate for which the snr threshold is smaller than m_lastSnr
  //to ensure correct packet delivery.
  WifiMode maxMode = GetDefaultMode ();
  WifiTxVector txVector;
  WifiMode mode;
  uint64_t bestRate = 0;
  uint8_t selectedNss = 1;
  uint16_t guardInterval;
  uint16_t channelWidth = std::min (GetChannelWidth (station), GetPhy ()->GetChannelWidth ());
  txVector.SetChannelWidth (channelWidth);

  if ((GetHeSupported ())
      && (GetHeSupported (st)))
    {
      // only consider HE mcs ########
      if(m_obssRestricted && TransNum > TRANLIMIT && staAddrs[5]==m_nexthopMac)
      {
        // std::cout<<"Using Obss rate!"<<std::endl;
        NS_LOG_DEBUG("Using Obss rate!");

        mode = GetMcsSupported (station, m_obssMcsLimit+HEMCS0);
        txVector.SetMode (mode);
        guardInterval = std::max (GetGuardInterval (station), GetGuardInterval ());
        txVector.SetGuardInterval (guardInterval);
        // If the node and peer are not both HE capable, only search (V)HT modes
        for (uint8_t nss = 1; nss <= std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (station)); nss++)
          {
            txVector.SetNss (nss);
            if (!txVector.IsValid ())
              {
                NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
                              " nss " << +nss <<
                              " width " << +txVector.GetChannelWidth ());
                continue;
              }
            uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
            if (dataRate > bestRate)
              {
                bestRate = dataRate;
                maxMode = mode;
                selectedNss = nss;
              }
          }
          // std::cout<< "Obss! Testing mode = " << maxMode.GetUniqueName () <<
          //       " data rate " << bestRate << std::endl;
          NS_LOG_DEBUG("Obss! Using mode = " << maxMode.GetUniqueName () << " data rate " << bestRate );

      }
      else
      {
        double f_SNRestimated;
        double f_threshold;
          mode = m_constMode;
          txVector.SetMode (mode);
          guardInterval = std::max (GetGuardInterval (station), GetGuardInterval ());
          txVector.SetGuardInterval (guardInterval);
          // If the node and peer are not both HE capable, only search (V)HT modes
          for (uint8_t nss = 1; nss <= std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (station)); nss++)
            {
              txVector.SetNss (nss);
              if (!txVector.IsValid ())
                {
                  NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
                                " nss " << +nss <<
                                " width " << +txVector.GetChannelWidth ());
                  continue;
                }
              // double threshold = GetSnrThreshold (txVector);
              double threshold = WToDbm(GetPhy()->CalculateSnr(txVector, m_ber)) - SNRMARGIN; // change to dbm and compare
              double temp_power = GetPhy()->GetPowerDbm(GetDefaultTxPowerLevel());
              double temp_signal = DbmToW(temp_power + GetPathLoss(staAddrs[5], m_myMac));
              double SNRestimated = WToDbm(CalculateSnr(temp_signal, 0, channelWidth));
              double dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);

              // std::cout<< "Testing mode = " << mode.GetUniqueName () <<
              //               " data rate " << dataRate <<
              //               " threshold " << threshold  << " SNR estimated " <<
              //               SNRestimated<< std::endl;
              if (dataRate > bestRate )
                {
                  bestRate = dataRate;
                  maxMode = mode;
                  selectedNss = nss;
                  f_SNRestimated = SNRestimated;
                  f_threshold = threshold;
                }
            }
          // std::cout<<"Tx mode = " << maxMode.GetUniqueName () <<
          // " data rate " << bestRate 
          // << " threshold " << f_threshold  << " SNR estimated " <<
          // f_SNRestimated<<std::endl;
          NS_LOG_DEBUG("Tx mode = " << maxMode.GetUniqueName () <<
          " data rate " << bestRate 
          << " threshold " << f_threshold  << " SNR estimated " << f_SNRestimated);
      }

    }
  else
    {
      // Non-HT selection
      selectedNss = 1;
      for (uint8_t i = 0; i < 1; i++)
        {
          mode = GetSupported (station, i);
          txVector.SetMode (mode);
          txVector.SetNss (selectedNss);
          txVector.SetChannelWidth (GetChannelWidthForMode (mode));;
          uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), txVector.GetNss ());
          NS_LOG_DEBUG ("mode = " << mode.GetUniqueName () );

          if (dataRate > bestRate)
            {
              NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                            " data rate " << dataRate);
              bestRate = dataRate;
              maxMode = mode;
            }
        }
    }

  NS_LOG_DEBUG ("Found maxMode: " << maxMode << " channelWidth: " << channelWidth);
  if (maxMode.GetModulationClass () == WIFI_MOD_CLASS_HE)
    {
      guardInterval = std::max (GetGuardInterval (station), GetGuardInterval ());
    }
  else if ((maxMode.GetModulationClass () == WIFI_MOD_CLASS_HT) || (maxMode.GetModulationClass () == WIFI_MOD_CLASS_VHT))
    {
      guardInterval = static_cast<uint16_t> (std::max (GetShortGuardIntervalSupported (station) ? 400 : 800, GetShortGuardIntervalSupported () ? 400 : 800));
    }
  else
    {
      guardInterval = 800;
    }
  if (m_currentRate != maxMode.GetDataRate (channelWidth, guardInterval, selectedNss))
    {
      NS_LOG_DEBUG ("New datarate: " << maxMode.GetDataRate (channelWidth, guardInterval, selectedNss));
      m_currentRate = maxMode.GetDataRate (channelWidth, guardInterval, selectedNss);
    }

  uint8_t MyTxpower;
  if(m_obssRestricted)
  {
    MyTxpower = m_obssPowerLimit;
  }
  else
  {
    MyTxpower = GetDefaultTxPowerLevel ();
  }
  TransNum++;
  // std::cout<<"TransNum= "<<+TransNum<<std::endl;
  NS_LOG_DEBUG("TransNum= "<<+TransNum);
  if(TransNum % 1000 ==0 )
  {
    std::ofstream myfile;
    myfile.open (m_recordPath);
    // std::cout<<"TransRecord writing"<<std::endl;
    myfile<<"Dst\tSrc\tHeMcs\tTxlevel\tNum\tisObss\n";
    for(uint16_t i=0; i<globalTransRecords.size(); i++)
    {
      uint8_t temp_dst = std::get<0>(globalTransRecords[i]);
      uint8_t temp_src = std::get<1>(globalTransRecords[i]);
      int temp_mcs = std::get<2>(globalTransRecords[i]);
      uint8_t temp_power = std::get<3>(globalTransRecords[i]);
      uint64_t temp_num = std::get<4>(globalTransRecords[i]);
      int temp_isObss = std::get<5>(globalTransRecords[i]);
      myfile << (int)temp_dst<<" \t  "<<(int)temp_src<<"\t  "<<temp_mcs<<"\t  "<<(int)temp_power<<"\t  "<<(long)temp_num<<"\t  "<<temp_isObss<<"\n";
    }
    myfile<<"\n TotalTx Number= "<<+TransNum<<"\n";
    myfile<<"\n TotalReset Number= "<<+ResetNum<<"\n";
    myfile.close();
  }

  bool tranFlag=false;
  int obssFlag;

  if(m_obssRestricted)obssFlag=1;
  else obssFlag=0;

  for(uint16_t i=0;i<globalTransRecords.size(); i++)
  {
    uint8_t temp_dst = std::get<0>(globalTransRecords[i]);
    uint8_t temp_src = std::get<1>(globalTransRecords[i]);
    int temp_mcs = std::get<2>(globalTransRecords[i]);
    uint8_t temp_power = std::get<3>(globalTransRecords[i]);
    int temp_isObss = std::get<5>(globalTransRecords[i]);
    if(temp_dst==staAddrs[5] && temp_src==m_myMac && temp_mcs==maxMode.GetMcsValue() && temp_power==MyTxpower && obssFlag==temp_isObss)
    {
      std::get<4>(globalTransRecords[i])++; // num ++
      tranFlag=true;
      break;
    }
  }
  if(!tranFlag)
  {
    TransRecord tran(staAddrs[5], m_myMac, maxMode.GetMcsValue(), MyTxpower, 1, obssFlag);
    globalTransRecords.push_back(tran);
    // std::cout<<"TransRecord pushed"<<std::endl;
  }

  return WifiTxVector (maxMode, MyTxpower, GetPreambleForTransmission (maxMode.GetModulationClass (), GetShortPreambleEnabled (), UseGreenfieldForDestination (GetAddress (station))), guardInterval, GetNumberOfAntennas (), selectedNss, 0, GetChannelWidthForTransmission (maxMode, channelWidth), GetAggregation (station), false);
}

// WifiTxVector
// ObssWifiManager::DoGetDataTxVector (WifiRemoteStation *st)
// {
//   NS_LOG_FUNCTION (this << st);
//   ObssWifiRemoteStation *station = (ObssWifiRemoteStation *)st;
//   //We search within the Supported rate set the mode with the
//   //highest data rate for which the snr threshold is smaller than m_lastSnr
//   //to ensure correct packet delivery.
//   WifiMode maxMode = GetDefaultMode ();
//   WifiTxVector txVector;
//   WifiMode mode;
//   uint64_t bestRate = 0;
//   uint8_t selectedNss = 1;
//   uint16_t guardInterval;
//   uint16_t channelWidth = std::min (GetChannelWidth (station), GetPhy ()->GetChannelWidth ());
//   txVector.SetChannelWidth (channelWidth);
//   if (station->m_lastSnrCached != CACHE_INITIAL_VALUE && station->m_lastSnrObserved == station->m_lastSnrCached)
//     {
//       // SNR has not changed, so skip the search and use the last
//       // mode selected
//       maxMode = station->m_lastMode;
//       selectedNss = station->m_nss;
//       NS_LOG_DEBUG ("Using cached mode = " << maxMode.GetUniqueName () <<
//                     " last snr observed " << station->m_lastSnrObserved <<
//                     " cached " << station->m_lastSnrCached <<
//                     " nss " << +selectedNss);
//     }
//   else
//     {
//       if ((GetHtSupported () || GetVhtSupported () || GetHeSupported ())
//           && (GetHtSupported (st) || GetVhtSupported (st) || GetHeSupported (st)))
//         {
//           for (uint8_t i = 0; i < GetNMcsSupported (station); i++)
//             {
//               mode = GetMcsSupported (station, i);
//               txVector.SetMode (mode);
//               if (mode.GetModulationClass () == WIFI_MOD_CLASS_HT)
//                 {
//                   guardInterval = static_cast<uint16_t> (std::max (GetShortGuardIntervalSupported (station) ? 400 : 800, GetShortGuardIntervalSupported () ? 400 : 800));
//                   txVector.SetGuardInterval (guardInterval);
//                   // If the node and peer are both VHT capable, only search VHT modes
//                   if (GetVhtSupported () && GetVhtSupported (station))
//                     {
//                       continue;
//                     }
//                   // If the node and peer are both HE capable, only search HE modes
//                   if (GetHeSupported () && GetHeSupported (station))
//                     {
//                       continue;
//                     }
//                   // Derive NSS from the MCS index. There is a different mode for each possible NSS value.
//                   uint8_t nss = (mode.GetMcsValue () / 8) + 1;
//                   txVector.SetNss (nss);
//                   if (!txVector.IsValid ()
//                       || nss > std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (st)))
//                     {
//                       NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
//                                     " nss " << +nss <<
//                                     " width " << txVector.GetChannelWidth ());
//                       continue;
//                     }
//                   double threshold = GetSnrThreshold (txVector);
//                   uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
//                   NS_LOG_DEBUG ("Testing mode " << mode.GetUniqueName () <<
//                                 " data rate " << dataRate <<
//                                 " threshold " << threshold  << " last snr observed " <<
//                                 station->m_lastSnrObserved << " cached " <<
//                                 station->m_lastSnrCached);
//                   if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
//                     {
//                       NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
//                                     " data rate " << dataRate <<
//                                     " threshold " << threshold  <<
//                                     " last snr observed " <<
//                                     station->m_lastSnrObserved);
//                       bestRate = dataRate;
//                       maxMode = mode;
//                       selectedNss = nss;
//                     }
//                 }
//               else if (mode.GetModulationClass () == WIFI_MOD_CLASS_VHT)
//                 {
//                   guardInterval = static_cast<uint16_t> (std::max (GetShortGuardIntervalSupported (station) ? 400 : 800, GetShortGuardIntervalSupported () ? 400 : 800));
//                   txVector.SetGuardInterval (guardInterval);
//                   // If the node and peer are both HE capable, only search HE modes
//                   if (GetHeSupported () && GetHeSupported (station))
//                     {
//                       continue;
//                     }
//                   // If the node and peer are not both VHT capable, only search HT modes
//                   if (!GetVhtSupported () || !GetVhtSupported (station))
//                     {
//                       continue;
//                     }
//                   for (uint8_t nss = 1; nss <= std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (station)); nss++)
//                     {
//                       txVector.SetNss (nss);
//                       if (!txVector.IsValid ())
//                         {
//                           NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
//                                         " nss " << +nss <<
//                                         " width " << txVector.GetChannelWidth ());
//                           continue;
//                         }
//                       double threshold = GetSnrThreshold (txVector);
//                       uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
//                       NS_LOG_DEBUG ("Testing mode = " << mode.GetUniqueName () <<
//                                     " data rate " << dataRate <<
//                                     " threshold " << threshold << " last snr observed " <<
//                                     station->m_lastSnrObserved << " cached " <<
//                                     station->m_lastSnrCached);
//                       if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
//                         {
//                           NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
//                                         " data rate " << dataRate <<
//                                         " threshold " << threshold  <<
//                                         " last snr observed " <<
//                                         station->m_lastSnrObserved);
//                           bestRate = dataRate;
//                           maxMode = mode;
//                           selectedNss = nss;
//                         }
//                     }
//                 }
//               else //HE
//                 {
//                   guardInterval = std::max (GetGuardInterval (station), GetGuardInterval ());
//                   txVector.SetGuardInterval (guardInterval);
//                   // If the node and peer are not both HE capable, only search (V)HT modes
//                   if (!GetHeSupported () || !GetHeSupported (station))
//                     {
//                       continue;
//                     }
//                   for (uint8_t nss = 1; nss <= std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (station)); nss++)
//                     {
//                       txVector.SetNss (nss);
//                       if (!txVector.IsValid ())
//                         {
//                           NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
//                                         " nss " << +nss <<
//                                         " width " << +txVector.GetChannelWidth ());
//                           continue;
//                         }
//                       double threshold = GetSnrThreshold (txVector);
//                       uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
//                       NS_LOG_DEBUG ("Testing mode = " << mode.GetUniqueName () <<
//                                     " data rate " << dataRate <<
//                                     " threshold " << threshold  << " last snr observed " <<
//                                     station->m_lastSnrObserved << " cached " <<
//                                     station->m_lastSnrCached);
//                       if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
//                         {
//                           NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
//                                         " data rate " << dataRate <<
//                                         " threshold " << threshold  <<
//                                         " last snr observed " <<
//                                         station->m_lastSnrObserved);
//                           bestRate = dataRate;
//                           maxMode = mode;
//                           selectedNss = nss;
//                         }
//                     }
//                 }
//             }
//         }
//       else
//         {
//           // Non-HT selection
//           selectedNss = 1;
//           for (uint8_t i = 0; i < GetNSupported (station); i++)
//             {
//               mode = GetSupported (station, i);
//               txVector.SetMode (mode);
//               txVector.SetNss (selectedNss);
//               txVector.SetChannelWidth (GetChannelWidthForMode (mode));
//               double threshold = GetSnrThreshold (txVector);
//               uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), txVector.GetNss ());
//               NS_LOG_DEBUG ("mode = " << mode.GetUniqueName () <<
//                             " threshold " << threshold  <<
//                             " last snr observed " <<
//                             station->m_lastSnrObserved);
//               if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
//                 {
//                   NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
//                                 " data rate " << dataRate <<
//                                 " threshold " << threshold  <<
//                                 " last snr observed " <<
//                                 station->m_lastSnrObserved);
//                   bestRate = dataRate;
//                   maxMode = mode;
//                 }
//             }
//         }
//       NS_LOG_DEBUG ("Updating cached values for station to " <<  maxMode.GetUniqueName () << " snr " << station->m_lastSnrObserved);
//       station->m_lastSnrCached = station->m_lastSnrObserved;
//       station->m_lastMode = maxMode;
//       station->m_nss = selectedNss;
//     }
//   NS_LOG_DEBUG ("Found maxMode: " << maxMode << " channelWidth: " << channelWidth);
//   if (maxMode.GetModulationClass () == WIFI_MOD_CLASS_HE)
//     {
//       guardInterval = std::max (GetGuardInterval (station), GetGuardInterval ());
//     }
//   else if ((maxMode.GetModulationClass () == WIFI_MOD_CLASS_HT) || (maxMode.GetModulationClass () == WIFI_MOD_CLASS_VHT))
//     {
//       guardInterval = static_cast<uint16_t> (std::max (GetShortGuardIntervalSupported (station) ? 400 : 800, GetShortGuardIntervalSupported () ? 400 : 800));
//     }
//   else
//     {
//       guardInterval = 800;
//     }
//   if (m_currentRate != maxMode.GetDataRate (channelWidth, guardInterval, selectedNss))
//     {
//       NS_LOG_DEBUG ("New datarate: " << maxMode.GetDataRate (channelWidth, guardInterval, selectedNss));
//       m_currentRate = maxMode.GetDataRate (channelWidth, guardInterval, selectedNss);
//     }

//   return WifiTxVector (maxMode, GetDefaultTxPowerLevel (), GetPreambleForTransmission (maxMode.GetModulationClass (), GetShortPreambleEnabled (), UseGreenfieldForDestination (GetAddress (station))), guardInterval, GetNumberOfAntennas (), selectedNss, 0, GetChannelWidthForTransmission (maxMode, channelWidth), GetAggregation (station), false);
// }

WifiTxVector
ObssWifiManager::DoGetRtsTxVector (WifiRemoteStation *st)
{
  NS_LOG_FUNCTION (this << st);
  ObssWifiRemoteStation *station = (ObssWifiRemoteStation *)st;
  //We search within the Basic rate set the mode with the highest
  //snr threshold possible which is smaller than m_lastSnr to
  //ensure correct packet delivery.
  WifiTxVector txVector;
  WifiMode mode;
  uint8_t nss = 1;
  //avoid to use legacy rate adaptation algorithms for IEEE 802.11n/ac/ax
  //RTS is sent in a legacy frame; RTS with HT/VHT/HE is not yet supported
  mode = GetBasicMode (0);
  txVector.SetMode (mode);
  txVector.SetNss (nss);
  txVector.SetChannelWidth (GetChannelWidthForMode (mode));

  return WifiTxVector (mode, GetDefaultTxPowerLevel (), GetPreambleForTransmission (mode.GetModulationClass (), GetShortPreambleEnabled (), UseGreenfieldForDestination (GetAddress (station))), 800, GetNumberOfAntennas (), nss, 0, GetChannelWidthForMode (mode), GetAggregation (station), false);
}

bool
ObssWifiManager::IsLowLatency (void) const
{
  return true;
}

void
ObssWifiManager::UpdatePathLoss(HePreambleParameters params)
{
  NS_LOG_FUNCTION (this<< +m_myMac);

  double loss = WToDbm(params.rssiW) - params.txpower/10.0 ; // negative value(dbm)

  //TO DO: more complex updating method
    for (uint8_t i=0; i< m_lossPairs.size(); i++)
    {
      uint8_t temp_src = std::get<1>(m_lossPairs[i]);
      double temp_loss = std::get<2>(m_lossPairs[i]);
      if(params.src == temp_src)
      {
        if(temp_loss != loss)
        {
          // std::cout<<"different loss, old: "<< i->second << " new: "<< loss<< std::endl; 
          NS_LOG_DEBUG("different loss, old: "<< temp_loss << " new: "<< loss);
          // should not be here, since path loss of ns3 never changes
        }
        return;
      }
    }

    // no match
    PathLossPair pair(m_myMac, params.src, loss);
    m_lossPairs.push_back(pair);
    // std::cout<<"Node mac "<< +m_myMac << " pushed src="<< +params.src <<" loss= "<<loss<<std::endl;
    NS_LOG_DEBUG("Node mac "<< +m_myMac << " pushed src="<< +params.src <<" loss= "<<loss);

    // update global loss pairs

    for (uint8_t i=0; i<globalLossPairs.size(); i++)
    {
      uint8_t i_dst = std::get<0>(globalLossPairs[i]);
      uint8_t i_src = std::get<1>(globalLossPairs[i]);
      double i_loss = std::get<2>(globalLossPairs[i]);
      if(params.src == i_src && m_myMac == i_dst )
      {
        if(i_loss != loss)
        {
          // std::cout<<"global different loss, old: "<< i->second << " new: "<< loss<< std::endl; 
          NS_LOG_DEBUG("global different loss, old: "<< i_loss << " new: "<< loss);
        }
        return;
      }
    }
    globalLossPairs.push_back(pair);
    PathLossPair pair2(params.src, m_myMac, loss); // both ways use same loss
    globalLossPairs.push_back(pair2);
    // std::cout<<"Global pushed : dst= "<< +m_myMac << "  src="<< +params.src <<" loss= "<<loss<<std::endl;
    NS_LOG_DEBUG("Global pushed : dst= "<< +m_myMac << "  src="<< +params.src <<" loss= "<<loss);

    return;
}

void
ObssWifiManager::ReceiveHeSig(HePreambleParameters params)
{
  NS_LOG_FUNCTION (this<< +m_myMac);
  NS_LOG_DEBUG("\nReceive HeSig");
  // std::cout<< "Mymac "<<+m_myMac <<"\t Dst "<<(int)params.dst <<" Src "<< (int)params.src<< 
  //  "  Power " << +params.txpower<<" Rssi: "<<WToDbm (params.rssiW) <<" Duration" << +params.time<< "  Mcs "<< +params.mcs <<std::endl;
  NS_LOG_DEBUG("Mymac "<<+m_myMac <<"\t Dst "<<(int)params.dst <<" Src "<< (int)params.src<< \
  "  Power " << +params.txpower<<" Rssi: "<<WToDbm (params.rssiW) <<"dbm  Duration" << +params.time<< "*1e4 NS  Mcs "<< +params.mcs );

  UpdatePathLoss(params);
  UpdateObssTransStatus(params);
  CheckObssStatus();
  ResetPhy();

  return;
}

void
ObssWifiManager::UpdateObssTransStatus(HePreambleParameters params)
{
  NS_LOG_FUNCTION (this<< +m_myMac);

  ObssTran tran(params.dst, params.src, Simulator::Now().ToInteger(Time::Unit::NS),\
   (uint64_t)params.time * 1e4, (double)params.txpower / 10.0, params.mcs);
  // tran.dst = params.dst;
  // tran.src = params.src;
  // tran.startTime = Simulator::Now().ToInteger(Time::Unit::NS);
  // tran.duration = (uint64_t)params.time * 1e4 ; // ns 
  // tran.txPower = (double)params.txpower / 10.0;
  m_obssTrans.push_back(tran);
}

void
ObssWifiManager::CheckObssStatus()
{
  NS_LOG_FUNCTION (this<< +m_myMac);

  // std::cout<<"checkObssStatus"<<std::endl;
  NS_LOG_DEBUG("checkObssStatus");
  //check timer
  for(int idx=0; idx<(int)m_obssTrans.size(); idx++)
  {
    double startTime = std::get<2>(m_obssTrans[idx]);
    double duration = std::get<3>(m_obssTrans[idx]);
    // std::cout<<"start "<<startTime<< " duration "<<duration<<"  Now "<< +Simulator::Now().ToInteger(Time::Unit::NS)<<std::endl;
    NS_LOG_DEBUG("start "<<startTime<< " duration "<<duration<<"  Now "<< +Simulator::Now().ToInteger(Time::Unit::NS));
    if(startTime+duration < Simulator::Now().ToInteger(Time::Unit::NS))
    {
      // time past, delete it
      m_obssTrans.erase(m_obssTrans.begin()+idx);
    }

  }
  // std::cout<<"m_obssTrans.size= "<<m_obssTrans.size()<<std::endl;
  NS_LOG_DEBUG("m_obssTrans.size= "<<m_obssTrans.size());
  if(m_obssTrans.size()==0 || !CheckRouting())
  {
    m_obssRestricted = false;
    return;
  }
  // #######
  // TO DO: Update power and rate limit

  double interference = 0;
  double interference2 =0;
  ReceiverInfos recvinfos;
  ReceiverInfos srcinfos;

  // sum up interference
  //for nexthop
  for(int idx=0; idx<(int)m_obssTrans.size(); idx++)
  {
    uint8_t temp_src = std::get<1>(m_obssTrans[idx]);
    double temp_txpower = std::get<4>(m_obssTrans[idx]);
    double temp_loss = GetPathLoss(m_nexthopMac, temp_src);
    double temp_loss2 = GetPathLoss(m_myMac, temp_src);
    // std::cout<<"dst: "<<+m_nexthopMac<<" src: "<<+temp_src<<" loss: "<<temp_loss<<std::endl;

    interference += DbmToW(temp_txpower + temp_loss);
    interference2 += DbmToW(temp_txpower + temp_loss2);
  }
  ReceiverInfo recvInfo(m_nexthopMac, interference, 0, -1); // first one
  recvinfos.push_back(recvInfo);

  NS_LOG_DEBUG("ack signal= "<< (double)(GetPhy()->GetTxPowerEnd() + GetPathLoss(m_myMac, m_nexthopMac)) );
  ReceiverInfo srcinfo(m_myMac, interference2, DbmToW(GetPhy()->GetTxPowerEnd() + GetPathLoss(m_myMac, m_nexthopMac)), -1); // first one Src
  srcinfos.push_back(srcinfo);

  // add other receivers & srouces
  double signal = 0;
  double signal2 = 0;
  for(int idx=0; idx<(int)m_obssTrans.size(); idx++)
  {
    uint8_t temp_dst = std::get<0>(m_obssTrans[idx]);
    uint8_t temp_src = std::get<1>(m_obssTrans[idx]);
    uint8_t temp_mcs = std::get<5>(m_obssTrans[idx]);
    interference = 0;
    signal = 0;
    for(int idx2=0; idx2<(int)m_obssTrans.size(); idx2++)
    {
      uint8_t temp_dst2 = std::get<0>(m_obssTrans[idx2]);
      uint8_t temp_src2 = std::get<1>(m_obssTrans[idx2]);
      double temp_txpower = std::get<4>(m_obssTrans[idx2]);
      double temp_loss = GetPathLoss(temp_dst, temp_src2);

      if(temp_dst2 == temp_dst) // not inf but signal !
      {
        signal = DbmToW(temp_txpower + temp_loss);
      }
      else
      {
        interference += DbmToW(temp_txpower + temp_loss); 
      }

      if(temp_src2==temp_src)
      {
        double temp_loss2 = GetPathLoss(temp_src, temp_dst);
        signal2 = DbmToW(GetPhy()->GetTxPowerEnd() + temp_loss2); // Ack with highest power
      }
      else
      {
        double temp_loss2 = GetPathLoss(temp_src, temp_src2);
        interference2 += DbmToW(temp_txpower + temp_loss2);
      }
      

    }
    ReceiverInfo recvInfo(temp_dst, interference, signal, temp_mcs);
    recvinfos.push_back(recvInfo);
    if(signal2==0)NS_LOG_DEBUG("singal2 ==0");
    ReceiverInfo srcinfo(temp_src, interference2, signal2, -1);
    srcinfos.push_back(srcinfo);
  }

  bool isOk;
  double myTxpower = 0;
  int level;
  // std::cout<<"we have "<<recvinfos.size()<<" recv info"<<std::endl;
  NS_LOG_DEBUG("Checking txpower. we have "<<recvinfos.size()<<" recv info");
  for(level=GetPhy()->GetNTxPower()-1;level>=0;level--)
  {
    myTxpower = GetPhy()->GetPowerDbm(level); //dbm
    isOk=true;

    // must not disturb on-going transimission
    NS_LOG_DEBUG("must not disturb on-going transimission");
    for(int idx=1;idx<(int)recvinfos.size();idx++)
    {
      uint8_t temp_recv = std::get<0>(recvinfos[idx]);
      double temp_inf = std::get<1>(recvinfos[idx]);
      double temp_signal = std::get<2>(recvinfos[idx]);
      int temp_mcs = std::get<3>(recvinfos[idx]);
      double temp_loss = GetPathLoss(temp_recv, m_myMac);
      if(temp_loss>0) // no path loss yet
      {
        m_obssRestricted = false;
        return;
      }
      
      WifiMode mode = GetPhy()->GetMcs(temp_mcs + HEMCS0); // Use HeMcs
      // std::cout<<"mcs: "<< temp_mcs << " name: "<< mode.GetUniqueName()<< std::endl;
      NS_LOG_DEBUG("mcs: "<< temp_mcs << " name: "<< mode.GetUniqueName());

      double myI = DbmToW(myTxpower+ temp_loss); // the interference I will introduce (W)
      uint16_t channelWidth = GetPhy ()->GetChannelWidth ();
      // std::cout<<"loss= "<<temp_loss<<" interference= "<<WToDbm(myI+temp_inf)<<" signal= "<<WToDbm(temp_signal)<<" ch= "<<channelWidth<<std::endl;
      NS_LOG_DEBUG("loss= "<<temp_loss<<" interference= "<<WToDbm(myI+temp_inf)<<" signal= "<<WToDbm(temp_signal)<<" ch= "<<channelWidth);
      double dstSNR = WToDbm(CalculateSnr(temp_signal, myI+temp_inf, channelWidth));

      WifiTxVector txVector;
      txVector.SetChannelWidth (channelWidth);
      txVector.SetNss (1);
      txVector.SetMode (mode);
      double SNRlimit = WToDbm( GetPhy()->CalculateSnr(txVector, m_ber)) - SNRMARGIN;

      // std::cout<<"mypower= "<<myTxpower<<" SNRlimit= "<<SNRlimit<<" dstSNR= "<<dstSNR<<std::endl;
      NS_LOG_DEBUG("mymac="<<+m_myMac<<" mypower= "<<myTxpower<<" SNRlimit= "<<SNRlimit<<" dstSNR= "<<dstSNR);

      if(dstSNR<SNRlimit)
      {
        isOk=false;
        break;
      }
    }
    if(!isOk)continue;

    //  must not disturb future ack
    NS_LOG_DEBUG("must not disturb future ack");
    for(int idx=0;idx<(int)srcinfos.size();idx++)
    {
      uint8_t temp_src = std::get<0>(srcinfos[idx]);
      double temp_inf = std::get<1>(srcinfos[idx]);
      double temp_signal = std::get<2>(srcinfos[idx]);
      NS_LOG_DEBUG("temp_signal= "<< temp_signal);
      if(temp_signal ==0)NS_LOG_DEBUG("Zero!");
      // int temp_mcs = std::get<3>(recvinfos[idx]);
      double temp_loss;

      // if(GetNBasicMcs()==0){m_obssRestricted=false;return;}

      // WifiMode mode = GetBasicMode(2); // Ack Basic Mode
      WifiMode mode = GetPhy()->GetOfdmRate24Mbps(); // Ack Basic Mode
      // std::cout<<"mcs: "<< temp_mcs << " name: "<< mode.GetUniqueName()<< std::endl;
      NS_LOG_DEBUG("basic ack mcs: "<< 2 << " name: "<< mode.GetUniqueName());

      double myI;
      if(idx==0)
      {
        temp_loss = GetPathLoss(m_myMac, m_nexthopMac);
        myI = 0;
      }
      else
      {
        temp_loss = GetPathLoss(temp_src, m_myMac);
        myI = DbmToW(myTxpower+ temp_loss); // the interference I will introduce (W)        
      }
      uint16_t channelWidth = GetPhy ()->GetChannelWidth ();
      // std::cout<<"loss= "<<temp_loss<<" interference= "<<WToDbm(myI+temp_inf)<<" signal= "<<WToDbm(temp_signal)<<" ch= "<<channelWidth<<std::endl;
      NS_LOG_DEBUG("loss= "<<temp_loss<<" interference= "<<WToDbm(myI+temp_inf)<<" signal= "<<WToDbm(temp_signal)<<" ch= "<<channelWidth);
      double dstSNR = WToDbm(CalculateSnr(temp_signal, myI+temp_inf, channelWidth));

      WifiTxVector txVector;
      txVector.SetChannelWidth (channelWidth);
      txVector.SetNss (1);
      txVector.SetMode (mode);
      double SNRlimit = WToDbm( GetPhy()->CalculateSnr(txVector, m_ber)) - ACKSNRMARGIN;

      // std::cout<<"mypower= "<<myTxpower<<" SNRlimit= "<<SNRlimit<<" dstSNR= "<<dstSNR<<std::endl;
      NS_LOG_DEBUG("mymac="<<+m_myMac<<" mypower= "<<myTxpower<<" SNRlimit= "<<SNRlimit<<" dstSNR= "<<dstSNR);

      if(dstSNR<SNRlimit)
      {
        isOk=false;
        break;
      }

    }

    if(isOk)
    {
      // std::cout<<"Found right txpower!"<<myTxpower<<std::endl;
      NS_LOG_DEBUG("Found right txpower!"<<myTxpower);
      break;
    }
    else
    {
      NS_LOG_DEBUG("Not Yet find txpower!"<<myTxpower);
    }
  }
  
  if(!isOk)
  {
    NS_LOG_DEBUG("No power finally.");
    m_obssRestricted=false;
    return;
  }

  uint8_t temp_recv = std::get<0>(recvinfos[0]);
  double temp_inf = std::get<1>(recvinfos[0]);
  double temp_loss = GetPathLoss(temp_recv, m_myMac);

  uint16_t channelWidth = GetPhy ()->GetChannelWidth ();
  // std::cout<<"loss= "<<temp_loss<<" interference= "<<WToDbm(temp_inf)<<" signal= "<<(myTxpower+temp_loss)<<" ch= "<<channelWidth<<std::endl;
  NS_LOG_DEBUG("loss= "<<temp_loss<<" interference= "<<WToDbm(temp_inf)<<" signal= "<<(myTxpower+temp_loss)<<" ch= "<<channelWidth);
  double SNR = WToDbm(CalculateSnr(DbmToW(myTxpower+temp_loss), temp_inf, channelWidth));
  int mcs;
  double SNRlimit;
  for(mcs=9;mcs>=0;mcs--)
  {
    WifiMode mode = GetPhy()->GetMcs(mcs + HEMCS0);
    WifiTxVector txVector;
    txVector.SetChannelWidth (channelWidth);
    txVector.SetNss (1);
    txVector.SetMode (mode);
    SNRlimit = WToDbm( GetPhy()->CalculateSnr(txVector, m_ber)) - SNRMARGIN;
    if(SNRlimit<SNR)break;
  }
  // std::cout<<"final mcs: "<<mcs<<" SNRlimit:"<<SNRlimit<<" SNR:"<<SNR<<std::endl;
  NS_LOG_DEBUG("final mcs: "<<mcs<<" SNRlimit:"<<SNRlimit<<" SNR:"<<SNR <<" dst="<<+m_nexthopMac <<" src="<<+m_myMac);
  if(mcs<0)
  {
    m_obssRestricted=false;
    return;
  }
  m_obssPowerLimit = level;
  m_obssMcsLimit = mcs;
  m_obssRestricted = true; 
}

double 
ObssWifiManager::CalculateSnr (double signal, double noiseInterference, uint16_t channelWidth)
{
  double noiseFigure = 7; // default value of wifi-phy attribute

  //thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  //Nt is the power of thermal noise in W
  double Nt = BOLTZMANN * 290 * channelWidth * 1e6;
  //receiver noise Floor (W) which accounts for thermal noise and non-idealities of the receiver
  double noiseFloor = noiseFigure * Nt;
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise; //linear scale
  // NS_LOG_DEBUG ("bandwidth(MHz)=" << channelWidth << ", signal(W)= " << signal << ", noise(W)=" << noiseFloor << ", interference(W)=" << noiseInterference << ", snr=" << RatioToDb(snr) << "dB");
  
  return snr;
}

double
ObssWifiManager::GetPathLoss(uint8_t dst, uint8_t src)
{

  for (uint8_t i=0; i<globalLossPairs.size(); i++)
  {
    uint8_t i_dst = std::get<0>(globalLossPairs[i]);
    uint8_t i_src = std::get<1>(globalLossPairs[i]);
    double i_loss = std::get<2>(globalLossPairs[i]);
    if(src == i_src && dst == i_dst )
    {
      NS_LOG_DEBUG("dst: "<<+dst<<" src: "<<+src<<" loss: "<<i_loss);
      return i_loss;
    }
  }
  {
    // std::cout<<"No match loss for "<<+src<<" to "<<+dst<< " give loss= -140"<<std::endl;
    NS_LOG_DEBUG("No match loss for "<<+src<<" to "<<+dst<< " give loss= "<<DEFAULTLOSS);
    return DEFAULTLOSS;
  }
  // std::cout<<"No loss for dst "<<+dst<<" src "<<+src<<std::endl;
  // return 1; // loss must be negative, so use 1 to indicate no match #####
}


// check routing table, 
// return false if no nexthop or the params.src==myNexthop || params.dst == myMac
bool
ObssWifiManager::CheckRouting()
{
  uint8_t addrs2[4]; // nexthop ip

  Ptr<Ipv4RoutingProtocol> aodvRouting;
  aodvRouting = Ipv4RoutingHelper::GetRouting <Ipv4RoutingProtocol> (m_device->GetNode()->GetObject<Ipv4> ()->GetRoutingProtocol ());
  // aodvRouting = m_device->GetNode()->GetObject<Ipv4> ()->GetRoutingProtocol ();
  Ipv4Address sourceAddr = m_device->GetNode()->GetObject<Ipv4> ()->GetAddress(1,0).GetLocal();
  Ipv4Header ipHead;
  ipHead.SetDestination("10.2.1.1");
  ipHead.SetSource(sourceAddr);
  Socket::SocketErrno err;
  Packet ptemp;
  Ptr<Ipv4Route> routeEntry = aodvRouting->RouteOutput(&ptemp, ipHead, m_device, err);

  if(!err)
  {
      // std::cout<<"  myAddr= "<<sourceAddr<<"  nextHop= "<<routeEntry->GetGateway() <<std::endl;
      routeEntry->GetGateway().Serialize(addrs2);
      m_nexthopMac = addrs2[3];

      // check all on-going transmission
      for(int idx=0; idx<(int)m_obssTrans.size(); idx++)
      {
        uint8_t tran_dst = std::get<0>(m_obssTrans[idx]);
        uint8_t tran_src = std::get<1>(m_obssTrans[idx]);

        if(tran_src == m_nexthopMac || tran_dst == m_nexthopMac || tran_dst ==m_myMac || addrs2[0]==127)
        {
          NS_LOG_DEBUG("loopback addr or not a obss frame.");
          // std::cout<<"loopback addr or not a obss frame."<<std::endl;
          return false;
        }
      }

      return true;      
  }
  else
  {
    // std::cout<<"myAddr= "<<sourceAddr<<"  error routing , no nexthop"<<std::endl;  
    NS_LOG_DEBUG("myAddr= "<<sourceAddr<<"  error routing , no nexthop");
    return false;
  }
  
}


void
ObssWifiManager::ResetPhy()
{
  if(m_obssRestricted && TransNum > TRANLIMIT)
  {
    GetPhy()->ResetCca(false, 25, 25);
    // std::cout<<"Phy Reset!"<<std::endl;
    // NS_LOG_DEBUG("Phy Reset!");
    // std::cout<<"reset\n";
    ResetNum++;
  }
  return;
}

} //namespace ns3

