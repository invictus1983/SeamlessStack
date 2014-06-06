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

#ifndef __MONGO_DB_H__
#define __MONGO_DB_H__

#include <mongoc.h>
#include <sstack_db.h>
#include <sstack_log.h>
#define DB_NAME "sstack_db"
#define POLICY_COLLECTION "policy"
#define INODE_COLLECTION "inodes"
#define JOURNAL_COLLECTION "journal"
#define CONFIG_COLLECTION "config"
#define MONGO_IP	"127.0.0.1" // Will be updated with IP of host runnning DB
#define MONGO_PORT 27017
#define NAME_SIZE 32
#define REC_NAME_SIZE 32

extern int mongo_db_open(log_ctx_t *);
extern int mongo_db_close(log_ctx_t *);
extern int mongo_db_init(log_ctx_t *);
extern int mongo_db_insert(char * , char * , size_t , db_type_t , log_ctx_t *);
extern int mongo_db_remove(char * , db_type_t , log_ctx_t *);
extern int mongo_db_seekread(char * , char *, size_t , off_t ,
		int , db_type_t , log_ctx_t *);
extern int mongo_db_update(char * , char * , size_t , db_type_t , log_ctx_t *);
extern void mongo_db_iterate(db_type_t , iterator_function_t , void *,
				log_ctx_t *);
extern int mongo_db_get(char * , char ** , db_type_t , log_ctx_t *);
extern int mongo_db_delete(char * , log_ctx_t *);
extern int mongo_db_cleanup(log_ctx_t *);

static inline char *
get_collection_name(db_type_t type)
{
	switch(type) {
		case POLICY_TYPE:
			return  POLICY_COLLECTION;
		case INODE_TYPE:
			return INODE_COLLECTION;
		case JOURNAL_TYPE:
			return JOURNAL_COLLECTION;
		case CONFIG_TYPE:
			return CONFIG_COLLECTION;
			break;
		default:
			return NULL;
	}
}

static inline
int construct_record_name(char *name, db_type_t type)
{
	int ret = 0;

	memset(name, '\0', REC_NAME_SIZE);

	switch(type) {
		case POLICY_TYPE:
			strcpy(name, "policy_struct");
			break;
		case INODE_TYPE:
			strcpy(name, "inode_struct");
			break;
		case JOURNAL_TYPE:
			strcpy(name, "journal_struct");
			break;
		case CONFIG_TYPE:
			strcpy(name, "config_struct");
			break;
		default:
			ret = -1;
	}

	return ret;
}

#endif // __MONGO_DB_H__
