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

#include "posix/event_loop.h"
#include "udp/connection.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

typedef struct _lwm2m_event_loop_ {
    lwm2m_context_t *lwm2m_ctx;
    int sock;
    lwm2m_eventloop_handle_stdin handle_stdin;
    lwm2m_event_loop_output_err output_err;
    lwm2m_event_loop_read_from_socket read_from_socket;
} lwm2m_event_loop_t;

static bool eventloop_quit = false;

static bool eventloop_stop_requested(void) { return eventloop_quit; }

static uint8_t *packet_buffer = NULL;

lwm2m_event_loop_t *lwm2m_event_loop_init(lwm2m_context_t *lwm2m_ctx, int sock,
                                          lwm2m_eventloop_handle_stdin handle_stdin,
                                          lwm2m_event_loop_output_err output_err,
                                          lwm2m_event_loop_read_from_socket read_from_socket, size_t max_packet_size) {

    lwm2m_event_loop_t *event_loop = lwm2m_malloc(sizeof(lwm2m_event_loop_t));
    memset(event_loop, 0, sizeof(lwm2m_event_loop_t));

    event_loop->lwm2m_ctx = lwm2m_ctx;
    event_loop->sock = sock;
    event_loop->handle_stdin = handle_stdin;
    event_loop->output_err = output_err;
    event_loop->read_from_socket = read_from_socket;

    assert(packet_buffer == NULL);
    packet_buffer = lwm2m_malloc(max_packet_size);

    return event_loop;
}

int lwm2m_event_loop_run(lwm2m_event_loop_t *event_loop) {
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;

    fd_set readfds;
    lwm2m_connection_t *connList = NULL;

    while (!eventloop_stop_requested()) {
        FD_ZERO(&readfds);
        FD_SET(event_loop->sock, &readfds);
        if (event_loop->handle_stdin != NULL) {
            FD_SET(STDIN_FILENO, &readfds);
        }

        int result = lwm2m_step(event_loop->lwm2m_ctx, &(tv.tv_sec));
        if (result != 0) {
            event_loop->output_err("lwm2m_step() failed: 0x%X\r\n", result);
            return -1;
        }

        result = select(FD_SETSIZE, &readfds, 0, 0, &tv);

        if (result < 0) {
            if (errno != EINTR) {
                event_loop->output_err("Error in select(): %d\r\n", errno);
            }
        } else if (result > 0) {

            if (FD_ISSET(event_loop->sock, &readfds)) {
                struct sockaddr_storage addr;
                socklen_t addrLen;

                addrLen = sizeof(addr);
                ssize_t numBytes = event_loop->read_from_socket(event_loop->sock, packet_buffer, &addr, &addrLen);

                if (numBytes >= 0) {

                    lwm2m_connection_t *connP =
                        lwm2m_connection_find_or_new_incoming(&connList, event_loop->sock, addr, addrLen);
                    if (connP != NULL) {
                        lwm2m_handle_packet(event_loop->lwm2m_ctx, packet_buffer, numBytes, connP);
                    }
                }
            } else if (event_loop->handle_stdin != NULL && FD_ISSET(STDIN_FILENO, &readfds)) {
                event_loop->handle_stdin(event_loop->lwm2m_ctx);
            }
        }
    }

    lwm2m_connection_free(connList);
    return 0;
}

void lwm2m_eventloop_stop(void) { eventloop_quit = true; }

void lwm2m_event_loop_close(lwm2m_event_loop_t *event_loop) {
    lwm2m_free(event_loop);

    assert(packet_buffer != NULL);
    lwm2m_free(packet_buffer);
}
