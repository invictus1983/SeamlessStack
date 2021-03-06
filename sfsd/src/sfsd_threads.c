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

#include <pthread.h>
#include <stdint.h>
#include <sstack_sfsd.h>
#include <sstack_log.h>
#include <sstack_chunk.h>
#include <sstack_jobs.h>
#include <sfsd_ops.h>
#include <sstack_md.h>
#include <sstack_types.h>
#include <sstack_serdes.h>

extern bds_cache_desc_t sfsd_global_cache_arr[];
/* ===================== PRIVATE DECLARATIONS ============================ */

static void* do_receive_thread (void *params);
static void handle_command(sstack_payload_t *command,
		sstack_payload_t **response,
		bds_cache_desc_t cache_arr[2], sfsd_t *sfsd,
		log_ctx_t *log_ctx);
static inline void send_unsucessful_response(sstack_payload_t *payload,
		sfsd_t *sfsd);
static sstack_payload_t* get_payload(sstack_transport_t *transport,
		sstack_client_handle_t handle);
static int32_t send_payload(sstack_transport_t *transport,
							sstack_client_handle_t handle, sstack_payload_t *response,
							log_ctx_t *ctx);

/* ====================+++++++++++++++++++++++++++++++++==================*/
static inline void send_unsucessful_response(sstack_payload_t *payload,
		sfsd_t *sfsd)
{
	if (payload) {
		payload->response_struct.command_ok = -1;
		send_payload(sfsd->transport, sfsd->handle, payload, sfsd->log_ctx);
	}
}

static int32_t sfsd_create_receiver_thread(sfsd_t *sfsd)
{
	int32_t ret = 0;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	ret = pthread_create(&sfsd->receiver_thread, &attr,
			do_receive_thread, sfsd);
	SFS_LOG_EXIT((ret == 0), "Receiver thread create failed\n", ret,
			sfsd->log_ctx, SFS_ERR);

	sfs_log(sfsd->log_ctx, SFS_INFO, "Receive thread creation successful\n");
	return 0;
}

int32_t init_thread_pool(sfsd_t *sfsd)
{
	int32_t ret = 0;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	ret = sfsd_create_receiver_thread(sfsd);
	SFS_LOG_EXIT((ret == 0), "Could not create receiver thread\n",
			ret, sfsd->log_ctx, SFS_ERR);

	sfsd->thread_pool = sstack_thread_pool_create(1, 5, 30, &attr);
	SFS_LOG_EXIT((sfsd->thread_pool != NULL),
			"Could not create sfsd thread pool\n",
			ret, sfsd->log_ctx, SFS_ERR);

	sfsd->chunk_thread_pool = sstack_thread_pool_create(1, 2, 0, &attr);
	SFS_LOG_EXIT((sfsd->thread_pool != NULL),
			"Could not create sfsd chunk thread pool\n",
			ret, sfsd->log_ctx, SFS_ERR);

	sfs_log(sfsd->log_ctx, SFS_INFO, "Thread pool initialized\n");
	return 0;

}

static sstack_payload_t* get_payload(sstack_transport_t *transport,
		sstack_client_handle_t handle)
{
	sstack_payload_t *payload = NULL;
	ssize_t nbytes = 0;
	sstack_transport_ops_t *ops = &transport->transport_ops;

	payload = malloc(sizeof(*payload));
	SFS_LOG_EXIT((payload != NULL), "Allocate payload failed\n", NULL,
			transport->ctx, SFS_ERR);
	/* Read of the payload */
	nbytes = ops->rx(handle, sizeof(*payload), payload, transport->ctx);
	sfs_log (transport->ctx, ((nbytes == 0) ? SFS_ERR : SFS_DEBUG),
			"Payload size: %d\n", nbytes);

	return payload;
}

int32_t send_payload(sstack_transport_t *transport,
					 sstack_client_handle_t handle, sstack_payload_t *payload,
					 log_ctx_t *ctx)
{
	int ret = -1;
	sstack_job_id_t job_id = 0;
	int priority = -1;
	sfs_job_t *job = NULL;

	// Parameter validation
	if (NULL == transport || handle == -1 || NULL == payload) {
		// Failure
		// Handle failure
		return -1;
	}
	job_id = payload->hdr.job_id;
	priority = payload->hdr.priority;
	job = (sfs_job_t *) payload->hdr.arg;

	ret = sstack_send_payload(handle, payload, transport, job_id, job,
					priority, ctx);
	return ret;
}

static void* do_process_payload(void *param)
{
	struct handle_payload_params *h_param =
		(struct handle_payload_params *)param;
	sstack_payload_t *command = h_param->payload;
	sstack_payload_t *response = NULL;
	bds_cache_desc_t param_cache = h_param->cache_p;
	handle_command(command, &response, h_param->cache_arr,
		       h_param->sfsd, h_param->log_ctx);

	sfs_log(h_param->log_ctx, SFS_DEBUG,"response code: %d, response status: %d\n",
			response->response_struct.command_ok, response->command);

	/* Send of the response */
	if (send_payload(h_param->sfsd->transport,
					 h_param->sfsd->handle, response, h_param->log_ctx)) {
		sfs_log (h_param->log_ctx, SFS_ERR, "Error sending payload\n");
	}
	free_payload_protobuf(sfsd_global_cache_arr, response);
	/* Free of the param cache */
	bds_cache_free(param_cache, h_param);

	/* Free off the response */
	// TODO
	// response structure is allocated from malloc
	// sstack_send_payload and sstack_recv_payload still use malloc/free
	// bds_cache_free(h_param->cache_arr[PAYLOAD_CACHE_OFFSET], response);
	//free(response);

	return NULL;
}

static void* do_receive_thread(void *param)
{
	sfsd_t *sfsd = (sfsd_t *)param;
	int32_t ret = 0;
	uint32_t mask = READ_BLOCK_MASK;
	sstack_payload_t *payload;
	struct handle_payload_params *handle_params;
	bds_cache_desc_t param_cache = NULL;
	/* Check for the handle, if it doesn't exist
	   simply exit */
	SFS_LOG_EXIT((sfsd->handle != 0),
			"Transport handle doesn't exist. Exiting..\n",
			NULL, sfsd->log_ctx, SFS_ERR);

	sfs_log(sfsd->log_ctx, SFS_DEBUG, "%s: %d \n", __FUNCTION__, __LINE__);

	param_cache = sfsd->local_caches[HANDLE_PARAM_OFFSET];

	while (1) {
		/* Check whether there is some command from the
		   sfs coming */
		ret = sfsd->transport->transport_ops.select(sfsd->handle, mask);
		if (ret != READ_NO_BLOCK) {
			/* Connection is down, wait for retry */
			//sleep (1);
			/* Select in this case is non-blocking. We could come out
			 * even when there is nothing to read. So, go back to select
			 */
			continue;
		}
		sfs_log(sfsd->log_ctx, SFS_DEBUG, "%s: %d \n", __FUNCTION__, __LINE__);
		payload = sstack_recv_payload(sfsd->handle, sfsd->transport,
				sfsd->log_ctx);
		/* After getting the payload, assign a thread pool from the
		   thread pool to do the job */
		if (payload != NULL) {
			/* Command could be
			   1) Chunk domain command
			   2) Standard NFS command */
			handle_params = bds_cache_alloc(param_cache);
			if (handle_params == NULL) {
				sfs_log(sfsd->log_ctx, SFS_ERR,
						"Failed to allocate %s",
						"handle_params\n");
				send_unsucessful_response(payload, sfsd);
				continue;
			}
			handle_params->payload = payload;
			handle_params->log_ctx = sfsd->log_ctx;
			handle_params->cache_arr =
				sfsd->local_caches;
			handle_params->cache_p = param_cache;
			handle_params->sfsd = sfsd;
			ret = sstack_thread_pool_queue(sfsd->thread_pool,
					do_process_payload, handle_params);
			sfs_log(sfsd->log_ctx,
				((ret == 0) ? SFS_DEBUG: SFS_ERR),
				"Job queue status: %d\n", ret);
		}
		usleep(100);
	}
}

void handle_command(sstack_payload_t *command, sstack_payload_t **response,
		bds_cache_desc_t *cache_arr, sfsd_t *sfsd, log_ctx_t *log_ctx)
{
	sfsd_storage_t *storage;
	char *path;
	int32_t ret = 0;

	switch(command->command)
	{
		/* sstack storage commands here */
		case SSTACK_ADD_STORAGE:
			storage = &command->storage;
			sfs_log(log_ctx, SFS_DEBUG, "%s: path = %s ipv4addr = %s\n",
					__FUNCTION__, storage->path, storage->address.ipv4_address);
			path = sfsd_add_chunk(sfsd->chunk, storage);
			if (path) {
				free(path);
				ret = SSTACK_SUCCESS;
			} else {
				ret = -EINVAL;
			}
			command->command = SSTACK_ADD_STORAGE_RSP;
			command->response_struct.command_ok = ret;
			*response = command;
			break;
		case SSTACK_UPDATE_STORAGE:
			storage = &command->storage;
			if (0 != (ret = sfsd_update_chunk(sfsd->chunk,
							storage))) {
				sfs_log(log_ctx, SFS_ERR, "Unable to update"
						"chunk\n");
			}
			command->command = SSTACK_UPDATE_STORAGE_RSP;
			command->response_struct.command_ok = ret;
			*response = command;
			break;
		case SSTACK_REMOVE_STORAGE:
			storage = &command->storage;
			if (0 != (ret = sfsd_remove_chunk(sfsd->chunk,
							storage))) {
				sfs_log(log_ctx, SFS_ERR, "Unable to remove"
						"chunk\n");
			}
			command->command = SSTACK_REMOVE_STORAGE_RSP;
			command->response_struct.command_ok = ret;
			*response = command;
			break;
		/* sstack nfs commands here */
		case NFS_GETATTR:
			*response = sstack_getattr(command, log_ctx);
			(*response)->command = NFS_GETATTR_RSP;
			break;
		case NFS_SETATTR:
			*response = sstack_setattr(command, log_ctx);
			(*response)->command = NFS_SETATTR_RSP;
			break;
		case NFS_LOOKUP:
			*response = sstack_lookup(command, log_ctx);
			(*response)->command = NFS_LOOKUP_RSP;
			break;
		case NFS_ACCESS:
			*response = sstack_access(command, log_ctx);
			(*response)->command = NFS_ACCESS_RSP;
			break;
		case NFS_READLINK:
			*response = sstack_readlink(command, log_ctx);
			(*response)->command = NFS_READLINK_RSP;
			break;
		case NFS_READ:
			*response = sstack_read(command, sfsd,log_ctx);
			(*response)->command = NFS_READ_RSP;
			break;
		case NFS_WRITE:
			sfs_log(log_ctx, SFS_DEBUG, "write command\n");
			*response = sstack_write(command, sfsd, log_ctx);
			(*response)->command = NFS_WRITE_RSP;
			break;
		case NFS_CREATE:
			*response = sstack_create(command, sfsd, log_ctx);
			(*response)->command = NFS_CREATE_RSP;
			break;
		case NFS_MKDIR:
			*response = sstack_mkdir(command, log_ctx);
			(*response)->command = NFS_MKDIR_RSP;
			break;
		case NFS_SYMLINK:
			*response = sstack_symlink(command, log_ctx);
			(*response)->command = NFS_SYMLINK_RSP;
			break;
		case NFS_MKNOD:
			*response = sstack_mknod(command, log_ctx);
			(*response)->command = NFS_MKNOD_RSP;
			break;
		case NFS_REMOVE:
			*response = sstack_remove(command, log_ctx);
			(*response)->command = NFS_REMOVE_RSP;
			break;
		case NFS_RMDIR:
			*response = sstack_rmdir(command, log_ctx);
			(*response)->command = NFS_RMDIR_RSP;
			break;
		case NFS_RENAME:
			*response = sstack_rename(command, log_ctx);
			(*response)->command = NFS_RENAME_RSP;
			break;
		case NFS_LINK:
			*response = sstack_link(command, log_ctx);
			(*response)->command = NFS_LINK_RSP;
			break;
		case NFS_READDIR:
			*response = sstack_readdir(command, log_ctx);
			(*response)->command = NFS_READDIR_RSP;
			break;
		case NFS_READDIRPLUS:
			*response = sstack_readdirplus(command, log_ctx);
			(*response)->command = NFS_READDIRPLUS_RSP;
			break;
		case NFS_FSSTAT:
			*response = sstack_fsstat(command, log_ctx);
			(*response)->command = NFS_FSSTAT_RSP;
			break;
		case NFS_FSINFO:
			*response = sstack_fsinfo(command, log_ctx);
			(*response)->command = NFS_FSINFO_RSP;
			break;
		case NFS_PATHCONF:
			*response = sstack_pathconf(command, log_ctx);
			(*response)->command = NFS_PATHCONF_RSP;
			break;
		case NFS_COMMIT:
			*response = sstack_commit(command, log_ctx);
			(*response)->command = NFS_COMMIT_RSP;
			break;
		case NFS_ESURE_CODE:
			*response = sstack_esure_code(command, sfsd, log_ctx);
			(*response)->command = NFS_ESURE_CODE_RSP;
			break;
		default:
			break;
	};

	(*response)->hdr.sequence = command->hdr.sequence;
	/* Send off response from here */
	//sstack_send_payload(sfsd->handle, *response, sfsd->transport,
	//					(*response)->hdr.job_id, (sfs_job_t*)(*response)->hdr.arg,
	//					(*response)->hdr.priority, log_ctx);
	//

}
