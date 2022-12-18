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
#include "ns3/dsr-module.h"
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
#include "ns3/wifi-net-device.h"


using namespace ns3;

/*****
*   Todos
*   [ ] correct parameters.
*   [ ] new energy.
*   [ ] plot with new energy model.
*   [ ] Better transmission for random.
*/

uint32_t nWifis = 20;
bool fixed = 0;
bool logonce;

//double initialEnergy[100];
double remainingEnergy[100];
uint32_t packets[100];

// Initializing extern variables (Type should only be cast here)
uint8_t γ;
uint8_t α;
double initialEnergy;
double Oldtime;
double idleW = 0.5;
NetDeviceContainer allDevices;
Timer logging;
std::string rate = "1Mbps";

void CalcIdleAll() {
    double newtime = ns3::Simulator::Now().GetSeconds(); //namespace ns3?
    for (uint32_t i = 0; i < nWifis; i++){
        if (remainingEnergy[i] > 0.0){
            remainingEnergy[i] -= (idleW*(newtime-Oldtime));
        }
    }
    Oldtime = newtime;
}


Ptr<GnuplotAggregator> aggregator;
Ptr<GnuplotAggregator> depletedAggregator;
void Logger() {
    CalcIdleAll();
    uint32_t depleted = nWifis;
    for (uint32_t i = 0; i < nWifis; i++){
        //NS_LOG_UNCOND ("node "<<i<<"energy "<<remainingEnergy[i]<<" s "<<Simulator::Now().GetSeconds());
        aggregator->Write2d(std::to_string(i), Simulator::Now().GetSeconds(), remainingEnergy[i]);
        if (remainingEnergy[i] <= 0.0) {
            depleted--;
            if (allDevices.Get(i)->GetObject<WifiNetDevice>()->GetPhy()->IsStateIdle())
                allDevices.Get(i)->GetObject<WifiNetDevice>()->GetPhy()->SetOffMode();
        }
    }
    depletedAggregator->Write2d("Nodes Alive", Simulator::Now().GetSeconds(), depleted);
    if (depleted == nWifis-1 && !logonce) {
        printf("%lf\n", Simulator::Now().GetSeconds());
        logonce = 1;
        //Simulator::Destroy();
    }
    logging.Schedule();
}

void txsniff(std::string nodeID, Ptr< const Packet > packet, double txPowerW) {
    //NS_LOG_UNCOND ("node "<<nodeID<<" time "<<Simulator::Now ().GetSeconds ()<<" power "<<txPowerW<<" size "<<packet->GetSize());
    remainingEnergy[std::stoi(nodeID)] -= txPowerW*DataRate(rate).CalculateBytesTxTime(packet->GetSize()).GetSeconds();
    packets[std::stoi(nodeID)] += packet->GetSize();
}

uint32_t port;
Ptr<Socket>
SetupPacketReceive(Ipv4Address addr, Ptr<Node> node)
{
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    Ptr<Socket> sink = Socket::CreateSocket(node, tid);
    InetSocketAddress local = InetSocketAddress(addr, port);
    sink->Bind(local);
    //sink->SetRecvCallback(MakeCallback(&RoutingExperiment::ReceivePacket, this));

    return sink;
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
    //LogComponentEnable("WifiRadioEnergyModel", LOG_DEBUG);
    NS_LOG_INFO("creating the nodes");

    // General parameters
    uint32_t nSinks = 10;
    double TotalTime = 100.0;
    //double dataTime = 19.0;
    double ppers = 100;
    uint32_t packetSize = 1000;
    double dataStart = 0.0; // start sending data at 10s
    uint32_t seed = 3;
    int runDSR = 0;
    int echo = 0;
    double logginginterval = 0.01;
    γ = 120;
    α = 5;

    //energymodel
    initialEnergy = 50; // joule
    double txPowerEnd = 36.0;   // dbm
    double txPowerStart = txPowerEnd;  // dbm

    
    std::string dataMode("DsssRate11Mbps");
    std::string phyMode("DsssRate11Mbps");

    // Allow users to override the default parameters and set it to new ones from CommandLine.
    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifis", "Number of wifi nodes", nWifis);
    cmd.AddValue("nSinks", "Number of SINK traffic nodes", nSinks);
    cmd.AddValue("rate", "CBR traffic rate(in kbps), Default:8", rate);
    cmd.AddValue("packetSize", "The packet size", packetSize);
    cmd.AddValue("seed", "Seed used for random placement, Default:1", seed);
    cmd.AddValue("comp", "Compare WDSR to DSR, Default: 0", runDSR);
    cmd.AddValue("fixed", "Run with fixed position, Default: 0", fixed);
    cmd.AddValue("gamma", "gamma/threshold value, Default: 120", γ);
    cmd.AddValue("alpha", "alpha value (in S), Default: 5", α);
    cmd.AddValue("echo", "EchoServer on/off, Default: 0", echo);
    cmd.Parse(argc, argv);

    if (fixed) {
        nWifis = 9;
        nSinks = 2;
    }
    for (int dsr = 0; dsr <= runDSR; dsr++) { //1 for only WDSR 2 for Compare.
    Oldtime = 0;
    logonce = 0;

    /********* Gnuplot ***********/
    aggregator = CreateObject<GnuplotAggregator>(dsr ? "Dsr-plot" : "Wdsr-plot");
    aggregator->SetTerminal("pdf");
    aggregator->SetTitle(dsr ? "Energy remaining (DSR)" : "Energy remaining (Wdsr)" );
    aggregator->SetLegend("Time (seconds)", "Energy (J)");  // (x-axis, y-axis)
    
    depletedAggregator = CreateObject<GnuplotAggregator>(dsr ? "DsrAlive-plot" : "WdsrAlive-plot");
    depletedAggregator->SetTerminal("pdf");
    depletedAggregator->SetTitle("Nodes Alive");
    depletedAggregator->SetLegend("Time (seconds)", "Number of nodes alive");
    
    logging.SetDelay(Seconds(logginginterval));
    logging.SetFunction(&Logger);
    logging.Schedule();
    /******************************/

    RngSeedManager::SetSeed(seed);
    RngSeedManager::SetRun(1);
    NodeContainer adhocNodes;
    adhocNodes.Create(nWifis);


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
    if (!fixed){
      wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                      "Frequency", DoubleValue(2.45e9));
    }
    else{
      wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                     "MaxRange",
                                     DoubleValue(110));
    }
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
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream(dsr ? "dsrtest.tr" : "wdsrtest.tr");
    wifiPhy.EnableAsciiAll(stream);
    wifiPhy.EnablePcapAll(dsr ? "dsr-sim" : "wdsr-sim");
#endif
    /******* Placement *******/
    MobilityHelper mobility;
    if (fixed) {
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
        positionAlloc->Add(Vector(9, 75, 0.0));
        positionAlloc->Add(Vector(75, 0.0, 0.0));
        positionAlloc->Add(Vector(75+100, 0.0, 0.0));
        positionAlloc->Add(Vector(75+200, 16.0, 0.0));
        positionAlloc->Add(Vector(271, 115.0, 0.0));
        positionAlloc->Add(Vector(75, 2*75.0, 0.0));
        positionAlloc->Add(Vector(170, 114, 0.0));
        positionAlloc->Add(Vector(134, 232, 0.0));
        positionAlloc->Add(Vector(233, 210.0, 0.0));
        mobility.SetPositionAllocator(positionAlloc);
    } else {
        mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                    "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=1500]"),
                                    "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=1500]"),
                                    "Z", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=50.0]"));
    }
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(adhocNodes);
    /**************************/

    for (uint32_t i = 0; i < allDevices.GetN(); ++i) {
        Ptr<NetDevice> staDevicePtr = allDevices.Get(i);
        Ptr<WifiPhy> wifiPhyPtr = staDevicePtr->GetObject<WifiNetDevice>()->GetPhy();
        if (wifiPhyPtr != NULL) {
            NS_ASSERT(staDevicePtr == adhocNodes.Get(i)->GetDevice(0));
            std::string context = std::to_string(i);
            wifiPhyPtr->TraceConnect("PhyTxBegin",context, MakeCallback(&txsniff));
            remainingEnergy[i] = initialEnergy;
            aggregator->Add2dDataset(context, std::string("Node: ") + context);
            packets[i] = 0;
        }
    }
    depletedAggregator->Add2dDataset("Nodes Alive", std::string("Nodes Alive: "));
    depletedAggregator->Set2dDatasetStyle("Nodes Alive", Gnuplot2dDataset::Style::STEPS);
    depletedAggregator->Enable();
    aggregator->Enable();

    InternetStackHelper internet;
    DsrMainHelper dsrMain;
    DsrHelper dsrH;
    dsrH.Set("CacheType", StringValue("PathCache"));
    WDsrMainHelper wdsrMain;
    WDsrHelper wdsr;
    wdsr.Set("CacheType", StringValue("PathCache"));
    internet.Install(adhocNodes);
    dsr ? dsrMain.Install(dsrH, adhocNodes) : wdsrMain.Install(wdsr, adhocNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer allInterfaces;
    allInterfaces = address.Assign(allDevices);

    uint16_t port = 9;
    double randomStartTime =
        (1 / ppers) / nSinks; // distributed btw 1s evenly as we are sending 4pkt/s
#if 0
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

        ApplicationContainer apps1 = onoff1.Install(adhocNodes.Get(i+ nWifis - nSinks));
        apps1.Start(Seconds(dataStart + i * randomStartTime));
        apps1.Stop(Seconds(dataTime + i * randomStartTime));
    }
#elif 0
        OnOffHelper onoff1("ns3::UdpSocketFactory",
                           Address());
        onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        onoff1.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff1.SetAttribute("DataRate", DataRateValue(DataRate(rate)));
    for (int i = 0; i < nSinks; i++)
    {
        Ptr<Socket> sink = SetupPacketReceive(allInterfaces.GetAddress(i), adhocNodes.Get(i));

        
        AddressValue remoteAddress(InetSocketAddress(allInterfaces.GetAddress(i), port));
        onoff1.SetAttribute("Remote", remoteAddress);

        Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
        ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + nSinks));
        temp.Start(Seconds(var->GetValue(0.0, 10.0)));
        temp.Stop(Seconds(TotalTime));
    }
#else
    double m_packetInterval = 0.1;
    uint16_t sinkNodeId = 4;
    ApplicationContainer serverApps;
    if (echo) {
        UdpEchoServerHelper echoServer(port);
        serverApps = echoServer.Install(adhocNodes.Get(sinkNodeId));
    } else {
        PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));
        serverApps = sink.Install(adhocNodes.Get(sinkNodeId));
    }
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(TotalTime));
    UdpEchoClientHelper echoClient(allInterfaces.GetAddress(sinkNodeId), port);
    echoClient.SetAttribute("MaxPackets",
                            UintegerValue((uint32_t)(TotalTime * (1 / m_packetInterval))));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(m_packetInterval)));
    echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));
    ApplicationContainer clientApps = echoClient.Install(adhocNodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(TotalTime));
#endif

    Simulator::Stop(Seconds(TotalTime));
    /******* Animation *******/
#if 1 //(dis/en)able animation
    AnimationInterface anim(dsr ? "dsr-sim.xml" : "wdsr-sim.xml"); // Mandatory
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
    for (int i = 0; i < 20; i++)
        fprintf(stderr, "%d:%u ",i, packets[i]); 
    fprintf(stderr, "\n");
    }
}
