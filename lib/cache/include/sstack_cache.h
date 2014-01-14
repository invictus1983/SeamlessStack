/*************************************************************************
 * 
 * SEAMLESSSTACK CONFIDENTIAL
 * __________________________
 * 
 *  [2012] - [2014]  SeamlessStack Inc
 *  All Rights Reserved.
 * 
 * NOTICE:  All information contained herein is, and remains
 * the property of SeamlessStack Incorporated and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to SeamlessStack Incorporated
 * and its suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from SeamlessStack Incorporated.
 */
#ifndef __SSTACK_CACHE_H__
#define __SSTACK_CACHE_H__

#include <inttypes.h>
#include <pthread.h>
#include <time.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <sstack_log.h>
#include <sstack_ssdcache.h>
#include <libmemcached/memcached.h>
#define FILENAME_LEN 32

typedef struct mcache {
	memcached_st *mc; // Memcached connection info
	size_t len; // Size of the object
} sstack_mcache_t;


// This file defines the generic cache structure

typedef struct sstack_cache {
	pthread_mutex_t lock;
	uint8_t hashkey[SHA256_DIGEST_LENGTH + 1]; // Hash of file name and offset
	bool on_ssd; // Is it on SSD?
	union {
		sstack_ssdcache_t ssdcache; // Cache entry for SSD cache	
		sstack_mcache_t memcache; // Memcached structure
	};
	time_t time; // Used for LRU impl

} sstack_cache_t;

//  Generic cache functions
extern uint8_t * create_hash(void * , size_t , uint8_t *, log_ctx_t *);
extern int sstack_cache_store(void *, size_t , sstack_cache_t *, log_ctx_t *);
extern sstack_cache_t *sstack_cache_search(uint8_t *, log_ctx_t *);
extern char *sstack_cache_get(uint8_t *, size_t , log_ctx_t *);
extern int sstack_cache_purge(uint8_t *, log_ctx_t *);
extern int cache_init(log_ctx_t *);


#endif // __SSTACK_CACHE_H__
