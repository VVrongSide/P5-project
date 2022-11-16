/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/netanim-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Random Placement");

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("Random placement");
    
    uint32_t APs = 5;
    NodeContainer apDevices;
    apDevices.Create(APs);

    //CSMA
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    csma.Install(apDevices);

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(apDevices);

    InternetStackHelper stack;
    stack.Install(apDevices);

    //Placement
    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(1);
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator", 
                                    "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=1000]"), 
                                    "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=1000]"), 
                                    "Z", StringValue("ns3::UniformRandomVariable[Min=0|Max=0]"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apDevices);

    Ipv4AddressHelper address;
    address.SetBase("10.0.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign(csmaDevices);

    //Animation
    AnimationInterface anim("scratch-animation.xml");
    
    for (uint32_t i = 0; i < apDevices.GetN(); ++i) {
        anim.UpdateNodeDescription(apDevices.Get(i), "AP"); // Optional
        anim.UpdateNodeColor(apDevices.Get(i), 0, 255, 0);  // Optional
    }
    //Simulator::Stop(Seconds(2.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
