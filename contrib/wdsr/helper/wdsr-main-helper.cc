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

#include "wdsr-main-helper.h"

#include "ns3/wdsr-helper.h"
#include "ns3/wdsr-rcache.h"
#include "ns3/wdsr-routing.h"
#include "ns3/wdsr-rreq-table.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/ptr.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WDsrMainHelper");

WDsrMainHelper::WDsrMainHelper()
    : m_wdsrHelper(nullptr)
{
    NS_LOG_FUNCTION(this);
}

WDsrMainHelper::WDsrMainHelper(const WDsrMainHelper& o)
{
    NS_LOG_FUNCTION(this);
    m_wdsrHelper = o.m_wdsrHelper->Copy();
}

WDsrMainHelper::~WDsrMainHelper()
{
    NS_LOG_FUNCTION(this);
    delete m_wdsrHelper;
}

WDsrMainHelper&
WDsrMainHelper::operator=(const WDsrMainHelper& o)
{
    if (this == &o)
    {
        return *this;
    }
    m_wdsrHelper = o.m_wdsrHelper->Copy();
    return *this;
}

void
WDsrMainHelper::Install(WDsrHelper& wdsrHelper, NodeContainer nodes)
{
    NS_LOG_DEBUG("Passed node container");
    delete m_wdsrHelper;
    m_wdsrHelper = wdsrHelper.Copy();
    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
    {
        Install(*i);
    }
}

void
WDsrMainHelper::Install(Ptr<Node> node)
{
    NS_LOG_FUNCTION(node);
    Ptr<ns3::wdsr::WDsrRouting> wdsr = m_wdsrHelper->Create(node);
    //  Ptr<ns3::wdsr::RouteCache> routeCache = CreateObject<ns3::wdsr::RouteCache> ();
    //  Ptr<ns3::wdsr::RreqTable> rreqTable = CreateObject<ns3::wdsr::RreqTable> ();
    //  wdsr->SetRouteCache (routeCache);
    //  wdsr->SetRequestTable (rreqTable);
    wdsr->SetNode(node);
    //  node->AggregateObject (routeCache);
    //  node->AggregateObject (rreqTable);
}

void
WDsrMainHelper::SetWDsrHelper(WDsrHelper& wdsrHelper)
{
    NS_LOG_FUNCTION(this);
    delete m_wdsrHelper;
    m_wdsrHelper = wdsrHelper.Copy();
}

} // namespace ns3
