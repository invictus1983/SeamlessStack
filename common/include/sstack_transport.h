/*
 * Copyright (C) 2014 SeamlessStack
 *
 *  This file is part of SeamlessStack distributed file system.
 * 
 * SeamlessStack distributed file system is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of the License,
 * or (at your option) any later version.
 * 
 * SeamlessStack distributed file system is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SeamlessStack distributed file system. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef __SSTACK_TRANSPORT_H_
#define __SSTACK_TRANSPORT_H_

#include <sstack_log.h>
#include <sstack_types.h>

#define SSTACK_BACKLOG 128

#define READ_BLOCK_MASK	1
#define WRITE_BLOCK_MASK 2

#define IGNORE_NO_BLOCK 0
#define READ_NO_BLOCK 1
#define WRITE_NO_BLOCK 2

#define MAX_RECV_RETRIES 10

typedef uint32_t sstack_handle_t;

typedef enum {
	TCPIP   = 1,
	IB  = 2,
	UDP = 3,
	UNIX    = 4,
} sstack_transport_type_t;

typedef struct sstack_transport sstack_transport_t; // Forward decl
sstack_transport_t* get_tcp_transport(char *addr, log_ctx_t *ctx);
/*
 *  init() is supposed to establish a connection and retrun client handle
 *  Client handle is socket fd i case of TCPIP
 */
typedef struct sstack_transport_ops {
	// First three are for client
	sstack_client_handle_t (*client_init) (sstack_transport_t *);
	int (*tx) (sstack_client_handle_t , size_t  , void *, log_ctx_t *);
	int (*rx) (sstack_client_handle_t , size_t , void *, log_ctx_t *);
	int (*select)(sstack_client_handle_t, uint32_t block_flags);
	// This one is for server side
	// Called only by sfs.
	sstack_client_handle_t  (*server_setup) (sstack_transport_t *);
} sstack_transport_ops_t;

typedef struct sstack_transport_hdr {
	union {
		struct tcp {
			int ipv4;       // 1 for ipv4 and 0 for ipv6
			union {
				char ipv4_addr[IPV4_ADDR_MAX];
				char ipv6_addr[IPV6_ADDR_MAX];
			};
			int port;
		} tcp;
	};
} sstack_transport_hdr_t;
	

struct sstack_transport {
	sstack_transport_type_t  transport_type;
	sstack_transport_hdr_t transport_hdr;
	sstack_transport_ops_t transport_ops;
	log_ctx_t *ctx;
};


inline sstack_transport_t * alloc_transport(void);
inline void free_transport(sstack_transport_t *);

#endif // __SSTACK_TRANSPORT_H_
