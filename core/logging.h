/*******************************************************************************
*
* Copyright (c) 2023 Gardena GmbH and others.
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
*    Lukas Woodtli, Gardena GmbH. - Please refer to git log
*
*******************************************************************************/

#ifndef WAKAAMA_LOGGING_H
#define WAKAAMA_LOGGING_H


#ifdef LWM2M_WITH_LOGS
#include <inttypes.h>
#define LOG(STR) lwm2m_printf("[%s:%d] " STR "\r\n", __func__ , __LINE__)
#define LOG_ARG(FMT, ...) lwm2m_printf("[%s:%d] " FMT "\r\n", __func__ , __LINE__ , __VA_ARGS__)
#ifdef LWM2M_VERSION_1_0
#define LOG_URI(URI)                                                                \
{                                                                                   \
    if ((URI) == NULL) lwm2m_printf("[%s:%d] NULL\r\n", __func__ , __LINE__);       \
    else                                                                            \
    {                                                                               \
        lwm2m_printf("[%s:%d] /%d", __func__ , __LINE__ , (URI)->objectId);         \
        if (LWM2M_URI_IS_SET_INSTANCE(URI)) lwm2m_printf("/%d", (URI)->instanceId); \
        if (LWM2M_URI_IS_SET_RESOURCE(URI)) lwm2m_printf("/%d", (URI)->resourceId); \
        lwm2m_printf("\r\n");                                                       \
    }                                                                               \
}
#else
#define LOG_URI(URI)                                                                \
    if ((URI) == NULL) lwm2m_printf("[%s:%d] NULL\r\n", __func__ , __LINE__);       \
    else if (!LWM2M_URI_IS_SET_OBJECT(URI)) lwm2m_printf("[%s:%d] /\r\n", __func__ , __LINE__); \
    else if (!LWM2M_URI_IS_SET_INSTANCE(URI)) lwm2m_printf("[%s:%d] /%d\r\n", __func__ , __LINE__, (URI)->objectId); \
    else if (!LWM2M_URI_IS_SET_RESOURCE(URI)) lwm2m_printf("[%s:%d] /%d/%d\r\n", __func__ , __LINE__, (URI)->objectId, (URI)->instanceId); \
    else if (!LWM2M_URI_IS_SET_RESOURCE_INSTANCE(URI)) lwm2m_printf("[%s:%d] /%d/%d/%d\r\n", __func__ , __LINE__, (URI)->objectId, (URI)->instanceId, (URI)->resourceId); \
    else lwm2m_printf("[%s:%d] /%d/%d/%d/%d\r\n", __func__ , __LINE__, (URI)->objectId, (URI)->instanceId, (URI)->resourceId, (URI)->resourceInstanceId)
#endif
#define STR_STATUS(S)                                           \
((S) == STATE_DEREGISTERED ? "STATE_DEREGISTERED" :             \
((S) == STATE_REG_HOLD_OFF ? "STATE_REG_HOLD_OFF" :             \
((S) == STATE_REG_PENDING ? "STATE_REG_PENDING" :               \
((S) == STATE_REGISTERED ? "STATE_REGISTERED" :                 \
((S) == STATE_REG_FAILED ? "STATE_REG_FAILED" :                 \
((S) == STATE_REG_UPDATE_PENDING ? "STATE_REG_UPDATE_PENDING" : \
((S) == STATE_REG_UPDATE_NEEDED ? "STATE_REG_UPDATE_NEEDED" :   \
((S) == STATE_REG_FULL_UPDATE_NEEDED ? "STATE_REG_FULL_UPDATE_NEEDED" :   \
((S) == STATE_DEREG_PENDING ? "STATE_DEREG_PENDING" :           \
((S) == STATE_BS_HOLD_OFF ? "STATE_BS_HOLD_OFF" :               \
((S) == STATE_BS_INITIATED ? "STATE_BS_INITIATED" :             \
((S) == STATE_BS_PENDING ? "STATE_BS_PENDING" :                 \
((S) == STATE_BS_FINISHED ? "STATE_BS_FINISHED" :               \
((S) == STATE_BS_FINISHING ? "STATE_BS_FINISHING" :             \
((S) == STATE_BS_FAILING ? "STATE_BS_FAILING" :                 \
((S) == STATE_BS_FAILED ? "STATE_BS_FAILED" :                   \
"Unknown"))))))))))))))))
#define STR_MEDIA_TYPE(M)                                        \
((M) == LWM2M_CONTENT_TEXT ? "LWM2M_CONTENT_TEXT" :              \
((M) == LWM2M_CONTENT_LINK ? "LWM2M_CONTENT_LINK" :              \
((M) == LWM2M_CONTENT_OPAQUE ? "LWM2M_CONTENT_OPAQUE" :          \
((M) == LWM2M_CONTENT_TLV ? "LWM2M_CONTENT_TLV" :                \
((M) == LWM2M_CONTENT_JSON ? "LWM2M_CONTENT_JSON" :              \
((M) == LWM2M_CONTENT_SENML_JSON ? "LWM2M_CONTENT_SENML_JSON" :  \
"Unknown"))))))
#define STR_STATE(S)                                \
((S) == STATE_INITIAL ? "STATE_INITIAL" :      \
((S) == STATE_BOOTSTRAP_REQUIRED ? "STATE_BOOTSTRAP_REQUIRED" :      \
((S) == STATE_BOOTSTRAPPING ? "STATE_BOOTSTRAPPING" :  \
((S) == STATE_REGISTER_REQUIRED ? "STATE_REGISTER_REQUIRED" :        \
((S) == STATE_REGISTERING ? "STATE_REGISTERING" :      \
((S) == STATE_READY ? "STATE_READY" :      \
"Unknown"))))))
#define STR_NULL2EMPTY(S) ((const char *)(S) ? (const char *)(S) : "")
#else
#define LOG_ARG(FMT, ...)
#define LOG(STR)
#define LOG_URI(URI)
#endif

#endif // WAKAAMA_LOGGING_H
