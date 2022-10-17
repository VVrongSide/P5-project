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

#ifndef WDSR_HELPER_H
#define WDSR_HELPER_H

#include "ns3/wdsr-routing.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/udp-l4-protocol.h"

namespace ns3
{

/**
 * \ingroup wdsr
 *
 * \brief WDSR helper class to manage creation of WDSR routing instance and
 *        to insert it on a node as a sublayer between transport and
 *        IP layers.
 */
class WDsrHelper
{
  public:
    /**
     * Create an WDsrHelper that makes life easier for people who want to install
     * WDsr routing to nodes.
     */
    WDsrHelper();
    ~WDsrHelper();

    // Delete assignment operator to avoid misuse
    WDsrHelper& operator=(const WDsrHelper&) = delete;

    /**
     * \brief Construct an WDsrHelper from another previously initialized instance
     * (Copy Constructor).
     * \param o object to copy from
     */
    WDsrHelper(const WDsrHelper& o);
    /**
     * \returns pointer to clone of this WDsrHelper
     *
     * This method is mainly for internal use by the other helpers;
     * clients are expected to free the dynamic memory allocated by this method
     */
    WDsrHelper* Copy() const;
    /**
     * \param node the node on which the routing protocol will run
     * \returns a newly-created L4 protocol
     */
    Ptr<ns3::wdsr::WDsrRouting> Create(Ptr<Node> node) const;
    /**
     * Set attribute values for future instances of WDSR that this helper creates
     * \param name the node on which the routing protocol will run
     * \param value newly-created L4 protocol
     */
    void Set(std::string name, const AttributeValue& value);

  private:
    ObjectFactory m_agentFactory; ///< WDSR factory
};

} // namespace ns3

#endif // WDSR_HELPER_H
