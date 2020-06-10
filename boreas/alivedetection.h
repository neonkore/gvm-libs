/* Copyright (C) 2020 Greenbone Networks GmbH
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef ALIVE_DETECTION_H
#define ALIVE_DETECTION_H

#include "../base/hosts.h"
#include "../util/kb.h"

#include <pcap.h>

/* to how many hosts are packets send to at a time. value <= 0 for no rate limit
 */
#define BURST 100
/* how long (in msec) to wait until new BURST of packets is send */
#define BURST_TIMEOUT 100000
/* how tong (in sec) to wait for replies after last packet was sent */
#define WAIT_FOR_REPLIES_TIMEOUT 5
/* Src port of outgoing TCP pings. Used for filtering incoming packets. */
#define FILTER_PORT 9910

/* Redis related */

/* Queue (Redis list) for communicating with openvas main process. */
#define ALIVE_DETECTION_QUEUE "alive_detection"
/* Signal to put on ALIVE_DETECTION_QUEUE if alive detection finished. */
#define ALIVE_DETECTION_FINISHED "alive_detection_finished"

void *
start_alive_detection (void *);

/**
 * @brief The scanner struct holds data which is used frequently by the alive
 * detection thread.
 */
struct scanner
{
  /* sockets */
  int tcpv4soc;
  int tcpv6soc;
  int icmpv4soc;
  int icmpv6soc;
  int arpv4soc;
  int arpv6soc;
  /* UDP socket needed for getting the source IP for the TCP header. */
  int udpv4soc;
  int udpv6soc;
  /* TH_SYN or TH_ACK */
  uint8_t tcp_flag;
  /* ports used for TCP ACK/SYN */
  GArray *ports;
  /* redis connection */
  kb_t main_kb;
  /* pcap handle */
  pcap_t *pcap_handle;
};

/**
 * @brief The hosts_data struct holds the alive hosts and target hosts in
 * separate hashtables.
 */
struct hosts_data
{
  /* Set of the form (ip_str, ip_str).
   * Hosts which passed our pcap filter. May include hosts which are alive but
   * are not in the targethosts list */
  GHashTable *alivehosts;
  /* Hashtable of the form (ip_str, gvm_host_t *). The gvm_host_t pointers point
   * to hosts which are to be freed by the caller of start_alive_detection(). */
  GHashTable *targethosts;
  /* Hosts which were detected as alive and are in the targetlist but are not
   * sent to openvas because max_scan_hosts was reached. */
  GHashTable *alivehosts_not_to_be_sent_to_openvas;
};

/**
 * @brief Alive tests.
 *
 * These numbers are used in the database by gvmd, so if the number associated
 * with any symbol changes in gvmd we need to change them here too.
 */
typedef enum
{
  ALIVE_TEST_TCP_ACK_SERVICE = 1,
  ALIVE_TEST_ICMP = 2,
  ALIVE_TEST_ARP = 4,
  ALIVE_TEST_CONSIDER_ALIVE = 8,
  ALIVE_TEST_TCP_SYN_SERVICE = 16
} alive_test_t;

/**
 * @brief Type of socket.
 */
typedef enum
{
  TCPV4,
  TCPV6,
  ICMPV4,
  ICMPV6,
  ARPV4,
  ARPV6,
  UDPV4,
  UDPV6,
} socket_type_t;

/* Getter methods for scan_restrictions. */

int
max_scan_hosts_reached ();
int
get_alive_hosts_count ();
int
get_max_scan_hosts ();

#endif /* not ALIVE_DETECTION_H */
