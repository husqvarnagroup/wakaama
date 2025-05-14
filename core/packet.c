/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
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
 *    David Navarro, Intel Corporation - initial API and implementation
 *    domedambrosio - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Tuve Nordius, Husqvarna Group - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>

*/

/*
Contains code snippets which are:

 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.

*/


#include "internals.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#if __STDC_VERSION__ >= 201112L
#include <assert.h>
static_assert(LWM2M_COAP_DEFAULT_BLOCK_SIZE == 16 || LWM2M_COAP_DEFAULT_BLOCK_SIZE == 32 ||
                  LWM2M_COAP_DEFAULT_BLOCK_SIZE == 64 || LWM2M_COAP_DEFAULT_BLOCK_SIZE == 128 ||
                  LWM2M_COAP_DEFAULT_BLOCK_SIZE == 256 || LWM2M_COAP_DEFAULT_BLOCK_SIZE == 512 ||
                  LWM2M_COAP_DEFAULT_BLOCK_SIZE == 1024,
              "Block transfer options support only power-of-two block sizes, from 2**4 (16) to 2**10 (1024) bytes.");
#endif

uint16_t coap_block_size = LWM2M_COAP_DEFAULT_BLOCK_SIZE;

static bool validate_block_size(const uint16_t coap_block_size_arg) {
    const uint16_t valid_block_sizes[7] = {16, 32, 64, 128, 256, 512, 1024};
    int i;
    for (i = 0; i < 7; i++) {
        if (coap_block_size_arg == valid_block_sizes[i]) {
            return true;
        }
    }
    return false;
}

bool lwm2m_set_coap_block_size(const uint16_t coap_block_size_arg) {
    if (validate_block_size(coap_block_size_arg)) {
        coap_block_size = coap_block_size_arg;
        return true;
    }
    return false;
}

uint16_t lwm2m_get_coap_block_size(void) { return coap_block_size; }

uint16_t lwm2m_get_coap_message_size(void) { return (uint16_t)LWM2M_COAP_MAX_MESSAGE_SIZE; }

static void handle_reset(lwm2m_context_t * contextP,
                         void * fromSessionH,
                         coap_packet_t * message)
{
#ifdef LWM2M_CLIENT_MODE
    LOG_DBG("Entering");
    observe_cancel(contextP, message->mid, fromSessionH);
#endif
}

static uint8_t handle_request(lwm2m_context_t * contextP,
                              void * fromSessionH,
                              coap_packet_t * message,
                              coap_packet_t * response)
{
    lwm2m_uri_t uri;
    lwm2m_request_type_t requestType;
    uint8_t result = COAP_IGNORE;

    LOG_DBG("Entering");

#ifdef LWM2M_CLIENT_MODE
    requestType = uri_decode(contextP->altPath, message->uri_path, message->code, &uri);
#else
    requestType = uri_decode(NULL, message->uri_path, message->code, &uri);
#endif

    switch(requestType)
    {
    case LWM2M_REQUEST_TYPE_UNKNOWN:
        return COAP_400_BAD_REQUEST;

#ifdef LWM2M_CLIENT_MODE
    case LWM2M_REQUEST_TYPE_DM:
    {
        lwm2m_server_t * serverP;

        serverP = utils_findServer(contextP, fromSessionH);
        if (serverP != NULL)
        {
            result = dm_handleRequest(contextP, &uri, serverP, message, response);
        }
#ifdef LWM2M_BOOTSTRAP
        else
        {
            serverP = utils_findBootstrapServer(contextP, fromSessionH);
            if (serverP != NULL)
            {
                result = bootstrap_handleCommand(contextP, &uri, serverP, message, response);
            }
        }
#endif
    }
    break;

#ifdef LWM2M_BOOTSTRAP
    case LWM2M_REQUEST_TYPE_DELETE_ALL:
        result = bootstrap_handleDeleteAll(contextP, fromSessionH);
        break;

    case LWM2M_REQUEST_TYPE_BOOTSTRAP:
        if (message->code == COAP_POST)
        {
            result = bootstrap_handleFinish(contextP, fromSessionH);
        }
        break;
#endif
#endif

#ifdef LWM2M_SERVER_MODE
    case LWM2M_REQUEST_TYPE_REGISTRATION:
        result = registration_handleRequest(contextP, &uri, fromSessionH, message, response);
        break;
#ifndef LWM2M_VERSION_1_0
    case LWM2M_REQUEST_TYPE_SEND:
        result = reporting_handleSend(contextP, fromSessionH, message);
        break;
#endif
#endif
#ifdef LWM2M_BOOTSTRAP_SERVER_MODE
    case LWM2M_REQUEST_TYPE_BOOTSTRAP:
        result = bootstrap_handleRequest(contextP, &uri, fromSessionH, message, response);
        break;
#endif
    default:
        result = COAP_IGNORE;
        break;
    }

    coap_set_status_code(response, result);

    if (COAP_IGNORE < result && result < COAP_400_BAD_REQUEST)
    {
        result = NO_ERROR;
    }

    return result;
}

/**
 * @brief Find a transaction by message ID.
 */
static lwm2m_transaction_t *transaction_get_by_mid(lwm2m_context_t *contextP, void *sessionH, uint16_t mid) {
    lwm2m_transaction_t *transaction = contextP->transactionList;
    while (transaction != NULL) {
        if (lwm2m_session_is_equal(sessionH, transaction->peerH, contextP->userData) == false) {
            transaction = transaction->next;
            continue;
        }
        if (transaction->mID == mid) {
            return transaction;
        }
    }
    return NULL;
}

/**
 * @brief Find a transaction by CoAP token.
 */
static lwm2m_transaction_t *transaction_get_by_token(lwm2m_context_t *contextP, void *sessionH, uint8_t *token,
                                                     uint8_t token_len) {
    lwm2m_transaction_t *transaction = contextP->transactionList;
    while (transaction != NULL) {
        if (lwm2m_session_is_equal(sessionH, transaction->peerH, contextP->userData) == false) {
            transaction = transaction->next;
            continue;
        }
        coap_packet_t *message = (coap_packet_t *)transaction->message;
        if (message == NULL) {
            transaction = transaction->next;
            continue;
        }
        if (token != NULL && token_len > 0) {
            if (message->token_len != token_len) {
                transaction = transaction->next;
                continue;
            }
            if (memcmp(message->token, token, token_len) == 0) {
                return transaction;
            }
        }
    }
    return NULL;
}

/**
 * @brief Create a new transaction for the next block-wise transfer request.
 */
static lwm2m_transaction_t *transaction_create_next_block(lwm2m_transaction_t *transaction, uint16_t nextMID) {
    static coap_packet_t old_message[1];
    if (0 != coap_parse_message(old_message, transaction->buffer, transaction->buffer_len)) {
        return NULL;
    }
    lwm2m_uri_t uri;
    uri_decode(NULL, old_message->uri_path, old_message->code, &uri);
    lwm2m_transaction_t *clone = transaction_new(transaction->peerH, (coap_method_t)old_message->code, NULL, &uri,
                                                 nextMID, old_message->token_len, old_message->token);
    if (clone == NULL) return NULL;

    coap_set_header_content_type(clone->message, old_message->type);

    if (old_message->proxy_uri != NULL) {
        char str[old_message->proxy_uri_len + 1];
        str[old_message->proxy_uri_len] = '\0';
        memcpy(str, old_message->proxy_uri, old_message->proxy_uri_len);
        coap_set_header_proxy_uri(clone->message, str);
    }

    if (IS_OPTION(old_message, COAP_OPTION_ETAG)) {
        coap_set_header_etag(clone->message, old_message->etag, old_message->etag_len);
    }

    if (old_message->uri_host != NULL) {
        char str[old_message->uri_host_len + 1];
        str[old_message->uri_host_len] = '\0';
        memcpy(str, old_message->uri_host, old_message->uri_host_len);
        coap_set_header_uri_host(clone->message, str);
    }

    if (IS_OPTION(old_message, COAP_OPTION_URI_PORT)) {
        coap_set_header_uri_port(clone->message, old_message->uri_port);
    }

    if (IS_OPTION(old_message, COAP_OPTION_LOCATION_PATH)) {
        ((coap_packet_t *)clone->message)->location_path = old_message->location_path;
        SET_OPTION((coap_packet_t *)clone->message, COAP_OPTION_LOCATION_PATH);
    }

    if (old_message->location_query != NULL) {
        char str[old_message->location_query_len + 1];
        str[old_message->location_query_len] = '\0';
        memcpy(str, old_message->location_query, old_message->location_query_len);
        coap_set_header_location_query(clone->message, str);
    }

    if (IS_OPTION(old_message, COAP_OPTION_CONTENT_TYPE)) {
        ((coap_packet_t *)clone->message)->content_type = old_message->content_type;
        SET_OPTION((coap_packet_t *)clone->message, COAP_OPTION_CONTENT_TYPE);
    }

    /* Add the Uri-Path option to the new message if the old already contained this option.
     * It may make sense to check if the new message has the filed uri_path set. */
    if (IS_OPTION(old_message, COAP_OPTION_URI_PATH)) {
        SET_OPTION((coap_packet_t *)clone->message, COAP_OPTION_URI_PATH);
    }

    if (IS_OPTION(old_message, COAP_OPTION_OBSERVE)) {
        coap_set_header_observe(clone->message, old_message->observe);
    }

    for (int i = 0; i < old_message->accept_num; i++) {
        coap_set_header_accept(clone->message, old_message->accept[i]);
    }

    if (IS_OPTION(old_message, COAP_OPTION_IF_MATCH)) {
        coap_set_header_if_match(clone->message, old_message->if_match, old_message->if_match_len);
    }

    if (IS_OPTION(old_message, COAP_OPTION_URI_QUERY)) {
        ((coap_packet_t *)clone->message)->uri_query = old_message->uri_query;
        SET_OPTION((coap_packet_t *)clone->message, COAP_OPTION_URI_QUERY);
    }

    if (IS_OPTION(old_message, COAP_OPTION_IF_NONE_MATCH)) {
        coap_set_header_if_none_match(clone->message);
    }

    uint8_t *cloned_transaction_payload = (uint8_t *)lwm2m_malloc(transaction->payload_len);
    if (cloned_transaction_payload == NULL) {
        return NULL;
    }
    memcpy(cloned_transaction_payload, transaction->payload, transaction->payload_len);

    clone->payload = cloned_transaction_payload;
    clone->payload_len = transaction->payload_len;
    clone->callback = transaction->callback;
    clone->userData = transaction->userData;
    return clone;
}

static lwm2m_transaction_t *transaction_get_new_block1(lwm2m_context_t *contextP, lwm2m_transaction_t *previous,
                                                       uint32_t block_num, uint16_t block_size) {
    lwm2m_transaction_t *next;
    // Done sending block
    if (block_num * (size_t)block_size >= previous->payload_len)
        return NULL;

    next = transaction_create_next_block(previous, contextP->nextMID++);
    if (next == NULL) {
        return NULL;
    }

    size_t remaining_payload_length = next->payload_len - block_num * (size_t)block_size;
    uint8_t *new_block_start = next->payload + block_num * block_size;

    coap_set_header_block1(next->message, block_num, remaining_payload_length > block_size, block_size);
    coap_set_payload(next->message, new_block_start, MIN(block_size, remaining_payload_length));

    contextP->transactionList = (lwm2m_transaction_t *)LWM2M_LIST_ADD(contextP->transactionList, next);
    return next;
}

static lwm2m_transaction_t *transaction_prepare_next_block1_by_mid(lwm2m_context_t *contextP, void *sessionH,
                                                                   uint16_t mid, uint16_t block_size) {
    lwm2m_transaction_t * transaction;
    coap_packet_t * message;
    uint32_t block_num;

    transaction = transaction_get_by_mid(contextP, sessionH, mid);
    if (transaction == NULL) {
        return NULL;
    }

    message = (coap_packet_t *) transaction->message;

    // safeguard, requested block size should not be greater or zero
    if (block_size > message->block1_size || block_size == 0) block_size = message->block1_size;

    if (message->block1_num == 0)
    {
        block_num = message->block1_size / block_size;
    }
    else
    {
        block_num = message->block1_num + 1;
    }

    return transaction_get_new_block1(contextP, transaction, block_num, block_size);
}

static lwm2m_transaction_t *transaction_prepare_next_block1_by_token(lwm2m_context_t *contextP, void *sessionH,
                                                                     uint16_t block_size, uint8_t *token,
                                                                     uint8_t token_len) {
    lwm2m_transaction_t *transaction;
    coap_packet_t *message;
    uint32_t block_num;

    transaction = transaction_get_by_token(contextP, sessionH, token, token_len);
    if (transaction == NULL) {
        return NULL;
    }

    message = (coap_packet_t *)transaction->message;

    // safeguard, requested block size should not be greater or zero
    if (block_size > message->block1_size || block_size == 0)
        block_size = message->block1_size;

    if (message->block1_num == 0) {
        block_num = message->block1_size / block_size;
    } else {
        block_num = message->block1_num + 1;
    }

    return transaction_get_new_block1(contextP, transaction, block_num, block_size);
}

static lwm2m_transaction_t *transaction_change_to_block1(lwm2m_context_t *contextP, void *sessionH, uint16_t mid,
                                                         uint32_t size) {
    lwm2m_transaction_t * transaction;
    uint16_t block_size = 16;

    transaction = transaction_get_by_mid(contextP, sessionH, mid);

    for (uint16_t n = 1; 16 << n <= (uint16_t)size; n++) {
        block_size = 16 << n;
    }

    block_size = MIN(block_size, lwm2m_get_coap_block_size());

    return transaction_get_new_block1(contextP, transaction, 0, block_size);
}

static lwm2m_transaction_t *transaction_retry_block1_by_mid(lwm2m_context_t *contextP, void *sessionH, uint16_t mid,
                                                            uint16_t block_size) {
    lwm2m_transaction_t * transaction;
    coap_packet_t * message;
    uint32_t block_num;

    transaction = transaction_get_by_mid(contextP, sessionH, mid);
    if (transaction == NULL) {
        return NULL;
    }
    message = (coap_packet_t *) transaction->message;
    if(!IS_OPTION(message, COAP_OPTION_BLOCK1)){
        // This wasn't a block option, just switch to block1 transfer with the given size
        return transaction_get_new_block1(contextP, transaction, 0, block_size);
    }

    // safeguard, requested block size should not be greater or zero
    // TODO: Why half the block size?
    if (block_size == message->block1_size && block_size > 16) {
        block_size = block_size >> 1;
    }
    if (block_size >= message->block1_size || block_size == 0) {
        return NULL;
    }
    block_num = message->block1_num;

    return transaction_get_new_block1(contextP, transaction, block_num, block_size);
}

static lwm2m_transaction_t *transaction_retry_block1_by_token(lwm2m_context_t *contextP, void *sessionH, uint16_t mid,
                                                              uint16_t block_size, uint8_t *token, uint8_t token_len) {
    lwm2m_transaction_t *transaction;
    coap_packet_t *message;
    uint32_t block_num;

    transaction = transaction_get_by_token(contextP, sessionH, token, token_len);
    if (transaction == NULL) {
        return NULL;
    }

    message = (coap_packet_t *)transaction->message;
    if (!IS_OPTION(message, COAP_OPTION_BLOCK1)) {
        // This wasn't a block option, just switch to block1 transfer with the given size
        return transaction_get_new_block1(contextP, transaction, 0, block_size);
    }

    // safeguard, requested block size should not be greater or zero
    if (block_size == message->block1_size && block_size > 16) {
        block_size = block_size >> 1;
    }
    if (block_size >= message->block1_size || block_size == 0) {
        return NULL;
    }

    block_num = message->block1_num;

    return transaction_get_new_block1(contextP, transaction, block_num, block_size);
}

static int prv_send_get_block2(lwm2m_context_t * contextP,
                                    void * sessionH,
                                    lwm2m_block_data_t * blockDataHead,
                                    uint16_t currentMID,
                                    uint32_t block2_num,
                                    uint16_t block2_size
                                    )
{
    lwm2m_transaction_t * transaction;
    lwm2m_transaction_t * next;
    uint16_t nextMID;

    // get current transaction
    transaction = transaction_get_by_mid(contextP, sessionH, currentMID);
    if(transaction == NULL) return COAP_500_INTERNAL_SERVER_ERROR;

    // Are we retrying something that already is a block 2 request?
    coap_packet_t * message = transaction->message;
    if (block2_num == 0 && IS_OPTION(message, COAP_OPTION_BLOCK2)) return COAP_IGNORE;

    // create new transaction
    nextMID = contextP->nextMID++;
    next = transaction_create_next_block(transaction, nextMID);
    if (next == NULL) return COAP_500_INTERNAL_SERVER_ERROR;

    // set block2 header
    coap_set_header_block2(next->message, block2_num, 0, MIN(block2_size, lwm2m_get_coap_block_size()));

    //  update block2data to nect expected mid
    coap_block2_set_expected_mid(blockDataHead, currentMID, nextMID);

    contextP->transactionList = (lwm2m_transaction_t *)LWM2M_LIST_ADD(contextP->transactionList, next);
    return transaction_send(contextP, next);
}

static int prv_send_get_next_block2(lwm2m_context_t * contextP,
                                    void * sessionH,
                                    lwm2m_block_data_t * blockDataHead,
                                    uint16_t currentMID,
                                    uint32_t block2_num,
                                    uint16_t block2_size
                                    )
{
    return prv_send_get_block2(contextP, sessionH, blockDataHead, currentMID, block2_num + 1, block2_size);
}

static bool is_message_too_large(const coap_packet_t *message, const size_t packet_size) {
    if (IS_OPTION(message, COAP_OPTION_BLOCK1) || IS_OPTION(message, COAP_OPTION_BLOCK2)) {
        /* In case of block-transfer the *payload* mustn't be bigger than the block size. */
        return message->payload_len > lwm2m_get_coap_block_size();
    }
    /* In case of a normal message (not block-transfer) the *complete packet* mustn't be bigger than the packet size. */
    return packet_size > LWM2M_COAP_MAX_MESSAGE_SIZE;
}

/* This function is an adaptation of function coap_receive() from Erbium's er-coap-13-engine.c.
 * Erbium is Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 */
void lwm2m_handle_packet(lwm2m_context_t *contextP, uint8_t *buffer, size_t length, void *fromSessionH) {
    uint8_t coap_error_code = NO_ERROR;
    static coap_packet_t message[1];
    static coap_packet_t response[1];

    LOG_DBG("Entering");
    /* The buffer length is uint16_t here, as UDP packet length field is 16 bit.
     * This might change in the future e.g. for supporting TCP or other transport.
     */
    coap_error_code = coap_parse_message(message, buffer, (uint16_t)length);
    if (coap_error_code == NO_ERROR)
    {
        LOG_ARG_DBG("Parsed: ver %u, type %u, tkl %u, code %u.%.2u, mid %u, Content type: %d", message->version,
                    message->type, message->token_len, message->code >> 5, message->code & 0x1F, message->mid,
                    message->content_type);
        LOG_ARG_DBG("Payload: %.*s", (int)message->payload_len, STR_NULL2EMPTY(message->payload));
        if (message->code >= COAP_GET && message->code <= COAP_DELETE)
        {
            uint32_t block_num = 0;
            uint16_t block_size = lwm2m_get_coap_block_size();
            uint32_t block_offset = 0;

            /* prepare response */
            if (message->type == COAP_TYPE_CON)
            {
                /* Reliable CON requests are answered with an ACK. */
                coap_init_message(response, COAP_TYPE_ACK, COAP_205_CONTENT, message->mid);
            }
            else
            {
                /* Unreliable NON requests are answered with a NON as well. */
                coap_init_message(response, COAP_TYPE_NON, COAP_205_CONTENT, contextP->nextMID++);
            }

            /* mirror token */
            if (message->token_len)
            {
                coap_set_header_token(response, message->token, message->token_len);
            }

            if (is_message_too_large(message, length)) {
                coap_error_code = COAP_413_ENTITY_TOO_LARGE;

                if (IS_OPTION(message, COAP_OPTION_BLOCK1)){
                    uint32_t block1_num;
                    uint8_t  block1_more;
                    coap_get_header_block1(message, &block1_num, &block1_more, NULL, NULL);
                    coap_set_header_block1(response, block1_num, block1_more, lwm2m_get_coap_block_size());
                } else {
                    coap_set_header_block1(response, 0, 1, lwm2m_get_coap_block_size());
                }
            } else if (IS_OPTION(message, COAP_OPTION_BLOCK1)) {
#ifdef LWM2M_CLIENT_MODE
                // get server
                lwm2m_server_t * peerP;
                peerP = utils_findServer(contextP, fromSessionH);
#ifdef LWM2M_BOOTSTRAP
                if (peerP == NULL)
                {
                    peerP = utils_findBootstrapServer(contextP, fromSessionH);
                }
#endif
#else
                lwm2m_client_t * peerP;
                multi_option_t * uriPath = message->uri_path;
                bool isRegistration = NULL != uriPath && URI_REGISTRATION_SEGMENT_LEN == uriPath->len && 0 == strncmp(URI_REGISTRATION_SEGMENT, (char *)uriPath->data, uriPath->len);
                peerP = utils_findClient(contextP, fromSessionH);
                if (peerP == NULL && isRegistration)
                {
                    peerP = (lwm2m_client_t *)lwm2m_malloc(sizeof(lwm2m_client_t));

                    if (peerP != NULL)
                    {
                        memset(peerP, 0, sizeof(lwm2m_client_t));
                        peerP->lifetime = LWM2M_DEFAULT_LIFETIME;
                        peerP->endOfLife = lwm2m_gettime() + LWM2M_DEFAULT_LIFETIME;
                        peerP->sessionH = fromSessionH;
                        peerP->internalID = lwm2m_list_newId((lwm2m_list_t *)contextP->clientList);
                        contextP->clientList = (lwm2m_client_t *)LWM2M_LIST_ADD(contextP->clientList, peerP);
                    }
                }
#endif
                if (peerP == NULL)
                {
                    coap_error_code = COAP_500_INTERNAL_SERVER_ERROR;
                }
                else
                {
                    uint32_t block1_num;
                    uint8_t  block1_more;
                    uint16_t block1_size;
                    uint8_t * complete_buffer = NULL;
                    size_t complete_buffer_size;

                    // parse block1 header
                    coap_get_header_block1(message, &block1_num, &block1_more, &block1_size, NULL);
                    LOG_ARG_DBG("Blockwise: block1 request NUM %u (SZX %u/ SZX Max%u) MORE %u", block1_num, block1_size,
                                lwm2m_get_coap_block_size(), block1_more);

                    char * uri = coap_get_packet_uri_as_string(message);
                    if (uri == NULL){
                        coap_error_code = COAP_500_INTERNAL_SERVER_ERROR;
                    } else {
                    // handle block 1
#ifdef LWM2M_RAW_BLOCK1_REQUESTS
                        coap_error_code = coap_block1_handler(&peerP->blockData, uri, message->mid, message->payload, message->payload_len, block1_size, block1_num, block1_more, &complete_buffer, &complete_buffer_size);
#else
                        coap_error_code = coap_block1_handler(&peerP->blockData, uri, message->payload, message->payload_len, block1_size, block1_num, block1_more, &complete_buffer, &complete_buffer_size);
#endif
                        lwm2m_free(uri);
                    }
#ifndef LWM2M_RAW_BLOCK1_REQUESTS
                    // if payload is complete, replace it in the coap message.
                    if (coap_error_code == NO_ERROR)
                    {
                        message->payload = complete_buffer;
                        message->payload_len = complete_buffer_size;
                    }
#endif
                    block1_size = MIN(block1_size, lwm2m_get_coap_block_size());
                    coap_set_header_block1(response, block1_num, block1_more, block1_size);
                }
            }
#ifdef LWM2M_RAW_BLOCK1_REQUESTS
            if (coap_error_code == NO_ERROR || coap_error_code == COAP_231_CONTINUE )
#else
            if (coap_error_code == NO_ERROR)
#endif
            {
                coap_error_code = handle_request(contextP, fromSessionH, message, response);
            }
            if (coap_error_code == NO_ERROR)
            {
                /* Save original payload pointer for later freeing. Payload in response may be updated. */
                uint8_t *payload = response->payload;
                if ( IS_OPTION(message, COAP_OPTION_BLOCK2) )
                {
                    /* get offset for blockwise transfers */
                    if (coap_get_header_block2(message, &block_num, NULL, &block_size, &block_offset))
                    {
                        LOG_ARG_DBG("Blockwise: block request %u (%u/%u) @ %u bytes", block_num, block_size,
                                    lwm2m_get_coap_block_size(), block_offset);
                        block_size = MIN(block_size, lwm2m_get_coap_block_size());
                    }

                    if (block_offset >= response->payload_len)
                    {
                        LOG_DBG("handle_incoming_data(): block_offset >= response->payload_len");

                        response->code = COAP_402_BAD_OPTION;
                        coap_set_payload(response, "BlockOutOfScope", 15); /* a const char str[] and sizeof(str) produces larger code size */
                    }
                    else
                    {
                        coap_set_header_block2(response, block_num, response->payload_len - block_offset > block_size, block_size);
                        coap_set_payload(response, response->payload+block_offset, MIN(response->payload_len - block_offset, block_size));
                    } /* if (valid offset) */
                } else if (response->payload_len > lwm2m_get_coap_block_size()) {
                    coap_set_header_block2(response, 0, response->payload_len > lwm2m_get_coap_block_size(),
                                           lwm2m_get_coap_block_size());
                    coap_set_payload(response, response->payload, lwm2m_get_coap_block_size());
                }

                coap_error_code = message_send(contextP, response, fromSessionH);

                lwm2m_free(payload);
                response->payload = NULL;
                response->payload_len = 0;
            }
            else if (coap_error_code != COAP_IGNORE)
            {
                if (coap_error_code == COAP_RETRANSMISSION) {
                    if (IS_OPTION(response, COAP_OPTION_BLOCK1)) {
                        uint32_t block1_num;
                        uint8_t block1_more;
                        uint16_t block1_size;
                        // parse block1 header
                        coap_get_header_block1(response, &block1_num, &block1_more, &block1_size, NULL);
                        if (block1_more) {
                            coap_error_code = COAP_231_CONTINUE;
                        } else {
                            coap_error_code = COAP_204_CHANGED;
                        }
                    }
                }
                if (1 == coap_set_status_code(response, coap_error_code))
                {
                    coap_error_code = message_send(contextP, response, fromSessionH);
                }
            }
        }
        else
        {
            /* Responses */
            switch (message->type)
            {
            case COAP_TYPE_NON:
            case COAP_TYPE_CON:
                if (message->payload_len > lwm2m_get_coap_block_size()) {
#ifdef LWM2M_CLIENT_MODE
                    // get server
                    lwm2m_server_t * peerP;
                    peerP = utils_findServer(contextP, fromSessionH);
#ifdef LWM2M_BOOTSTRAP
                    if (peerP == NULL)
                    {
                        peerP = utils_findBootstrapServer(contextP, fromSessionH);
                    }
#endif
#else
                    lwm2m_client_t * peerP;
                    peerP = utils_findClient(contextP, fromSessionH);
#endif
                    if (peerP != NULL)
                    {
                        // retry as a block2 request
                        prv_send_get_block2(contextP, fromSessionH, peerP->blockData, message->mid, 0,
                                            lwm2m_get_coap_block_size());
                    }
                    transaction_handleResponse(contextP, fromSessionH, message, NULL);
                } else {
                    lwm2m_transaction_t *next_block_transaction = NULL;
                    // check if next block should be sent
                    if (IS_OPTION(message, COAP_OPTION_BLOCK1)) {
                        uint32_t block_num;
                        uint16_t block_size;

                        coap_get_header_block1(message, &block_num, NULL, &block_size, NULL);

                        switch (message->code) {
                        case COAP_201_CREATED:
                        case COAP_204_CHANGED:
                        case COAP_231_CONTINUE:
                            next_block_transaction = transaction_prepare_next_block1_by_token(
                                contextP, fromSessionH, block_size, message->token, message->token_len);
                            break;
                        case COAP_413_ENTITY_TOO_LARGE:
                            // resend with smaller block size as specified in the block 1 option give by the peer
                            if (block_num > 0)
                                break;
                            transaction_retry_block1_by_token(contextP, fromSessionH, message->mid, block_size,
                                                              message->token, message->token_len);
                        default:
                            break;
                        }
                    }
                    // send ack, this also removes the transaction
                    bool done = transaction_handleResponse(contextP, fromSessionH, message, response);
                    // send next block if needed
                    if (done && next_block_transaction != NULL) {
                        transaction_send(contextP, next_block_transaction);
                    }
#ifdef LWM2M_SERVER_MODE
                    if (!done && IS_OPTION(message, COAP_OPTION_OBSERVE) &&
                        ((message->code == COAP_204_CHANGED) || (message->code == COAP_205_CONTENT)))
                    {
                        done = observe_handleNotify(contextP, fromSessionH, message, response);
                    }
#endif
                    if (!done && message->type == COAP_TYPE_CON )
                    {
                        coap_init_message(response, COAP_TYPE_ACK, 0, message->mid);
                        if (is_message_too_large(message, length)) {
                            coap_set_status_code(response, COAP_413_ENTITY_TOO_LARGE);
                        }
                        coap_error_code = message_send(contextP, response, fromSessionH);
                    }
                }
                break;

            case COAP_TYPE_RST:
                /* Cancel possible subscriptions. */
                handle_reset(contextP, fromSessionH, message);
                transaction_handleResponse(contextP, fromSessionH, message, NULL);
                break;

            case COAP_TYPE_ACK:
                if (message->payload_len > lwm2m_get_coap_block_size()) {
#ifdef LWM2M_CLIENT_MODE
                    // get server
                    lwm2m_server_t * peerP;
                    peerP = utils_findServer(contextP, fromSessionH);
#ifdef LWM2M_BOOTSTRAP
                    if (peerP == NULL)
                    {
                        peerP = utils_findBootstrapServer(contextP, fromSessionH);
                    }
#endif
#else
                    lwm2m_client_t * peerP;
                    peerP = utils_findClient(contextP, fromSessionH);
#endif
                    if (peerP != NULL)
                    {
                        // retry as a block2 request
                        prv_send_get_block2(contextP, fromSessionH, peerP->blockData, message->mid, 0,
                                            lwm2m_get_coap_block_size());
                    }
                    transaction_handleResponse(contextP, fromSessionH, message, NULL);
                } else if (IS_OPTION(message, COAP_OPTION_BLOCK1)) {
                    lwm2m_transaction_t *next_block_transaction = NULL;
                    uint32_t block_num;
                    uint16_t block_size;

                    coap_get_header_block1(message, &block_num, NULL, &block_size, NULL);

                    switch (message->code) {
                        case COAP_201_CREATED:
                        case COAP_204_CHANGED:
                        case COAP_231_CONTINUE:
                            next_block_transaction = transaction_prepare_next_block1_by_mid(contextP, fromSessionH,
                                                                                            message->mid, block_size);
                            break;
                        case COAP_413_ENTITY_TOO_LARGE:
                            // resend with smaller block size as specified in the block 1 option
                            if (block_num > 0) {
                                break;
                            }
                            next_block_transaction =
                                transaction_retry_block1_by_mid(contextP, fromSessionH, message->mid, block_size);
                        default:
                            break;
                    }
                    if (next_block_transaction != NULL) {
                        transaction_send(contextP, next_block_transaction);
                    }

                    transaction_handleResponse(contextP, fromSessionH, message, NULL);
                } else if (IS_OPTION(message, COAP_OPTION_BLOCK2)) {
#ifdef LWM2M_CLIENT_MODE
                    // get server
                    lwm2m_server_t * peerP;
                    peerP = utils_findServer(contextP, fromSessionH);
#ifdef LWM2M_BOOTSTRAP
                    if (peerP == NULL)
                    {
                        peerP = utils_findBootstrapServer(contextP, fromSessionH);
                    }
#endif
#else
                    lwm2m_client_t * peerP;
                    peerP = utils_findClient(contextP, fromSessionH);
#endif

                    if (peerP == NULL)
                    {
                        coap_error_code = COAP_500_INTERNAL_SERVER_ERROR;
                    }
                    else
                    {
                        uint32_t block2_num;
                        uint8_t  block2_more;
                        uint16_t block2_size;
                        uint8_t * complete_buffer = NULL;
                        size_t complete_buffer_size;

                        // parse block2 header
                        coap_get_header_block2(message, &block2_num, &block2_more, &block2_size, NULL);
                        LOG_ARG_DBG("Blockwise: block2 response NUM %u (SZX %u/ SZX Max%u) MORE %u", block2_num,
                                    block2_size, lwm2m_get_coap_block_size(), block2_more);

                        // handle block 2
                        coap_error_code = coap_block2_handler(&peerP->blockData, message->mid, message->payload, message->payload_len, block2_size, block2_num, block2_more, &complete_buffer, &complete_buffer_size);

                        // if payload is complete, replace it in the coap message.
                        if (coap_error_code == NO_ERROR)
                        {
                            message->payload = complete_buffer;
                            message->payload_len = complete_buffer_size;
                            transaction_handleResponse(contextP, fromSessionH, message, NULL);
                            block2_delete(&peerP->blockData, message->mid);
                        }
                        else if (coap_error_code == COAP_231_CONTINUE)
                        {
                            prv_send_get_next_block2(contextP, fromSessionH, peerP->blockData, message->mid, block2_num, block2_size);
                            transaction_handleResponse(contextP, fromSessionH, message, NULL);
                            coap_error_code = NO_ERROR;
                        }
                    }
                } else if (message->code == COAP_413_ENTITY_TOO_LARGE) {
                    /*
                    All our responses are piggyback so this must be the result of request being too large (not a separate CON)
                    switch to a block1 request.
                    */
                    transaction_change_to_block1(contextP, fromSessionH, message->mid, message->size);
                    transaction_handleResponse(contextP, fromSessionH, message, NULL);
                } else {
                    transaction_handleResponse(contextP, fromSessionH, message, NULL);
                }
                break;

            default:
                break;
            }
        } /* Request or Response */
        coap_free_header(message);
    } /* if (parsed correctly) */
    else
    {
        LOG_ARG_DBG("Message parsing failed %u.%02u", coap_error_code >> 5, coap_error_code & 0x1F);
    }

    if (coap_error_code != NO_ERROR && coap_error_code != COAP_IGNORE)
    {
        LOG_ARG_DBG("ERROR %u: %s", coap_error_code, STR_NULL2EMPTY(coap_error_message));

        /* Set to sendable error code. */
        if (coap_error_code >= 192)
        {
            coap_error_code = COAP_500_INTERNAL_SERVER_ERROR;
        }
        /* Reuse input buffer for error message. */
        coap_init_message(message, COAP_TYPE_ACK, coap_error_code, message->mid);
        coap_set_payload(message, coap_error_message, strlen(coap_error_message));
        message_send(contextP, message, fromSessionH);
    }
}

uint8_t message_send(lwm2m_context_t * contextP,
                     coap_packet_t * message,
                     void * sessionH)
{
    uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;
    uint8_t * pktBuffer;
    size_t pktBufferLen = 0;
    size_t allocLen;

    LOG_DBG("Entering");
    allocLen = coap_serialize_get_size(message);
    LOG_ARG_DBG("Size to allocate: %zd", allocLen);
    if (allocLen == 0) return COAP_500_INTERNAL_SERVER_ERROR;

    pktBuffer = (uint8_t *)lwm2m_malloc(allocLen);
    if (pktBuffer != NULL)
    {
        pktBufferLen = coap_serialize_message(message, pktBuffer);
        LOG_ARG_DBG("coap_serialize_message() returned %zd", pktBufferLen);
        if (0 != pktBufferLen)
        {
            result = lwm2m_buffer_send(sessionH, pktBuffer, pktBufferLen, contextP->userData);
        }
        lwm2m_free(pktBuffer);
    }

    return result;
}
