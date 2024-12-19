/*******************************************************************************
 *
 * Copyright (c) 2024 GARDENA GmbH
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *   Lukas Woodtli, GARDENA GmbH - Please refer to git log
 *
 *******************************************************************************/

#ifndef WAKAAMA_EVENT_LOOP_H
#define WAKAAMA_EVENT_LOOP_H

#include "liblwm2m.h"

#include <sys/socket.h>
#include <sys/types.h>

/** Type of handler that reacts to stdin. */
typedef void (*lwm2m_eventloop_handle_stdin)(lwm2m_context_t *lwm2m_ctx);
/** Type of handler that prints error messages. */
typedef void (*lwm2m_event_loop_output_err)(char const *format, ...);
/** Type of handler that reads data from socket. */
typedef ssize_t (*lwm2m_event_loop_read_from_socket)(int sock, uint8_t *buffer, struct sockaddr_storage *addr,
                                                     socklen_t *addr_len);

/** Opaque type that encapsulates an event loop */
typedef struct _lwm2m_event_loop_ lwm2m_event_loop_t;

/** Initialize the event loop
 *
 * @param lwm2m_ctx The lwm2m context to operate on
 * @param sock The socket for the CoAP communication
 * @param handle_stdin Handler function to act on stdin
 * @param output_err Handler function to print errors
 * @param read_from_socket Handler function to read from the socker
 * @param max_packet_size Size of buffer used to read from buffer
 * @return Pointer to event loop instance (needs to be cleared with lwm2m_event_loop_close)
 */
lwm2m_event_loop_t *lwm2m_event_loop_init(lwm2m_context_t *lwm2m_ctx, int sock,
                                          lwm2m_eventloop_handle_stdin handle_stdin,
                                          lwm2m_event_loop_output_err output_err,
                                          lwm2m_event_loop_read_from_socket read_from_socket, size_t max_packet_size);

/** Run the event loop
 * The event loop runs until it is stopped (lwm2m_eventloop_stop)
 * or an error occurs.
 * @param event_loop Event loop instance to run
 * @return 0 on ok, -1 on error
 */
int lwm2m_event_loop_run(lwm2m_event_loop_t *event_loop);

/** Stop the event loop */
void lwm2m_eventloop_stop(void);


/** Cloase and clean up the event loop
 *
 * @param event_loop The event loop instance to be closed
 */
void lwm2m_event_loop_close(lwm2m_event_loop_t *event_loop);

#endif // WAKAAMA_EVENT_LOOP_H
