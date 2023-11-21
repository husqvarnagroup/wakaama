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

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "connection.h"
#include "liblwm2m.h"

#define SEND_BUFFER_MAX_LEN 100
static uint8_t send_buffer[SEND_BUFFER_MAX_LEN];
static size_t send_len = 0;

bool lwm2m_session_is_equal(void *session1, void *session2, void *userData) { return session1 == session2; }

uint8_t lwm2m_buffer_send(void *sessionH, uint8_t *buffer, size_t length, void *userdata) {

    (void)sessionH;
    (void)userdata;

    memset(send_buffer, 0, SEND_BUFFER_MAX_LEN);
    send_len = 0;

    assert(length <= SEND_BUFFER_MAX_LEN);
    if (length <= SEND_BUFFER_MAX_LEN) {
        send_len = length;
        memcpy(send_buffer, buffer, length);
    }

    return COAP_NO_ERROR;
}

uint8_t *test_get_send_buffer(size_t *len) {
    *len = send_len;
    return send_buffer;
}
