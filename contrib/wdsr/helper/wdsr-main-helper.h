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

#ifndef WDSR_MAIN_HELPER_H
#define WDSR_MAIN_HELPER_H

#include "ns3/wdsr-helper.h"
#include "ns3/wdsr-routing.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

namespace ns3
{

/**
 * \ingroup wdsr
 *
 * \brief Helper class that adds WDSR routing to nodes.
 */
class WDsrMainHelper
{
  public:
    /**
     * Create an WDsrMainHelper that makes life easier for people who want to install
     * WDSR routing to nodes.
     */
    WDsrMainHelper();
    ~WDsrMainHelper();
    /**
     * \brief Construct an WDsrMainHelper from another previously initialized instance
     * (Copy Constructor).
     * \param o object to copy from
     */
    WDsrMainHelper(const WDsrMainHelper& o);
    /**
     * Install routing to the nodes
     * \param wdsrHelper The WDSR helper class
     * \param nodes the collection of nodes
     */
    void Install(WDsrHelper& wdsrHelper, NodeContainer nodes);
    /**
     * Set the helper class
     * \param wdsrHelper the WDSR helper class
     */
    void SetWDsrHelper(WDsrHelper& wdsrHelper);

  private:
    /**
     * Install routing to a node
     * \param node the node to install WDSR routing
     */
    void Install(Ptr<Node> node);
    /**
     * \brief Assignment operator declared private and not implemented to disallow
     * assignment and prevent the compiler from happily inserting its own.
     * \param o source object to assign
     * \return WDsrHelper object
     */
    WDsrMainHelper& operator=(const WDsrMainHelper& o);
    const WDsrHelper* m_wdsrHelper; ///< helper class
};

} // namespace ns3

#endif /* WDSR_MAIN_HELPER_H */
