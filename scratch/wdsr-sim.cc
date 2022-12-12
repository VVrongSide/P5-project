/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Yufei Cheng
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
 * Author: Yufei Cheng   <yfcheng@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/wdsr-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/stats-module.h"
#include <sstream>
#include "ns3/wifi-radio-energy-model-helper.h"

using namespace ns3;

Ptr<GnuplotAggregator> aggregator;
void TotalEnergy (std::string context, double oldValue, double totalEnergy) {
    aggregator->Write2d(context, Simulator::Now().GetSeconds(), totalEnergy);
    //NS_LOG_UNCOND ("%INFO TimeStamp: "<<Simulator::Now ().GetSeconds ()<<" secs Total energy consumed Node: "<<context<<" "<<totalEnergy<< " Joules");
}

void depletion(double value){
    NS_LOG_UNCOND ("i got value:" << value);
}

NS_LOG_COMPONENT_DEFINE("WDsrTest");

int
main(int argc, char* argv[])
{
    //
    // Users may find it convenient to turn on explicit debugging
    // for selected modules; the below lines suggest how to do this
    //
#if 0
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_ALL);
  LogComponentEnable ("NetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv4EndPointDemux", LOG_LEVEL_ALL);
#endif

#if 0
  LogComponentEnable ("WDsrOptions", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrRouting", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrOptionHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrFsHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrGraReplyTable", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrSendBuffer", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrRouteCache", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrMaintainBuffer", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrRreqTable", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrErrorBuffer", LOG_LEVEL_ALL);
  LogComponentEnable ("WDsrNetworkQueue", LOG_LEVEL_ALL);
#endif

    NS_LOG_INFO("creating the nodes");

    // General parameters
    uint32_t nWifis = 20;
    uint32_t nSinks = 10;
    double TotalTime = 200.0;
    double dataTime = 190.0;
    double ppers = 100;
    uint32_t packetSize = 1024;
    double dataStart = 1.0; // start sending data at 10s
    uint32_t seed = 2;

    //energymodel
    double initialEnergy = 100; // joule
    double voltage = 3.0;       // volts
    double txPowerEnd = 36.0;   // dbm
    double txPowerStart = txPowerEnd;  // dbm
    double idleCurrent = 0.273; // Ampere
    double txCurrent = DbmToW(txPowerEnd)/voltage*1.5;   // Ampere
    printf("imma transmit with %.2lfW for this test\n",  DbmToW(txPowerEnd));

    std::string rate = "1Mbps";
    std::string dataMode("DsssRate11Mbps");
    std::string phyMode("DsssRate11Mbps");

    // Allow users to override the default parameters and set it to new ones from CommandLine.
    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifis", "Number of wifi nodes", nWifis);
    cmd.AddValue("nSinks", "Number of SINK traffic nodes", nSinks);
    cmd.AddValue("rate", "CBR traffic rate(in kbps), Default:8", rate);
    cmd.AddValue("packetSize", "The packet size", packetSize);
    cmd.AddValue("seed", "Seed used for random placement, Default:1", seed);
    cmd.Parse(argc, argv);

    /********* Gnuplot ***********/
    std::string plotXAxisHeading = "Time (seconds)";
    std::string plotYAxisHeading = "Energy";
    aggregator = CreateObject<GnuplotAggregator>("Wdsr-plot");
    aggregator->SetTerminal("pdf");
    aggregator->SetTitle("Energy remaining");
    aggregator->SetLegend(plotXAxisHeading, plotYAxisHeading);
    /******************************/

    SeedManager::SetSeed(10);
    SeedManager::SetRun(1);
    NodeContainer adhocNodes;
    adhocNodes.Create(nWifis);
    NetDeviceContainer allDevices;

    NS_LOG_INFO("setting the default phy and channel parameters");
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));

    NS_LOG_INFO("setting the default phy and channel parameters ");
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    YansWifiPhyHelper wifiPhy;

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                    "Frequency", DoubleValue(2.45e9));
    wifiPhy.Set("TxPowerStart", DoubleValue(txPowerStart));
    wifiPhy.Set("TxPowerEnd", DoubleValue(txPowerEnd));
    //wifiPhy.Set("TxPowerLevels", UintegerValue(nTxPowerLevels));
    wifiPhy.SetChannel(wifiChannel.Create());

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue(dataMode),
                                 "ControlMode", StringValue(phyMode));

    wifiMac.SetType("ns3::AdhocWifiMac");
    allDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);

    NS_LOG_INFO("Configure Tracing.");
#if 0 //trace file + pcap
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("wdsrtest.tr");
    wifiPhy.EnableAsciiAll(stream);
    wifiPhy.EnablePcapAll("wdsr-sim");
#endif
    /******* Placement *******/
    RngSeedManager::SetSeed(seed);
    RngSeedManager::SetRun(1);
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                    "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=15000]"),
                                    "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=15000]"),
                                    "Z", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=50.0]"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(adhocNodes);
    /**************************/

    Ptr<BasicEnergySource> energySources[adhocNodes.GetN()];
    DeviceEnergyModelContainer deviceModels[adhocNodes.GetN()];
    WifiRadioEnergyModelHelper energyHelper;
    // **** configure radio energy model*****
    // IdleCurrentA:      The default radio Idle current in Ampere.
    // CcaBusyCurrentA:   The default radio CCA Busy State current in Ampere.
    // TxCurrentA:        The radio Tx current in Ampere.
    // RxCurrentA:        The radio Rx current in Ampere.
    // SwitchingCurrentA: The default radio Channel Switch current in Ampere.
    // SleepCurrentA:     The radio Sleep current in Ampere.
    // TxCurrentModel:    A pointer to the attached tx current model.
    energyHelper.Set("IdleCurrentA", DoubleValue(idleCurrent));
    energyHelper.Set("TxCurrentA", DoubleValue(txCurrent));
    double eta = DbmToW(txPowerStart) / ((txCurrent - idleCurrent) * voltage);
    NS_LOG_UNCOND("eta feta is: " << eta);
    energyHelper.SetTxCurrentModel("ns3::LinearWifiTxCurrentModel",
                                    "Voltage", DoubleValue(voltage),
                                    "IdleCurrent", DoubleValue(idleCurrent),
                                    "Eta", DoubleValue(eta));
    WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback = MakeCallback(&depletion);
    energyHelper.SetDepletionCallback(callback);


    for (uint32_t i = 0; i < adhocNodes.GetN();i++){
        energySources[i] = CreateObject<BasicEnergySource>();
        energySources[i]->SetInitialEnergy(initialEnergy);
        energySources[i]->SetSupplyVoltage(voltage);

        energySources[i]->SetNode(adhocNodes.Get(i));
        deviceModels[i] = energyHelper.Install(allDevices.Get(i), energySources[i]);
        adhocNodes.Get(i)->AggregateObject(energySources[i]);
    }

    /********* CSV generator energy node ********/
    for (uint32_t i = 0; i < adhocNodes.GetN(); ++i) {
        Ptr<BasicEnergySource> NodeEnergySource = adhocNodes.Get(i)->GetObject<BasicEnergySource>();
        if (NodeEnergySource != NULL) {
            std::string context = std::to_string(i);
            NodeEnergySource->TraceConnect("RemainingEnergy",
                                            context,
                                            MakeCallback(&TotalEnergy));
            aggregator->Add2dDataset(context, std::string("Node: ") + context);
        }
    }
    aggregator->Enable();
    /********************************************/

    InternetStackHelper internet;
    WDsrMainHelper wdsrMain;
    WDsrHelper wdsr;
    internet.Install(adhocNodes);
    wdsrMain.Install(wdsr, adhocNodes);

    NS_LOG_INFO("assigning ip address");
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer allInterfaces;
    allInterfaces = address.Assign(allDevices);

    uint16_t port = 9;
    double randomStartTime =
        (1 / ppers) / nSinks; // distributed btw 1s evenly as we are sending 4pkt/s
#if 1
    for (uint32_t i = 0; i < nSinks; ++i)
    {
        PacketSinkHelper sink("ns3::UdpSocketFactory",
                              InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer apps_sink = sink.Install(adhocNodes.Get(i));
        apps_sink.Start(Seconds(0.0));
        apps_sink.Stop(Seconds(TotalTime));

        OnOffHelper onoff1("ns3::UdpSocketFactory",
                           Address(InetSocketAddress(allInterfaces.GetAddress(i), port)));
        onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        onoff1.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff1.SetAttribute("DataRate", DataRateValue(DataRate(rate)));

        ApplicationContainer apps1 = onoff1.Install(adhocNodes.Get(i + nWifis - nSinks));
        apps1.Start(Seconds(dataStart + i * randomStartTime));
        apps1.Stop(Seconds(dataTime + i * randomStartTime));
    }
#else
    double m_packetInterval = 0.4;
    uint16_t portNumber = 9;
    UdpEchoServerHelper echoServer(portNumber);
    uint16_t sinkNodeId = 3;
    ApplicationContainer serverApps = echoServer.Install(adhocNodes.Get(sinkNodeId));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(dataTime + 1));
    UdpEchoClientHelper echoClient(allInterfaces.GetAddress(sinkNodeId), portNumber);
    echoClient.SetAttribute("MaxPackets",
                            UintegerValue((uint32_t)(dataTime * (1 / m_packetInterval))));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(m_packetInterval)));
    echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));
    ApplicationContainer clientApps = echoClient.Install(adhocNodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(TotalTime));
#endif

    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(TotalTime));
    /******* Animation *******/
#if 0 //(dis/en)able animation
    AnimationInterface anim("wdsr-sim.xml"); // Mandatory
    anim.EnablePacketMetadata(); // Optional
    anim.EnableIpv4RouteTracking("routingtable-wire.xml",
                                Seconds(0),             //Start at 0
                                Seconds(TotalTime),     //Run to totaltime
                                Seconds(TotalTime/25)); //25 timesteps  
    anim.EnableWifiMacCounters(Seconds(0), Seconds(TotalTime)); // Optional
    anim.EnableWifiPhyCounters(Seconds(0), Seconds(TotalTime)); // Optional
    for (uint32_t i = 0; i < adhocNodes.GetN(); ++i) {
        anim.UpdateNodeDescription(adhocNodes.Get(i), "Fuckers"); // Optional
        anim.UpdateNodeColor(adhocNodes.Get(i), 0, 255, 0);  // Optional
        anim.UpdateNodeSize(i, 200, 200);
    }
#endif
    /*************************/
    Simulator::Run();
    Simulator::Destroy();
}
