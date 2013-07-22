/*************************************************************************
 * 
 * SEAMLESSSTACK CONFIDENTIAL
 * __________________________
 * 
 *  [2012] - [2013]  SeamlessStack Inc
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
#ifndef _SSTACK_SFSD_H_
#define _SSTACK_SFSD_H_

/**
 * Register all the relevant 
 * signals in this funtion
 **/
int32_t register_signals(void);

/** 
 * Create the thread pool
 **/

int32_t init_thread_pool(void);

/**
 * Run the actual SFS daemon
 **/
void run_daemon_sfsd(void);


#endif /* _STACK_SFSD_H_ */
