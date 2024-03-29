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

#define NS_LOG_APPEND_CONTEXT                                                                      \
    if (GetObject<Node>())                                                                         \
    {                                                                                              \
        std::clog << "[node " << GetObject<Node>()->GetId() << "] ";                               \
    }

#include "wdsr-options.h"

#include "wdsr-option-header.h"
#include "wdsr-rcache.h"
#include "wdsr-test.h"

#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/object-vector.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-header.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"

#include <ctime>
#include <list>
#include <map>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("WDsrOptions");

namespace wdsr
{

NS_OBJECT_ENSURE_REGISTERED(WDsrOptions);

/*
* \brief The max capacity for all node batteries
*/

TypeId
WDsrOptions::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptions")
                            .SetParent<Object>()
                            .SetGroupName("WDsr")
                            .AddAttribute("OptionNumber",
                                          "The WDsr option number.",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&WDsrOptions::GetOptionNumber),
                                          MakeUintegerChecker<uint8_t>())
                            .AddTraceSource("Drop",
                                            "Packet dropped.",
                                            MakeTraceSourceAccessor(&WDsrOptions::m_dropTrace),
                                            "ns3::Packet::TracedCallback")
                            .AddTraceSource("Rx",
                                            "Receive WDSR packet.",
                                            MakeTraceSourceAccessor(&WDsrOptions::m_rxPacketTrace),
                                            "ns3::wdsr::WDsrOptionSRHeader::TracedCallback");
    return tid;
}

WDsrOptions::WDsrOptions()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptions::~WDsrOptions()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
WDsrOptions::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

Ptr<Node>
WDsrOptions::GetNode() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_node;
}

bool
WDsrOptions::ContainAddressAfter(Ipv4Address ipv4Address,
                                Ipv4Address destAddress,
                                std::vector<Ipv4Address>& nodeList)
{
    NS_LOG_FUNCTION(this << ipv4Address << destAddress);
    std::vector<Ipv4Address>::iterator it = find(nodeList.begin(), nodeList.end(), destAddress);

    for (std::vector<Ipv4Address>::iterator i = it; i != nodeList.end(); ++i)
    {
        if ((ipv4Address == (*i)) && ((*i) != nodeList.back()))
        {
            return true;
        }
    }
    return false;
}

std::vector<Ipv4Address>
WDsrOptions::CutRoute(Ipv4Address ipv4Address, std::vector<Ipv4Address>& nodeList)
{
    NS_LOG_FUNCTION(this << ipv4Address);
    std::vector<Ipv4Address>::iterator it = find(nodeList.begin(), nodeList.end(), ipv4Address);
    std::vector<Ipv4Address> cutRoute;
    for (std::vector<Ipv4Address>::iterator i = it; i != nodeList.end(); ++i)
    {
        cutRoute.push_back(*i);
    }
    return cutRoute;
}

Ptr<Ipv4Route>
WDsrOptions::SetRoute(Ipv4Address nextHop, Ipv4Address srcAddress)
{
    NS_LOG_FUNCTION(this << nextHop << srcAddress);
    m_ipv4Route = Create<Ipv4Route>();
    m_ipv4Route->SetDestination(nextHop);
    m_ipv4Route->SetGateway(nextHop);
    m_ipv4Route->SetSource(srcAddress);
    return m_ipv4Route;
}

bool
WDsrOptions::ReverseRoutes(std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this);
    std::vector<Ipv4Address> vec2(vec);
    vec.clear(); // To ensure vec is empty before start
    for (std::vector<Ipv4Address>::reverse_iterator ri = vec2.rbegin(); ri != vec2.rend(); ++ri)
    {
        vec.push_back(*ri);
    }

    if ((vec.size() == vec2.size()) && (vec.front() == vec2.back()))
    {
        return true;
    }
    return false;
}

Ipv4Address
WDsrOptions::SearchNextHop(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this << ipv4Address);
    Ipv4Address nextHop;
    NS_LOG_DEBUG("the vector size " << vec.size());
    if (vec.size() == 2)
    {
        NS_LOG_DEBUG("The two nodes are neighbors");
        nextHop = vec[1];
        return nextHop;
    }
    else
    {
        if (ipv4Address == vec.back())
        {
            NS_LOG_DEBUG("We have reached to the final destination " << ipv4Address << " "
                                                                     << vec.back());
            return ipv4Address;
        }
        for (std::vector<Ipv4Address>::const_iterator i = vec.begin(); i != vec.end(); ++i)
        {
            if (ipv4Address == (*i))
            {
                nextHop = *(++i);
                return nextHop;
            }
        }
    }
    NS_LOG_DEBUG("next hop address not found, route corrupted");
    Ipv4Address none = "0.0.0.0";
    return none;
}

Ipv4Address
WDsrOptions::ReverseSearchNextHop(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this << ipv4Address);
    Ipv4Address nextHop;
    if (vec.size() == 2)
    {
        NS_LOG_DEBUG("The two nodes are neighbors");
        nextHop = vec[0];
        return nextHop;
    }
    else
    {
        for (std::vector<Ipv4Address>::reverse_iterator ri = vec.rbegin(); ri != vec.rend(); ++ri)
        {
            if (ipv4Address == (*ri))
            {
                nextHop = *(++ri);
                return nextHop;
            }
        }
    }
    NS_LOG_DEBUG("next hop address not found, route corrupted");
    Ipv4Address none = "0.0.0.0";
    return none;
}

Ipv4Address
WDsrOptions::ReverseSearchNextTwoHop(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this << ipv4Address);
    Ipv4Address nextTwoHop;
    NS_LOG_DEBUG("The vector size " << vec.size());
    NS_ASSERT(vec.size() > 2);
    for (std::vector<Ipv4Address>::reverse_iterator ri = vec.rbegin(); ri != vec.rend(); ++ri)
    {
        if (ipv4Address == (*ri))
        {
            nextTwoHop = *(ri + 2);
            return nextTwoHop;
        }
    }
    NS_FATAL_ERROR("next hop address not found, route corrupted");
    Ipv4Address none = "0.0.0.0";
    return none;
}

void
WDsrOptions::PrintVector(std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this);
    /*
     * Check elements in a route vector
     */
    if (!vec.size())
    {
        NS_LOG_DEBUG("The vector is empty");
    }
    else
    {
        NS_LOG_DEBUG("Print all the elements in a vector");
        for (std::vector<Ipv4Address>::const_iterator i = vec.begin(); i != vec.end(); ++i)
        {
            NS_LOG_DEBUG("The ip address " << *i);
        }
    }
}

bool
WDsrOptions::IfDuplicates(std::vector<Ipv4Address>& vec, std::vector<Ipv4Address>& vec2)
{
    NS_LOG_FUNCTION(this);
    for (std::vector<Ipv4Address>::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
        for (std::vector<Ipv4Address>::const_iterator j = vec2.begin(); j != vec2.end(); ++j)
        {
            if ((*i) == (*j))
            {
                return true;
            }
            else
            {
                continue;
            }
        }
    }
    return false;
}

bool
WDsrOptions::CheckDuplicates(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this << ipv4Address);
    for (std::vector<Ipv4Address>::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
        if ((*i) == ipv4Address)
        {
            return true;
        }
        else
        {
            continue;
        }
    }
    return false;
}

void
WDsrOptions::RemoveDuplicates(std::vector<Ipv4Address>& vec)
{
    NS_LOG_FUNCTION(this);
    // Remove duplicate ip address from the route if any, should not happen with normal behavior
    // nodes
    std::vector<Ipv4Address> vec2(vec); // declare vec2 as a copy of the vec
    PrintVector(vec2);                  // Print all the ip address in the route
    vec.clear();                        // clear vec
    for (std::vector<Ipv4Address>::const_iterator i = vec2.begin(); i != vec2.end(); ++i)
    {
        if (vec.empty())
        {
            vec.push_back(*i);
            continue;
        }
        else
        {
            for (std::vector<Ipv4Address>::iterator j = vec.begin(); j != vec.end(); ++j)
            {
                if ((*i) == (*j))
                {
                    if ((j + 1) != vec.end())
                    {
                        vec.erase(j + 1, vec.end()); // Automatic shorten the route
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
                else if (j == (vec.end() - 1))
                {
                    vec.push_back(*i);
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
    }
}

uint32_t
WDsrOptions::GetIDfromIP(Ipv4Address address)
{
    NS_LOG_FUNCTION(this << address);
    int32_t nNodes = NodeList::GetNNodes();
    for (int32_t i = 0; i < nNodes; ++i)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (ipv4->GetAddress(1, 0).GetLocal() == address)
        {
            return i;
        }
    }
    return 255;
}

Ptr<Node>
WDsrOptions::GetNodeWithAddress(Ipv4Address ipv4Address)
{
    NS_LOG_FUNCTION(this << ipv4Address);
    int32_t nNodes = NodeList::GetNNodes();
    for (int32_t i = 0; i < nNodes; ++i)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        int32_t ifIndex = ipv4->GetInterfaceForAddress(ipv4Address);
        if (ifIndex != -1)
        {
            return node;
        }
    }
    return nullptr;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionPad1);

TypeId
WDsrOptionPad1::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionPad1")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionPad1>();
    return tid;
}

WDsrOptionPad1::WDsrOptionPad1()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionPad1::~WDsrOptionPad1()
{
    NS_LOG_FUNCTION_NOARGS();
}

uint8_t
WDsrOptionPad1::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();

    return OPT_NUMBER;
}

uint8_t
WDsrOptionPad1::Process(Ptr<Packet> packet,
                       Ptr<Packet> wdsrP,
                       Ipv4Address ipv4Address,
                       Ipv4Address source,
                       const Ipv4Header& ipv4Header,
                       uint8_t protocol,
                       bool& isPromisc,
                       Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    Ptr<Packet> p = packet->Copy();
    WDsrOptionPad1Header pad1Header;
    p->RemoveHeader(pad1Header);

    isPromisc = false;

    return pad1Header.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionPadn);

TypeId
WDsrOptionPadn::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionPadn")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionPadn>();
    return tid;
}

WDsrOptionPadn::WDsrOptionPadn()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionPadn::~WDsrOptionPadn()
{
    NS_LOG_FUNCTION_NOARGS();
}

uint8_t
WDsrOptionPadn::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();
    return OPT_NUMBER;
}

uint8_t
WDsrOptionPadn::Process(Ptr<Packet> packet,
                       Ptr<Packet> wdsrP,
                       Ipv4Address ipv4Address,
                       Ipv4Address source,
                       const Ipv4Header& ipv4Header,
                       uint8_t protocol,
                       bool& isPromisc,
                       Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);

    Ptr<Packet> p = packet->Copy();
    WDsrOptionPadnHeader padnHeader;
    p->RemoveHeader(padnHeader);

    isPromisc = false;

    return padnHeader.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRreq);

TypeId
WDsrOptionRreq::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRreq")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionRreq>();
    return tid;
}

TypeId
WDsrOptionRreq::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionRreq::WDsrOptionRreq()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionRreq::~WDsrOptionRreq()
{
    NS_LOG_FUNCTION_NOARGS();
}

uint8_t
WDsrOptionRreq::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();

    return OPT_NUMBER;
}

uint8_t
WDsrOptionRreq::Process(Ptr<Packet> packet,
                       Ptr<Packet> wdsrP,
                       Ipv4Address ipv4Address,
                       Ipv4Address source,
                       const Ipv4Header& ipv4Header,
                       uint8_t protocol,
                       bool& isPromisc,
                       Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    // Fields from IP header
    Time ActiveRouteTimeout = Seconds(α);
    NS_LOG_DEBUG("ActiveRouteTimeout result: "<<ActiveRouteTimeout);
    Ipv4Address srcAddress = ipv4Header.GetSource();
    /*
     * \ when the ip source address is equal to the address of our own, this is request packet
     * originated \ by the node itself, discard it
     */
    if (source == ipv4Address)
    {
        NS_LOG_DEBUG("Discard the packet since it was originated from same source address");
        m_dropTrace(packet); // call the drop trace to show in the tracing
        return 0;
    }
    /*
     * Get the node associated with the ipv4 address and get several objects from the node and leave
     * for further use
     */
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();

    Ptr<Packet> p =
        packet->Copy(); // Note: The packet here doesn't contain the fixed size wdsr header
    /*
     * \brief Get the number of routers' address field before removing the header
     * \peek the packet and get the value
     */
    uint8_t buf[2];
    p->CopyData(buf, sizeof(buf));
    uint8_t numberAddress = (buf[1] - 6) / 4;
    NS_LOG_DEBUG("The number of Ip addresses " << (uint32_t)numberAddress);
    if (numberAddress >= 255)
    {
        NS_LOG_DEBUG("Discard the packet, malformed header since two many ip addresses in route");
        m_dropTrace(packet); // call the drop trace to show in the tracing
        return 0;
    }

    /*
     * Create the wdsr rreq header
     */
    WDsrOptionRreqHeader rreq;
    /*
     * Set the number of addresses with the value from peek data and remove the rreq header
     */
    rreq.SetNumberAddress(numberAddress);
    // Remove the route request header
    NS_LOG_DEBUG("****************************************************************************");
    NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Deserialization of RREQ");                                
    p->RemoveHeader(rreq);  // space for deserialize header
    NS_LOG_DEBUG("****************************************************************************");
    // Verify the option length
    uint8_t length = rreq.GetLength();
    if (length % 2 != 0)
    {
        NS_LOG_LOGIC("Malformed header. Drop!");
        m_dropTrace(packet); // call drop trace
        return 0;
    }
    // Check the rreq id for verifying the request id
    uint16_t requestId = rreq.GetId();
    // The target address is where we want to send the data packets
    Ipv4Address targetAddress = rreq.GetTarget();
    // Get the node list and source address from the route request header
    std::vector<Ipv4Address> mainVector = rreq.GetNodesAddresses();
    std::vector<Ipv4Address> nodeList(mainVector);
    // Get the real source address of this request, it will be used when checking if we have
    // received the save route request before or not
    Ipv4Address sourceAddress = nodeList.front();
    PrintVector(nodeList);
    /*
     * Construct the wdsr routing header for later use
     */
    WDsrRoutingHeader wdsrRoutingHeader;
    wdsrRoutingHeader.SetNextHeader(protocol);
    wdsrRoutingHeader.SetMessageType(1);
    wdsrRoutingHeader.SetSourceId(GetIDfromIP(source));
    wdsrRoutingHeader.SetDestId(255);

    // check whether we have received this request or not, if not, it will save the request in the
    // table for later use, if not found, return false, and push the newly received source request
    // entry in the cache

    // Get the TTL value, this is used to test if the packet will be forwarded or not
    uint8_t ttl = ipv4Header.GetTtl();
    bool dupRequest = false; // initialize the duplicate request check value
    if (ttl)
    {
        NS_LOG_DEBUG("if ttl");
        // if the ttl value is not 0, then this request will be forwarded, then we need to
        // save it in the source entry
        dupRequest = wdsr->FindSourceEntry(sourceAddress, targetAddress, requestId);
    }
    /*
     * Before processing the route request, we need to check two things
     * 1. if this is the exact same request we have just received, ignore it
     * 2. if our address is already in the path list, ignore it
     * 3. otherwise process further
     */
    NS_LOG_DEBUG("dupRequest : "<<dupRequest<<" requestId :"<<requestId);
    /*if (dupRequest)
    {
        // We have received this same route request before, not forwarding it now
        NS_LOG_LOGIC("Duplicate request. Drop!");
        NS_LOG_DEBUG("Duplicate request. Drop!");
        m_dropTrace(packet); // call drop trace
        return 0;
    }*/

    if (CheckDuplicates(ipv4Address, nodeList))
    {
        /*
         * if the route contains the node address already, drop the request packet
         */
        m_dropTrace(packet); // call drop trace
        NS_LOG_DEBUG("Our node address is already seen in the route, drop the request");
        return 0;
    }
    else
    {
        NS_LOG_DEBUG("its not a duplicate:");
        // A node ignores all RREQs received from any node in its blacklist
        WDsrRouteCacheEntry toPrev;
        bool isRouteInCache = wdsr->LookupRoute(targetAddress, toPrev);
        WDsrRouteCacheEntry::IP_VECTOR ip =
            toPrev.GetVector(); // The route from our own route cache to dst
        if (source != ipv4Address){
            toPrev.SetLowestBat(rreq.GetLowestBat());    
        }
        toPrev.SetTxCost(rreq.GetTxCost());        
        PrintVector(ip);
        std::vector<Ipv4Address> saveRoute(nodeList);
        PrintVector(saveRoute);
        bool areThereDuplicates = IfDuplicates(ip, saveRoute);
        NS_LOG_DEBUG("Are there duplicates: "<<(bool) areThereDuplicates);
        /*
         *  When the reverse route is created or updated, the following actions on the route are
         * also carried out:
         *  3. the next hop in the routing table becomes the node from which the  RREQ was received
         *  4. the hop count is copied from the Hop Count in the RREQ message;
         */

        //  A node generates a RREP if either:
        //  (i)  it is itself the destination,
        /*
         * The target address equal to our own ip address
         */
        NS_LOG_DEBUG("The target address over here " << targetAddress << " and the ip address "
                                                     << ipv4Address << " and the source address "
                                                     << mainVector[0]);
        if (targetAddress == ipv4Address)
        {
            NS_LOG_DEBUG("We are at targetAddress");
            Ipv4Address nextHop; // Declare the next hop address to use
            if (nodeList.size() == 1)
            {
                NS_LOG_DEBUG("These two nodes are neighbors");
                m_finalRoute.clear();
                /// TODO has changed the srcAddress to source, should not matter either way, check
                /// later
                m_finalRoute.push_back(source);      // push back the request originator's address
                m_finalRoute.push_back(ipv4Address); // push back our own address
                nextHop = srcAddress;
                NS_LOG_DEBUG(">>0 nextHop " << nextHop);
            }
            else
            {
                NS_LOG_DEBUG("We are not at targetAddress");
                std::vector<Ipv4Address> changeRoute(nodeList);
                changeRoute.push_back(ipv4Address); // push back our own address
                m_finalRoute.clear();               // get a clear route vector
                for (std::vector<Ipv4Address>::iterator i = changeRoute.begin();
                     i != changeRoute.end();
                     ++i)
                {
                    m_finalRoute.push_back(*i); // Get the full route from source to destination
                }
                PrintVector(m_finalRoute);
                nextHop = ReverseSearchNextHop(ipv4Address, m_finalRoute); // get the next hop
                NS_LOG_DEBUG(">>1 nextHop " << nextHop);
            }

            WDsrOptionRrepHeader rrep;
            rrep.SetNodesAddress(m_finalRoute); // Set the node addresses in the route reply header
            NS_LOG_DEBUG("The nextHop address " << nextHop);
            Ipv4Address replyDst = m_finalRoute.front();
            /*
             * This part add wdsr header to the packet and send route reply packet
             */
            WDsrRoutingHeader wdsrRoutingHeader;
            wdsrRoutingHeader.SetNextHeader(protocol);
            wdsrRoutingHeader.SetMessageType(1);
            wdsrRoutingHeader.SetSourceId(GetIDfromIP(ipv4Address));
            wdsrRoutingHeader.SetDestId(GetIDfromIP(replyDst));
            // Set the route for route reply
            SetRoute(nextHop, ipv4Address);
            uint16_t hops = m_finalRoute.size();
            
            uint8_t length =
                rrep.GetLength(); // Get the length of the rrep header excluding the type header
            uint32_t nodeID =
                node->GetId();
            uint8_t txCost = 
                rrep.GetTxCost();
            uint8_t placeholder =
                1;  // ! Set this to whatever the txCost is

            wdsrRoutingHeader.SetPayloadLength(length + 2);

            NS_LOG_DEBUG("**************************************");
            NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\]");
            NS_LOG_FUNCTION(this<<" Calculating lowestBat:");
            rrep.SetLowestBat(rreq.GetLowestBat());
            
            NS_LOG_DEBUG("txCost before setting: "<<(int)txCost);
            rrep.SetTxCost(hops);
            NS_LOG_DEBUG("txCost after setting: "<<(int)rrep.GetTxCost());
            NS_LOG_DEBUG("**************************************");
            NS_LOG_DEBUG("****************************************************************************");
            NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Serialization of RREP");
            wdsrRoutingHeader.AddWDsrOption(rrep);
            NS_LOG_DEBUG("****************************************************************************");
            Ptr<Packet> newPacket = Create<Packet>();

            newPacket->AddHeader(wdsrRoutingHeader);
            wdsr->ScheduleInitialReply(newPacket, ipv4Address, nextHop, m_ipv4Route);
            /*
             * Create the route entry to the rreq originator and save it to route cache, also need
             * to reverse the route
             */
            PrintVector(m_finalRoute);
            if (ReverseRoutes(m_finalRoute))
            {
                NS_LOG_DEBUG("if ReverseRoutes(m_finalRoute)");
                PrintVector(m_finalRoute);
                Ipv4Address dst = m_finalRoute.back();
                bool addRoute = false;
                if (numberAddress > 0)
                {
                    WDsrRouteCacheEntry toSource(/*ip=*/m_finalRoute,
                                                /*dst=*/dst,
                                                /*exp=*/ActiveRouteTimeout,
                                                /*lowestBat=*/rrep.GetLowestBat(),
                                                /*txCost=*/ rrep.GetTxCost());
                    NS_LOG_DEBUG(">> 1");
                    if (wdsr->IsLinkCache())
                    {
                        NS_LOG_DEBUG("Added route link with lowestBat: "<<(int) rrep.GetLowestBat());
                        NS_LOG_DEBUG("Added route link with txCost: "<<(int) rrep.GetTxCost());          
                        addRoute = wdsr->AddRoute_Link(m_finalRoute, ipv4Address);
                    }
                    else
                    {
                        NS_LOG_DEBUG("ActiveRouteTimeout: "<<ActiveRouteTimeout);
                        NS_LOG_DEBUG("Added route with lowestBat: "<<(int) rrep.GetLowestBat());
                        NS_LOG_DEBUG("Added route with txCost: "<<(int) rrep.GetTxCost());                        
                        addRoute = wdsr->AddRoute(toSource);
                    }
                }
                else
                {
                    NS_LOG_DEBUG("Abnormal RouteRequest");
                    return 0;
                }

                if (addRoute)
                {
                    NS_LOG_DEBUG("If a route is added");
                    /*
                     * Found a route to the dst, construct the source route option header
                     */

                    WDsrOptionSRHeader sourceRoute;
                    NS_LOG_DEBUG("The route length " << m_finalRoute.size());
                    sourceRoute.SetNodesAddress(m_finalRoute);

                    /// TODO !!!!!!!!!!!!!!
                    /// Think about this part, we just added the route,
                    /// probability no need to increase stability now?????
                    // if (wdsr->IsLinkCache ())
                    //   {
                    //     wdsr->UseExtends (m_finalRoute);
                    //   }
                    sourceRoute.SetSegmentsLeft((m_finalRoute.size() - 2));
                    // The salvage value here is 0
                    sourceRoute.SetSalvage(0);
                    Ipv4Address nextHop =
                        SearchNextHop(ipv4Address, m_finalRoute); // Get the next hop address
                        NS_LOG_DEBUG(">>2 nextHop " << nextHop);
                    NS_LOG_DEBUG("The nextHop address " << nextHop);

                    if (nextHop == "0.0.0.0")
                    {
                        wdsr->PacketNewRoute(wdsrP, ipv4Address, dst, protocol);
                        return 0;
                    }
                    SetRoute(nextHop, ipv4Address);
                    /*
                     * Send the data packet from the send buffer
                     */
                    wdsr->SendPacketFromBuffer(sourceRoute, nextHop, protocol);
                    // Cancel the route request timer for destination after sending the data packet
                    wdsr->CancelRreqTimer(dst, true);
                }
                else
                {
                    NS_LOG_DEBUG("The route is failed to add in cache");
                    return 0;
                }
            }
            else
            {
                NS_LOG_DEBUG("Unable to reverse route");
                return 0;
            }
            isPromisc = false;
            return rreq.GetSerializedSize();
        }

        /*
         * (ii) or it has an active route to the destination, send reply based on request header and
         * route cache, need to delay based on a random value from d = H * (h - 1 + r), which can
         * avoid possible route reply storm. Also, verify if two vectors do not contain duplicates
         * (part of the route to the destination from route cache and route collected so far). If
         * so, do not use the route found and forward the route request.
         */
        else if (isRouteInCache && !areThereDuplicates)
        {
            NS_LOG_DEBUG("isRouteInCache && !areThereDuplicates");
            m_finalRoute.clear(); // Clear the final route vector
            /**
             * push back the intermediate node address from the source to this node
             */
            for (std::vector<Ipv4Address>::iterator i = saveRoute.begin(); i != saveRoute.end();
                 ++i)
            {
                m_finalRoute.push_back(*i);
            }
            /**
             * push back the route vector we found in our route cache to destination, including this
             * node's address
             */
            for (std::vector<Ipv4Address>::iterator j = ip.begin(); j != ip.end(); ++j)
            {
                m_finalRoute.push_back(*j);
            }
            /*
             * Create the route entry to the rreq originator and save it to route cache, also need
             * to reverse the route
             */
            bool addRoute = false;
            std::vector<Ipv4Address> reverseRoute(m_finalRoute);
            NS_LOG_DEBUG("Checking m_finalRoute, size is: "<<m_finalRoute.size());
            PrintVector(m_finalRoute);
            if (ReverseRoutes(reverseRoute))
            {
                NS_LOG_DEBUG("if (ReverseRoutes(reverseRoute))");
                saveRoute.push_back(ipv4Address);
                ReverseRoutes(saveRoute);
                Ipv4Address dst = saveRoute.back();
                NS_LOG_DEBUG("This is the route save in route cache");
                PrintVector(saveRoute);

                WDsrRouteCacheEntry toSource(/*ip=*/saveRoute,
                                            /*dst=*/dst,
                                            /*exp=*/ActiveRouteTimeout,
                                            /*lowestBat=*/rreq.GetLowestBat(),
                                            /*txCost=*/ rreq.GetTxCost());
                
                NS_ASSERT(saveRoute.front() == ipv4Address);
                // Add the route entry in the route cache
                NS_LOG_DEBUG(">> 2");
                if (wdsr->IsLinkCache())
                {
                    NS_LOG_DEBUG("Added route link with lowestBat: "<<(int) rreq.GetLowestBat());
                    NS_LOG_DEBUG("Added route link with txCost: "<<(int) rreq.GetTxCost());          
                    addRoute = wdsr->AddRoute_Link(saveRoute, ipv4Address);
                }
                else
                {
                    NS_LOG_DEBUG("ActiveRouteTimeout: "<<ActiveRouteTimeout);
                    NS_LOG_DEBUG("Added route with lowestBat: "<<(int) rreq.GetLowestBat());
                    NS_LOG_DEBUG("Added route with txCost: "<<(int) rreq.GetTxCost());          
                    
                    addRoute = wdsr->AddRoute(toSource);

                }

                if (addRoute)
                {
                    NS_LOG_DEBUG("If route is added:");
                    NS_LOG_LOGIC("We have added the route and search send buffer for packet with "
                                 "destination "
                                 << dst);
                     NS_LOG_DEBUG(
                                this<<" We have added the route and search send buffer for packet with destination "
                                << dst);
                    /*
                     * Found a route the dst, construct the source route option header
                     */
                    WDsrOptionSRHeader sourceRoute;
                    PrintVector(saveRoute);

                    sourceRoute.SetNodesAddress(saveRoute);
                    // if (wdsr->IsLinkCache ())
                    //   {
                    //     wdsr->UseExtends (saveRoute);
                    //   }
                    sourceRoute.SetSegmentsLeft((saveRoute.size() - 2));
                    uint8_t salvage = 0;
                    sourceRoute.SetSalvage(salvage);
                    Ipv4Address nextHop =
                        SearchNextHop(ipv4Address, saveRoute); // Get the next hop address
                    NS_LOG_DEBUG(">>3 nextHop " << nextHop);
                    NS_LOG_DEBUG("The nextHop address " << nextHop);

                    if (nextHop == "0.0.0.0")
                    {
                        wdsr->PacketNewRoute(wdsrP, ipv4Address, dst, protocol);
                        return 0;
                    }
                    SetRoute(nextHop, ipv4Address);
                    /*
                     * Schedule the packet retry
                     */
                    wdsr->SendPacketFromBuffer(sourceRoute, nextHop, protocol);
                    // Cancel the route request timer for destination
                    wdsr->CancelRreqTimer(dst, true);
                }
                else
                {
                    NS_LOG_DEBUG("The route is failed to add in cache");
                    return 0;
                }
            }
            else
            {
                NS_LOG_DEBUG("Unable to reverse the route");
                return 0;
            }

            /*
             * Need to first pin down the next hop address before removing duplicates
             */
            Ipv4Address nextHop = ReverseSearchNextHop(ipv4Address, m_finalRoute);
            NS_LOG_DEBUG(">>4 nextHop " << nextHop);
            /*
             * First remove the duplicate ip address to automatically shorten the route, and then
             * reversely search the next hop address
             */
            // Set the route
            SetRoute(nextHop, ipv4Address);

            uint16_t hops = m_finalRoute.size();
            WDsrOptionRrepHeader rrep;
            rrep.SetNodesAddress(m_finalRoute); // Set the node addresses in the route reply header
            // Get the real source of the reply
            Ipv4Address realSource = m_finalRoute.back();
            PrintVector(m_finalRoute);
            NS_LOG_DEBUG("This is the full route from " << realSource << " to "
                                                        << m_finalRoute.front());
            /*
             * This part add wdsr header to the packet and send route reply packet
             */
            WDsrRoutingHeader wdsrRoutingHeader;
            wdsrRoutingHeader.SetNextHeader(protocol);
            wdsrRoutingHeader.SetMessageType(1);
            wdsrRoutingHeader.SetSourceId(GetIDfromIP(realSource));
            wdsrRoutingHeader.SetDestId(GetIDfromIP(targetAddress));

            
            uint8_t length =
                rrep.GetLength(); // Get the length of the rrep header excluding the type header
            uint32_t nodeID =
                node->GetId();
            uint8_t txCost = 
                rrep.GetTxCost();
            uint8_t placeholder =
                1;  // ! Set this to whatever the txCost is

            wdsrRoutingHeader.SetPayloadLength(length + 2);

            NS_LOG_DEBUG("**************************************");
            NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\]");
            NS_LOG_FUNCTION(this<<" Calculating lowestBat:");
            rrep.SetLowestBat(rreq.GetLowestBat());
            NS_LOG_DEBUG("txCost before setting: "<<(int)txCost);
            rrep.SetTxCost(hops);
            NS_LOG_DEBUG("txCost after setting: "<<(int)rrep.GetTxCost());
            
            NS_LOG_DEBUG("**************************************");
            NS_LOG_DEBUG("****************************************************************************");
            NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Serialization of RREP");
            wdsrRoutingHeader.AddWDsrOption(rrep);
            NS_LOG_DEBUG("****************************************************************************");
            Ptr<Packet> newPacket = Create<Packet>();
            
            
            newPacket->AddHeader(wdsrRoutingHeader);
            wdsr->ScheduleCachedReply(newPacket, ipv4Address, nextHop, m_ipv4Route, hops);
            isPromisc = false;
            return rreq.GetSerializedSize();
        }
        /*
         * (iii) no route in any type has been found
         */
        else
        {
            NS_LOG_DEBUG("This is not the target");
            mainVector.push_back(ipv4Address);
            NS_ASSERT(mainVector.front() == source);
            NS_LOG_DEBUG("Print out the main vector");
            PrintVector(mainVector);
            rreq.SetNodesAddress(mainVector);

            Ptr<Packet> errP = p->Copy();
            if (errP->GetSize())
            {
                NS_LOG_DEBUG("Error header included");
                WDsrOptionRerrUnreachHeader rerr;
                p->RemoveHeader(rerr);
                Ipv4Address errorSrc = rerr.GetErrorSrc();
                Ipv4Address unreachNode = rerr.GetUnreachNode();
                Ipv4Address errorDst = rerr.GetErrorDst();

                if ((errorSrc == srcAddress) && (unreachNode == ipv4Address))
                {
                    NS_LOG_DEBUG("The error link back to work again");
    
            
                    uint8_t length =
                        rreq.GetLength(); // Get the length of the rreq header excluding the type header
                   
                    uint32_t nodeID =
                        node->GetId();
                    uint8_t txCost = 
                        rreq.GetTxCost();
                    uint8_t placeholder =
                        1;  // ! Set this to whatever the txCost is

                    wdsrRoutingHeader.SetPayloadLength(length + 2);

                    NS_LOG_DEBUG("**************************************");
                    NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\]");
                    NS_LOG_FUNCTION(this<<" Calculating lowestBat:");
                    if (source != ipv4Address){
                        rreq.CalcLowestBat(/*remainingEnergy=*/remainingEnergy[nodeID],
                                           /*initialEnergy=*/initialEnergy[nodeID]);
                    }
                    rreq.SetTxCost(txCost+placeholder);
                        
                    NS_LOG_DEBUG("**************************************");
                    NS_LOG_DEBUG("****************************************************************************");
                    NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Serialization of RREQ");
                    wdsrRoutingHeader.AddWDsrOption(rreq);
                    NS_LOG_DEBUG("****************************************************************************");
                    wdsrRoutingHeader.SetPayloadLength(length + 2);
                }
                else
                {
                    wdsr->DeleteAllRoutesIncludeLink(errorSrc, unreachNode, ipv4Address);

                    WDsrOptionRerrUnreachHeader newUnreach;
                    newUnreach.SetErrorType(1);
                    newUnreach.SetErrorSrc(errorSrc);
                    newUnreach.SetUnreachNode(unreachNode);
                    newUnreach.SetErrorDst(errorDst);
                    newUnreach.SetSalvage(rerr.GetSalvage()); // Set the value about whether to
                                                              // salvage a packet or not
    
            
                    uint8_t length =
                        rreq.GetLength(); // Get the length of the rreq header excluding the type header
                   
                    uint32_t nodeID =
                        node->GetId();
                    uint8_t txCost = 
                        rreq.GetTxCost();
                    uint8_t placeholder =
                        1;  // ! Set this to whatever the txCost is

                    wdsrRoutingHeader.SetPayloadLength(length + 2);

                    NS_LOG_DEBUG("**************************************");
                    NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\]");
                    NS_LOG_FUNCTION(this<<" Calculating lowestBat:");
                    if (source != ipv4Address){
                        rreq.CalcLowestBat(/*remainingEnergy=*/remainingEnergy[nodeID],
                                           /*initialEnergy=*/initialEnergy[nodeID]);
                    }
                    rreq.SetTxCost(txCost+placeholder);
                    
                    NS_LOG_DEBUG("**************************************");
                    NS_LOG_DEBUG("****************************************************************************");
                    NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Serialization of RREQ");
                    wdsrRoutingHeader.AddWDsrOption(rreq);
                    NS_LOG_DEBUG("****************************************************************************");

                    wdsrRoutingHeader.AddWDsrOption(newUnreach);
                }
            }
            else
            {

            
                uint8_t length =
                    rreq.GetLength(); // Get the length of the rreq header excluding the type header
                uint32_t nodeID =
                    node->GetId();
                uint8_t txCost = 
                    rreq.GetTxCost();
                uint8_t placeholder =
                    1;  // ! Set this to whatever the txCost is

                wdsrRoutingHeader.SetPayloadLength(length + 2);

                NS_LOG_DEBUG("**************************************");
                NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\]");
                NS_LOG_FUNCTION(this<<" Calculating lowestBat:");
                if (source != ipv4Address){
                    rreq.CalcLowestBat(/*remainingEnergy=*/remainingEnergy[nodeID],
                                       /*initialEnergy=*/initialEnergy[nodeID]);
                }
                rreq.SetTxCost(txCost+placeholder);
                NS_LOG_DEBUG("**************************************");
                NS_LOG_DEBUG("****************************************************************************");
                NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Serialization of RREQ");
                wdsrRoutingHeader.AddWDsrOption(rreq);
                NS_LOG_DEBUG("****************************************************************************");

                wdsrRoutingHeader.SetPayloadLength(length + 2);
            }
            // Get the TTL value
            uint8_t ttl = ipv4Header.GetTtl();
            /*
             * Decrease the TTL value in the packet tag by one, this tag will go to ip layer 3 send
             * function and drop packet when TTL value equals to 0
             */
            NS_LOG_DEBUG("The ttl value here " << (uint32_t)ttl);
            if (ttl)
            {
                Ptr<Packet> interP = Create<Packet>();
                SocketIpTtlTag tag;
                tag.SetTtl(ttl - 1);
                interP->AddPacketTag(tag);
                interP->AddHeader(wdsrRoutingHeader);
                wdsr->ScheduleInterRequest(interP);
                isPromisc = false;
            }
            return rreq.GetSerializedSize();
        }
    }
    // unreachable:  return rreq.GetSerializedSize ();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRrep);

TypeId
WDsrOptionRrep::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRrep")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionRrep>();
    return tid;
}

WDsrOptionRrep::WDsrOptionRrep()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionRrep::~WDsrOptionRrep()
{
    NS_LOG_FUNCTION_NOARGS();
}

TypeId
WDsrOptionRrep::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint8_t
WDsrOptionRrep::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();

    return OPT_NUMBER;
}

uint8_t
WDsrOptionRrep::Process(Ptr<Packet> packet,
                       Ptr<Packet> wdsrP,
                       Ipv4Address ipv4Address,
                       Ipv4Address source,
                       const Ipv4Header& ipv4Header,
                       uint8_t protocol,
                       bool& isPromisc,
                       Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    Time ActiveRouteTimeout = Seconds(α);
    Ptr<Packet> p = packet->Copy();

    // Get the number of routers' address field
    uint8_t buf[2];
    p->CopyData(buf, sizeof(buf));
    uint8_t numberAddress = (buf[1] - 2) / 4;

    WDsrOptionRrepHeader rrep;
    rrep.SetNumberAddress(numberAddress); // Set the number of ip address in the header to reserver
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    NS_LOG_DEBUG("****************************************************************************");
    NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Deserialization of RREP");                                
    p->RemoveHeader(rrep);  // space for deserialize header
    NS_LOG_DEBUG("****************************************************************************");
    
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();
    
    NS_LOG_DEBUG("The next header value " << (uint32_t)protocol);

    std::vector<Ipv4Address> nodeList = rrep.GetNodesAddress();
    /**
     * Get the destination address, which is the last element in the nodeList
     */
    Ipv4Address targetAddress = nodeList.front();
    // If the RREP option has reached to the destination
    if (targetAddress == ipv4Address)
    {
        RemoveDuplicates(nodeList); // This is for the route reply from intermediate node since we
                                    // didn't remove duplicate there
        if (nodeList.size() == 0)
        {
            NS_LOG_DEBUG("The route we have contains 0 entries");
            return 0;
        }
        /**
         * Get the destination address for the data packet, which is the last element in the
         * nodeList
         */
        Ipv4Address dst = nodeList.back();
        /**
         * Add the newly found route to the route cache
         * The route looks like:
         * \\ "srcAddress" + "intermediate node address" + "targetAddress"
         */
        WDsrRouteCacheEntry toDestination(/*ip=*/nodeList,
                                         /*dst=*/dst,
                                         /*exp=*/ActiveRouteTimeout,
                                        /*lowestBat=*/rrep.GetLowestBat(),
                                        /*txCost=*/ rrep.GetTxCost());
        NS_ASSERT(nodeList.front() == ipv4Address);
        bool addRoute = false;
        NS_LOG_DEBUG(">> 3");
        if (wdsr->IsLinkCache())
        {
            NS_LOG_DEBUG("__Is link cache:");
            NS_LOG_DEBUG("Added route link with lowestBat: "<<(int) rrep.GetLowestBat());
            NS_LOG_DEBUG("Added route link with txCost: "<<(int) rrep.GetTxCost());
            NS_LOG_DEBUG("Added route link with ActiveRouteTimeout: "<<ActiveRouteTimeout);         
            addRoute = wdsr->AddRoute_Link(nodeList, ipv4Address);
        }
        else
        {
            NS_LOG_DEBUG("__Is path rout:");
            NS_LOG_DEBUG("Added route with lowestBat: "<<(int) rrep.GetLowestBat());
            NS_LOG_DEBUG("Added route with txCost: "<<(int) rrep.GetTxCost());
            NS_LOG_DEBUG("Added route with ActiveRouteTimeout: "<<ActiveRouteTimeout);
            addRoute = wdsr->AddRoute(toDestination);
        }

        if (addRoute)
        {
            NS_LOG_DEBUG(
                "We have added the route and search send buffer for packet with destination "
                << dst);
            /**
             * Found a route the dst, construct the source route option header
             */
            WDsrOptionSRHeader sourceRoute;
            NS_LOG_DEBUG("The route length " << nodeList.size());
            sourceRoute.SetNodesAddress(nodeList);
            sourceRoute.SetSegmentsLeft((nodeList.size() - 2));
            sourceRoute.SetSalvage(0);
            Ipv4Address nextHop = SearchNextHop(ipv4Address, nodeList); // Get the next hop address
            NS_LOG_DEBUG(">>5 nextHop " << nextHop);
            NS_LOG_DEBUG("The nextHop address " << nextHop);
            if (nextHop == "0.0.0.0")
            {
                wdsr->PacketNewRoute(wdsrP, ipv4Address, dst, protocol);
                return 0;
            }
            PrintVector(nodeList);
            SetRoute(nextHop, ipv4Address);
            // Cancel the route request timer for destination
            wdsr->CancelRreqTimer(dst, true);
            /**
             * Schedule the packet retry
             */
            wdsr->SendPacketFromBuffer(sourceRoute, nextHop, protocol);
        }
        else
        {
            NS_LOG_DEBUG("Failed to add the route");
            return 0;
        }
    }
    else
    {
        uint8_t length = rrep.GetLength() -
                         2; // The get length - 2 is to get aligned for the malformed header check
        NS_LOG_DEBUG("The length of rrep option " << (uint32_t)length);

        if (length % 2 != 0)
        {
            NS_LOG_LOGIC("Malformed header. Drop!");
            m_dropTrace(packet);
            return 0;
        }
        PrintVector(nodeList);
        /*
         * This node is only an intermediate node, but it needs to save the possible route to the
         * destination when cutting the route
         */
        std::vector<Ipv4Address> routeCopy = nodeList;
        std::vector<Ipv4Address> cutRoute = CutRoute(ipv4Address, nodeList);
        PrintVector(cutRoute);
        if (cutRoute.size() >= 2)
        {
            Ipv4Address dst = cutRoute.back();
            NS_LOG_DEBUG("The route destination after cut " << dst);
            WDsrRouteCacheEntry toDestination(/*ip=*/cutRoute,
                                             /*dst=*/dst,
                                             /*exp=*/ActiveRouteTimeout,
                                            /*lowestBat=*/rrep.GetLowestBat(),
                                            /*txCost=*/ rrep.GetTxCost());
            NS_ASSERT(cutRoute.front() == ipv4Address);
            bool addRoute = false;
            NS_LOG_DEBUG(">> 4");
            if (wdsr->IsLinkCache())
            {
                NS_LOG_DEBUG("Added route link with lowestBat: "<<(int) rrep.GetLowestBat());
                NS_LOG_DEBUG("Added route link with txCost: "<<(int) rrep.GetTxCost());          
                addRoute = wdsr->AddRoute_Link(nodeList, ipv4Address);
            }
            else
            {
                NS_LOG_DEBUG("ActiveRouteTimeout: "<<ActiveRouteTimeout);
                NS_LOG_DEBUG("Added route with lowestBat: "<<(int) rrep.GetLowestBat());
                NS_LOG_DEBUG("Added route with txCost: "<<(int) rrep.GetTxCost());          
                addRoute = wdsr->AddRoute(toDestination);
            }
            if (addRoute)
            {
                wdsr->CancelRreqTimer(dst, true);
            }
            else
            {
                NS_LOG_DEBUG("The route not added");
            }
        }
        else
        {
            NS_LOG_DEBUG("The route is corrupted");
        }
        /*
         * Reverse search the vector for next hop address
         */
        Ipv4Address nextHop = ReverseSearchNextHop(ipv4Address, routeCopy);
        NS_LOG_DEBUG(">>6 nextHop " << nextHop);
        NS_ASSERT(routeCopy.back() == source);
        PrintVector(routeCopy);
        NS_LOG_DEBUG("The nextHop address " << nextHop << " and the source in the route reply "
                                            << source);
        /*
         * Set the route entry we will use to send reply
         */
        SetRoute(nextHop, ipv4Address);
        /*
         * This part add wdsr routing header to the packet and send reply
         */
        WDsrRoutingHeader wdsrRoutingHeader;
        wdsrRoutingHeader.SetNextHeader(protocol);

        length = rrep.GetLength(); // Get the length of the rrep header excluding the type header
        NS_LOG_DEBUG("The reply header length " << (uint32_t)length);
        wdsrRoutingHeader.SetPayloadLength(length + 2);
        wdsrRoutingHeader.SetMessageType(1);
        wdsrRoutingHeader.SetSourceId(GetIDfromIP(source));
        wdsrRoutingHeader.SetDestId(GetIDfromIP(targetAddress));
        NS_LOG_DEBUG("****************************************************************************");
        NS_LOG_DEBUG("\[Node "<<node->GetId()<<"\] Serialization of RREP");
        wdsrRoutingHeader.AddWDsrOption(rrep);
        NS_LOG_DEBUG("****************************************************************************");

        Ptr<Packet> newPacket = Create<Packet>();
        newPacket->AddHeader(wdsrRoutingHeader);
        wdsr->SendReply(newPacket, ipv4Address, nextHop, m_ipv4Route);
        isPromisc = false;
    }
    return rrep.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionSR);

TypeId
WDsrOptionSR::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionSR")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionSR>();
    return tid;
}

WDsrOptionSR::WDsrOptionSR()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionSR::~WDsrOptionSR()
{
    NS_LOG_FUNCTION_NOARGS();
}

TypeId
WDsrOptionSR::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint8_t
WDsrOptionSR::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();
    return OPT_NUMBER;
}

uint8_t
WDsrOptionSR::Process(Ptr<Packet> packet,
                     Ptr<Packet> wdsrP,
                     Ipv4Address ipv4Address,
                     Ipv4Address source,
                     const Ipv4Header& ipv4Header,
                     uint8_t protocol,
                     bool& isPromisc,
                     Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Address << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    Ptr<Packet> p = packet->Copy();
    // Get the number of routers' address field
    uint8_t buf[2];
    p->CopyData(buf, sizeof(buf));
    uint8_t numberAddress = (buf[1] - 2) / 4;
    WDsrOptionSRHeader sourceRoute;
    sourceRoute.SetNumberAddress(numberAddress);
    p->RemoveHeader(sourceRoute);

    // The route size saved in the source route
    std::vector<Ipv4Address> nodeList = sourceRoute.GetNodesAddress();
    uint8_t segsLeft = sourceRoute.GetSegmentsLeft();
    uint8_t salvage = sourceRoute.GetSalvage();
    /*
     * Get the node from IP address and get the WDSR extension object
     */
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();
    /*
     * Get the source and destination address from ipv4 header
     */
    Ipv4Address srcAddress = ipv4Header.GetSource();
    Ipv4Address destAddress = ipv4Header.GetDestination();

    // Get the node list destination
    Ipv4Address destination = nodeList.back();
    /*
     * If it's a promiscuous receive data packet,
     * 1. see if automatic route shortening possible or not
     * 2. see if it is a passive acknowledgment
     */
    if (isPromisc)
    {
        NS_LOG_LOGIC("We process promiscuous receipt data packet");
        if (ContainAddressAfter(ipv4Address, destAddress, nodeList))
        {
            NS_LOG_LOGIC("Send back the gratuitous reply");
            wdsr->SendGratuitousReply(source, srcAddress, nodeList, protocol);
        }

        uint16_t fragmentOffset = ipv4Header.GetFragmentOffset();
        uint16_t identification = ipv4Header.GetIdentification();

        if (destAddress != destination)
        {
            NS_LOG_DEBUG("Process the promiscuously received packet");
            bool findPassive = false;
            int32_t nNodes = NodeList::GetNNodes();
            for (int32_t i = 0; i < nNodes; ++i)
            {
                NS_LOG_DEBUG("Working with node " << i);

                Ptr<Node> node = NodeList::GetNode(i);
                Ptr<wdsr::WDsrRouting> wdsrNode = node->GetObject<wdsr::WDsrRouting>();
                // The source and destination addresses here are the real source and destination for
                // the packet
                findPassive = wdsrNode->PassiveEntryCheck(packet,
                                                         source,
                                                         destination,
                                                         segsLeft,
                                                         fragmentOffset,
                                                         identification,
                                                         false);
                if (findPassive)
                {
                    break;
                }
            }

            if (findPassive)
            {
                NS_LOG_DEBUG("We find one previously received passive entry");
                /*
                 * Get the node from IP address and get the WDSR extension object
                 * the srcAddress would be the source address from ip header
                 */
                PrintVector(nodeList);

                NS_LOG_DEBUG("promisc source " << promiscSource);
                Ptr<Node> node = GetNodeWithAddress(promiscSource);
                Ptr<wdsr::WDsrRouting> wdsrSrc = node->GetObject<wdsr::WDsrRouting>();
                wdsrSrc->CancelPassiveTimer(packet, source, destination, segsLeft);
            }
            else
            {
                NS_LOG_DEBUG("Saved the entry for further use");
                wdsr->PassiveEntryCheck(packet,
                                       source,
                                       destination,
                                       segsLeft,
                                       fragmentOffset,
                                       identification,
                                       true);
            }
        }
        /// Safely terminate promiscuously received packet
        return 0;
    }
    else
    {
        /*
         * Get the number of address from the source route header
         */
        uint8_t length = sourceRoute.GetLength();
        uint8_t nextAddressIndex;
        Ipv4Address nextAddress;

        // Get the option type value
        uint32_t size = p->GetSize();
        uint8_t* data = new uint8_t[size];
        p->CopyData(data, size);
        uint8_t optionType = 0;
        optionType = *(data);
        /// When the option type is 160, means there is ACK request header after the source route,
        /// we need to send back acknowledgment
        if (optionType == 160)
        {
            NS_LOG_LOGIC("Remove the ack request header and add ack header to the packet");
            // Here we remove the ack packet to the previous hop
            WDsrOptionAckReqHeader ackReq;
            p->RemoveHeader(ackReq);
            uint16_t ackId = ackReq.GetAckId();
            /*
             * Send back acknowledgment packet to the earlier hop
             * If the node list is not empty, find the previous hop from the node list,
             * otherwise, use srcAddress
             */
            Ipv4Address ackAddress = srcAddress;
            if (!nodeList.empty())
            {
                if (segsLeft > numberAddress) // The segmentsLeft field should not be larger than
                                              // the total number of ip addresses
                {
                    NS_LOG_LOGIC("Malformed header. Drop!");
                    m_dropTrace(packet);
                    return 0;
                }
                // -fstrict-overflow sensitive, see bug 1868
                if (numberAddress - segsLeft < 2) // The index is invalid
                {
                    NS_LOG_LOGIC("Malformed header. Drop!");
                    m_dropTrace(packet);
                    return 0;
                }
                ackAddress = nodeList[numberAddress - segsLeft - 2];
            }
            m_ipv4Route = SetRoute(ackAddress, ipv4Address);
            NS_LOG_DEBUG("Send back ACK to the earlier hop " << ackAddress << " from us "
                                                             << ipv4Address);
            wdsr->SendAck(ackId, ackAddress, source, destination, protocol, m_ipv4Route);
        }
        /*
         * After send back ACK, check if the segments left value has turned to 0 or not, if yes,
         * update the route entry and return header length
         */
        if (segsLeft == 0)
        {
            NS_LOG_DEBUG("This is the final destination");
            isPromisc = false;
            return sourceRoute.GetSerializedSize();
        }

        if (length % 2 != 0)
        {
            NS_LOG_LOGIC("Malformed header. Drop!");
            m_dropTrace(packet);
            return 0;
        }

        if (segsLeft > numberAddress) // The segmentsLeft field should not be larger than the total
                                      // number of ip addresses
        {
            NS_LOG_LOGIC("Malformed header. Drop!");
            m_dropTrace(packet);
            return 0;
        }

        WDsrOptionSRHeader newSourceRoute;
        newSourceRoute.SetSegmentsLeft(segsLeft - 1);
        newSourceRoute.SetSalvage(salvage);
        newSourceRoute.SetNodesAddress(nodeList);
        nextAddressIndex = numberAddress - segsLeft;
        nextAddress = newSourceRoute.GetNodeAddress(nextAddressIndex);
        NS_LOG_DEBUG("The next address of source route option "
                     << nextAddress << " and the nextAddressIndex: " << (uint32_t)nextAddressIndex
                     << " and the segments left : " << (uint32_t)segsLeft);
        /*
         * Get the target Address in the node list
         */
        Ipv4Address targetAddress = nodeList.back();
        Ipv4Address realSource = nodeList.front();
        /*
         * Search the vector for next hop address
         */
        Ipv4Address nextHop = SearchNextHop(ipv4Address, nodeList);
        NS_LOG_DEBUG(">>7 nextHop " << nextHop);
        PrintVector(nodeList);

        if (nextHop == "0.0.0.0")
        {
            NS_LOG_DEBUG("Before new packet " << *wdsrP);
            wdsr->PacketNewRoute(wdsrP, realSource, targetAddress, protocol);
            return 0;
        }

        if (ipv4Address == nextHop)
        {
            NS_LOG_DEBUG("We have reached the destination");
            newSourceRoute.SetSegmentsLeft(0);
            return newSourceRoute.GetSerializedSize();
        }
        // Verify the multicast address, leave it here for now
        if (nextAddress.IsMulticast() || destAddress.IsMulticast())
        {
            m_dropTrace(packet);
            return 0;
        }
        // Set the route and forward the data packet
        SetRoute(nextAddress, ipv4Address);
        NS_LOG_DEBUG("wdsr packet size " << wdsrP->GetSize());
        wdsr->ForwardPacket(wdsrP,
                           newSourceRoute,
                           ipv4Header,
                           realSource,
                           nextAddress,
                           targetAddress,
                           protocol,
                           m_ipv4Route);
    }
    return sourceRoute.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRerr);

TypeId
WDsrOptionRerr::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRerr")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionRerr>();
    return tid;
}

WDsrOptionRerr::WDsrOptionRerr()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionRerr::~WDsrOptionRerr()
{
    NS_LOG_FUNCTION_NOARGS();
}

TypeId
WDsrOptionRerr::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint8_t
WDsrOptionRerr::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();
    return OPT_NUMBER;
}

uint8_t
WDsrOptionRerr::Process(Ptr<Packet> packet,
                       Ptr<Packet> wdsrP,
                       Ipv4Address ipv4Address,
                       Ipv4Address source,
                       const Ipv4Header& ipv4Header,
                       uint8_t protocol,
                       bool& isPromisc,
                       Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    Ptr<Packet> p = packet->Copy();
    uint32_t size = p->GetSize();
    uint8_t* data = new uint8_t[size];
    p->CopyData(data, size);
    uint8_t errorType = *(data + 2);
    /*
     * Get the node from Ip address and get the wdsr extension object
     */
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();
    /*
     * The error serialized size
     */
    [[maybe_unused]] uint32_t rerrSize;
    NS_LOG_DEBUG("The error type value here " << (uint32_t)errorType);
    if (errorType == 1) // unreachable ip address
    {
        /*
         * Remove the route error header from the packet, and get the error type
         */
        WDsrOptionRerrUnreachHeader rerrUnreach;
        p->RemoveHeader(rerrUnreach);
        /*
         * Get the error destination address
         */
        Ipv4Address unreachAddress = rerrUnreach.GetUnreachNode();
        Ipv4Address errorSource = rerrUnreach.GetErrorSrc();

        NS_LOG_DEBUG("The error source is " << rerrUnreach.GetErrorDst()
                                            << "and the unreachable node is " << unreachAddress);
        /*
         * Get the serialized size of the rerr header
         */
        rerrSize = rerrUnreach.GetSerializedSize();
        /*
         * Delete all the routes including the unreachable node address from the route cache
         */
        Ptr<Node> node = GetNodeWithAddress(ipv4Address);
        wdsr->DeleteAllRoutesIncludeLink(errorSource, unreachAddress, ipv4Address);

        Ptr<Packet> newP = p->Copy();
        uint32_t serialized = DoSendError(newP, rerrUnreach, rerrSize, ipv4Address, protocol);
        return serialized;
    }
    else
    {
        /*
         * Two other type of error headers:
         * 1. flow state not supported type-specific information
         * 2. unsupported option with option number
         */
        /*
         * Remove the route error header from the packet, and get the error type
         */
        WDsrOptionRerrUnsupportHeader rerrUnsupport;
        p->RemoveHeader(rerrUnsupport);
        rerrSize = rerrUnsupport.GetSerializedSize();

        /// \todo This is for the other two error options, not supporting for now
        // uint32_t serialized = DoSendError (p, rerrUnsupport, rerrSize, ipv4Address, protocol);
        uint32_t serialized = 0;
        return serialized;
    }
}

uint8_t
WDsrOptionRerr::DoSendError(Ptr<Packet> p,
                           WDsrOptionRerrUnreachHeader& rerr,
                           uint32_t rerrSize,
                           Ipv4Address ipv4Address,
                           uint8_t protocol)
{
    // Get the number of routers' address field
    uint8_t buf[2];
    p->CopyData(buf, sizeof(buf));
    uint8_t numberAddress = (buf[1] - 2) / 4;

    // Here remove the source route header and schedule next hop error transmission
    NS_LOG_DEBUG("The number of addresses " << (uint32_t)numberAddress);
    WDsrOptionSRHeader sourceRoute;
    sourceRoute.SetNumberAddress(numberAddress);
    p->RemoveHeader(sourceRoute);
    NS_ASSERT(p->GetSize() == 0);
    /*
     * Get the node from ip address and the wdsr extension object
     */
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();
    /*
     * Get the segments left field and the next address
     */
    uint8_t segmentsLeft = sourceRoute.GetSegmentsLeft();
    uint8_t length = sourceRoute.GetLength();
    uint8_t nextAddressIndex;
    Ipv4Address nextAddress;
    /*
     * Get the route size and the error target address
     */
    std::vector<Ipv4Address> nodeList = sourceRoute.GetNodesAddress();
    Ipv4Address targetAddress = nodeList.back();
    /*
     * The total serialized size for both the rerr and source route headers
     */
    uint32_t serializedSize = rerrSize + sourceRoute.GetSerializedSize();

    if (length % 2 != 0)
    {
        NS_LOG_LOGIC("Malformed header. Drop!");
        m_dropTrace(p);
        return 0;
    }

    if (segmentsLeft > numberAddress)
    {
        NS_LOG_LOGIC("Malformed header. Drop!");
        m_dropTrace(p);
        return 0;
    }
    /*
     * When the error packet has reached to the destination
     */
    if (segmentsLeft == 0 && targetAddress == ipv4Address)
    {
        NS_LOG_INFO("This is the destination of the error, send error request");
        wdsr->SendErrorRequest(rerr, protocol);
        return serializedSize;
    }

    // Get the next Router Address
    WDsrOptionSRHeader newSourceRoute;
    newSourceRoute.SetSegmentsLeft(segmentsLeft - 1);
    nextAddressIndex = numberAddress - segmentsLeft;
    nextAddress = sourceRoute.GetNodeAddress(nextAddressIndex);
    newSourceRoute.SetSalvage(sourceRoute.GetSalvage());
    newSourceRoute.SetNodesAddress(nodeList);
    nextAddress = newSourceRoute.GetNodeAddress(nextAddressIndex);

    /// to test if the next address is multicast or not
    if (nextAddress.IsMulticast() || targetAddress.IsMulticast())
    {
        m_dropTrace(p);
        return serializedSize;
    }

    // Set the route entry
    SetRoute(nextAddress, ipv4Address);
    wdsr->ForwardErrPacket(rerr, newSourceRoute, nextAddress, protocol, m_ipv4Route);
    return serializedSize;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionAckReq);

TypeId
WDsrOptionAckReq::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionAckReq")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionAckReq>();
    return tid;
}

WDsrOptionAckReq::WDsrOptionAckReq()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionAckReq::~WDsrOptionAckReq()
{
    NS_LOG_FUNCTION_NOARGS();
}

TypeId
WDsrOptionAckReq::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint8_t
WDsrOptionAckReq::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();
    return OPT_NUMBER;
}

uint8_t
WDsrOptionAckReq::Process(Ptr<Packet> packet,
                         Ptr<Packet> wdsrP,
                         Ipv4Address ipv4Address,
                         Ipv4Address source,
                         const Ipv4Header& ipv4Header,
                         uint8_t protocol,
                         bool& isPromisc,
                         Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    /*
     * Current implementation of the ack request header processing is coded in source route header
     * processing
     */
    /*
     * Remove the ack request header
     */
    Ptr<Packet> p = packet->Copy();
    WDsrOptionAckReqHeader ackReq;
    p->RemoveHeader(ackReq);
    /*
     * Get the node with ip address and get the wdsr extension and reoute cache objects
     */
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();

    NS_LOG_DEBUG("The next header value " << (uint32_t)protocol);

    return ackReq.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionAck);

TypeId
WDsrOptionAck::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionAck")
                            .SetParent<WDsrOptions>()
                            .SetGroupName("WDsr")
                            .AddConstructor<WDsrOptionAck>();
    return tid;
}

WDsrOptionAck::WDsrOptionAck()
{
    NS_LOG_FUNCTION_NOARGS();
}

WDsrOptionAck::~WDsrOptionAck()
{
    NS_LOG_FUNCTION_NOARGS();
}

TypeId
WDsrOptionAck::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint8_t
WDsrOptionAck::GetOptionNumber() const
{
    NS_LOG_FUNCTION_NOARGS();
    return OPT_NUMBER;
}

uint8_t
WDsrOptionAck::Process(Ptr<Packet> packet,
                      Ptr<Packet> wdsrP,
                      Ipv4Address ipv4Address,
                      Ipv4Address source,
                      const Ipv4Header& ipv4Header,
                      uint8_t protocol,
                      bool& isPromisc,
                      Ipv4Address promiscSource)
{
    NS_LOG_FUNCTION(this << packet << wdsrP << ipv4Address << source << ipv4Header
                         << (uint32_t)protocol << isPromisc);
    /*
     * Remove the ACK header
     */
    Ptr<Packet> p = packet->Copy();
    WDsrOptionAckHeader ack;
    p->RemoveHeader(ack);
    /*
     * Get the ACK source and destination address
     */
    Ipv4Address realSrc = ack.GetRealSrc();
    Ipv4Address realDst = ack.GetRealDst();
    uint16_t ackId = ack.GetAckId();
    /*
     * Get the node with ip address and get the wdsr extension and route cache objects
     */
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    Ptr<wdsr::WDsrRouting> wdsr = node->GetObject<wdsr::WDsrRouting>();
    wdsr->UpdateRouteEntry(realDst);
    /*
     * Cancel the packet retransmit timer when receiving the ack packet
     */
    wdsr->CallCancelPacketTimer(ackId, ipv4Header, realSrc, realDst);
    return ack.GetSerializedSize();
}

} // namespace wdsr
} // namespace ns3
