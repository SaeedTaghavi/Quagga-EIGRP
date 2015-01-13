/*
 * EIGRP Definition of Data Structures.
 * Copyright (C) 2013-2014
 * Authors:
 *   Donnie Savage
 *   Jan Janovic
 *   Matej Perina
 *   Peter Orsag
 *   Peter Paluch
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _ZEBRA_EIGRP_STRUCTS_H_
#define _ZEBRA_EIGRP_STRUCTS_H_

#include "eigrpd/eigrp_const.h"
#include "eigrpd/eigrp_macros.h"

/* EIGRP master for system wide configuration and variables. */
struct eigrp_master
{
  /* EIGRP instance. */
  struct list *eigrp;

  /* EIGRP thread master. */
  struct thread_master *master;

  /* Zebra interface list. */
  struct list *iflist;

  /* EIGRP start time. */
  time_t start_time;

  /* Various EIGRP global configuration. */
  u_char options;

#define EIGRP_MASTER_SHUTDOWN (1 << 0) /* deferred-shutdown */
};

struct eigrp_metrics
{
  u_int32_t delay;
  u_int32_t bandwith;
  unsigned char mtu[3];
  u_char hop_count;
  u_char reliability;
  u_char load;
  u_char tag;
  u_char flags;
};

struct eigrp
{
  u_int16_t AS;			/* Autonomous system number */
  u_int16_t vrid;		/* Virtual Router ID */
  u_char    k_values[6];	/*Array for K values configuration*/
  u_char variance;              /*Metric variance multiplier*/

  /* EIGRP Router ID. */
  u_int32_t router_id; /* Configured automatically. */
  u_int32_t router_id_static; /* Configured manually. */

  struct list *eiflist; /* eigrp interfaces */
  u_char passive_interface_default; /* passive-interface default */

  unsigned int fd;
  unsigned int maxsndbuflen;

  u_int32_t sequence_number; /*Global EIGRP sequence number*/

  struct stream *ibuf;
  struct list *oi_write_q;

  /*Threads*/
  struct thread *t_write;
  struct thread *t_read;

  struct route_table *networks; /* EIGRP config networks. */


  struct list *topology_table;

  /*Neighbor self*/
  struct eigrp_neighbor *neighbor_self;

  /*Configured metric for redistributed routes*/
  struct eigrp_metrics dmetric[ZEBRA_ROUTE_MAX + 1];
  int redistribute;           /* Num of redistributed protocols. */

};

//------------------------------------------------------------------------------------------------------------------------------------------

/*EIGRP interface structure*/
struct eigrp_interface
{
  /* This interface's parent eigrp instance. */
  struct eigrp *eigrp;

  /* Interface data from zebra. */
  struct interface *ifp;

  /* Packet send buffer. */
  struct eigrp_fifo *obuf; /* Output queue */

  /* To which multicast groups do we currently belong? */

  /* Configured varables. */
  struct eigrp_if_params *params;

  u_char multicast_memberships;

  /* EIGRP Network Type. */
  u_char type;

  struct prefix *address; /* Interface prefix */
  struct connected *connected; /* Pointer to connected */

  /* Neighbor information. */
  struct list *nbrs; /* EIGRP Neighbor List */

  /* Threads. */
  struct thread *t_hello; /* timer */

  int on_write_q;

  /* Statistics fields. */
  u_int32_t hello_in; /* Hello message input count. */
  u_int32_t update_in; /* Update message input count. */
  u_int32_t query_in; /* Querry message input count. */
  u_int32_t reply_in; /* Querry message input count. */

  u_int32_t crypt_seqnum;             /* Cryptographic Sequence Number */
};

struct eigrp_if_params
{
  DECLARE_IF_PARAM (u_char, passive_interface); /* EIGRP Interface is passive: no sending or receiving (no need to join multicast groups) */
  DECLARE_IF_PARAM (u_int32_t, v_hello); /* Hello Interval */
  DECLARE_IF_PARAM (u_int16_t, v_wait); /* Router Hold Time Interval */
  DECLARE_IF_PARAM (u_char, type); /* type of interface */
  DECLARE_IF_PARAM (u_int32_t, bandwidth);
  DECLARE_IF_PARAM (u_int32_t, delay);
  DECLARE_IF_PARAM (u_char, reliability);
  DECLARE_IF_PARAM (u_char, load);

  DECLARE_IF_PARAM (char *, auth_keychain );    /* Associated keychain with interface*/
  DECLARE_IF_PARAM (int, auth_type);         /* EIGRP authentication type */
};

enum
{
  MEMBER_ALLROUTERS = 0, MEMBER_MAX,
};

struct eigrp_if_info
{
  struct eigrp_if_params *def_params;
  struct route_table *params;
  struct route_table *eifs;
  unsigned int membership_counts[MEMBER_MAX]; /* multicast group refcnts */
};

//------------------------------------------------------------------------------------------------------------------------------------------

/* Neighbor Data Structure */
struct eigrp_neighbor
{
  /* This neighbor's parent eigrp interface. */
  struct eigrp_interface *ei;

  /* EIGRP neighbor Information */
  u_char state; /* neigbor status. */

  u_int32_t recv_sequence_number; /* Last received sequence Number. */
  u_int32_t init_sequence_number;

  /*If packet is unacknowledged, we try to send it again 16 times*/
  u_char retrans_counter;

  struct in_addr src; /* Neighbor Src address. */

  u_char os_rel_major;		// system version - just for show
  u_char os_rel_minor;		// system version - just for show
  u_char tlv_rel_major;		// eigrp version - tells us what TLV format to use
  u_char tlv_rel_minor;		// eigrp version - tells us what TLV format to use

  u_char K1;
  u_char K2;
  u_char K3;
  u_char K4;
  u_char K5;
  u_char K6;

  /* Timer values. */
  u_int16_t v_holddown;

  /* Threads. */
  struct thread *t_holddown;

  struct eigrp_fifo *retrans_queue;
  struct eigrp_fifo *multicast_queue;

  u_int32_t crypt_seqnum;           /* Cryptographic Sequence Number. */
};

//---------------------------------------------------------------------------------------------------------------------------------------------


struct eigrp_packet
{
  struct eigrp_packet *next;
  struct eigrp_packet *previous;

  /* Pointer to data stream. */
  struct stream *s;

  /* IP destination address. */
  struct in_addr dst;

  /*Packet retransmission thread*/
  struct thread *t_retrans_timer;

  /*Packet retransmission counter*/
  u_char retrans_counter;

  u_int32_t sequence_number;

  /* EIGRP packet length. */
  u_int16_t length;
};

struct eigrp_fifo
{
  struct eigrp_packet *head;
  struct eigrp_packet *tail;

  unsigned long count;
};

struct eigrp_header
{
  u_char version;
  u_char opcode;
  u_int16_t checksum;
  u_int32_t flags;
  u_int32_t sequence;
  u_int32_t ack;
  u_int16_t vrid;
  u_int16_t ASNumber;
  char *tlv[0];

}__attribute__((packed));


/**
 * Generic TLV type used for packet decoding.
 *
 *      +-----+------------------+
 *      |     |     |            |
 *      | Type| Len |    Vector  |
 *      |     |     |            |
 *      +-----+------------------+
 */
struct eigrp_tlv_hdr_type
{
  u_int16_t type;
  u_int16_t length;
  uint8_t  value[0];
}__attribute__((packed));

struct TLV_Parameter_Type
{
  u_int16_t type;
  u_int16_t length;
  u_char K1;
  u_char K2;
  u_char K3;
  u_char K4;
  u_char K5;
  u_char K6;
  u_int16_t hold_time;
}__attribute__((packed));

struct TLV_Authentication_Type
{
  u_int16_t type;
  u_int16_t length;
  u_int16_t auth_type;
  u_int16_t auth_length;
  u_int32_t key_id;
  u_int32_t key_sequence;
  u_char Nullpad[8];
  u_char digest[EIGRP_AUTH_TYPE_MD5_LEN];

}__attribute__((packed));

struct TLV_Sequence_Type
{
  u_int16_t type;
  u_int16_t length;
  u_char addr_length;
  struct in_addr *addresses;
}__attribute__((packed));

struct TLV_Next_Multicast_Sequence
{
  u_int16_t type;
  u_int16_t length;
  u_int32_t multicast_sequence;
}__attribute__((packed));

struct TLV_Software_Type
{
  u_int16_t type;
  u_int16_t length;
  u_char vender_major;
  u_char vender_minor;
  u_char eigrp_major;
  u_char eigrp_minor;
}__attribute__((packed));

struct TLV_IPv4_Internal_type
{
  u_int16_t type;
  u_int16_t length;
  struct in_addr forward;

  /*Metrics*/
  struct eigrp_metrics metric;

  u_char prefix_length;

  unsigned char destination_part[4];
  struct in_addr destination;
}__attribute__((packed));

struct TLV_IPv4_External_type
{
  u_int16_t type;
  u_int16_t length;
  struct in_addr next_hop;
  struct in_addr originating_router;
  u_int32_t originating_as;
  u_int32_t administrative_tag;
  u_int32_t external_metric;
  u_int16_t reserved;
  u_char external_protocol;
  u_char external_flags;

  /*Metrics*/
  struct eigrp_metrics metric;

  u_char prefix_length;
  unsigned char destination_part[4];
  struct in_addr destination;
}__attribute__((packed));

//---------------------------------------------------------------------------------------------------------------------------------------------

/* EIGRP Topology table node structure */
struct eigrp_prefix_entry
{
  struct list *entries, *rij;
  u_int32_t fdistance;						// FD
  u_int32_t rdistance;						// RD
  u_int32_t distance;						// D
  struct eigrp_metrics reported_metric;		// RD for sending

  u_char nt;                                                    //network type
  u_char state; 							//route fsm state
  u_char af;								// address family

  struct prefix_ipv4 *destination_ipv4;		// pointer to struct with ipv4 address
  struct prefix_ipv6 *destination_ipv6;		// pointer to struct with ipv6 address

  //If network type is REMOTE_EXTERNAL, pointer will have reference to its external TLV
  struct TLV_IPv4_External_type *extTLV;
};

/* EIGRP Topology table record structure */
struct eigrp_neighbor_entry
{
  struct eigrp_prefix_entry *prefix;
  u_int32_t reported_distance; 				//distance reported by neighbor
  u_int32_t distance; 						//sum of reported distance and link cost to advertised neighbor

  struct eigrp_metrics reported_metric;
  struct eigrp_metrics total_metric;

  struct eigrp_neighbor *adv_router; 		//ip address of advertising neighbor
  u_char flags; 							//used for marking successor and FS

  struct eigrp_interface *ei; 				//pointer for case of connected entry

};

//---------------------------------------------------------------------------------------------------------------------------------------------

/* EIGRP Finite State Machine */

struct eigrp_fsm_action_message
{
  u_char packet_type; 					//UPDATE, QUERY, SIAQUERY, SIAREPLY
  struct eigrp *eigrp;					// which thread sent mesg
  struct eigrp_neighbor *adv_router; 	//advertising neighbor
  struct eigrp_neighbor_entry *entry;
  struct eigrp_prefix_entry *prefix;
  int data_type; // internal or external tlv type
  union{
    struct TLV_IPv4_External_type *ipv4_ext_data;
    struct TLV_IPv4_Internal_type *ipv4_int_type;
  }data;
};

#endif /* _ZEBRA_EIGRP_STRUCTURES_H_ */
