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

#include "liblwm2m.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


void lwm2m_object_resources_free(lwm2m_object_definition_resource_t *res) {
    lwm2m_object_definition_resource_t * next = res;
    while (next != NULL) {
        lwm2m_free(next->name);
        next = next->next;
    }
    LWM2M_LIST_FREE(res);
}

void lwm2m_registry_free_object_definition(lwm2m_object_definition_t *def) {
    lwm2m_object_definition_t * next = def;
    while (next != NULL) {
        lwm2m_free(next->name);
        lwm2m_free(next->object_urn);
        lwm2m_object_resources_free(next->resources);
        next = next->next;
    }
    LWM2M_LIST_FREE(def);
}


static inline char* malloc_str(const char* const str) {
    char* str_on_heap = (char*) lwm2m_malloc(strlen(str) + 1);
    strcpy(str_on_heap, str);
    return str_on_heap;
}

void lwm2m_registry_init_object(lwm2m_object_definition_t* obj,
                       const uint16_t obj_id,
                       const char * const name,
                       const char * const urn,
                       const lwm2m_version_t lwm2m_version,
                       const uint8_t object_version_major,
                       const uint8_t object_version_minor,
                       const bool multi_instances,
                       const bool mandatory) {
    memset(obj, 0, sizeof(lwm2m_object_definition_t));
    obj->obj_id = obj_id;
    obj->name = malloc_str(name);
    obj->object_urn = malloc_str(urn);
    obj->lwm2m_version = lwm2m_version;
    obj->object_version.major = object_version_major;
    obj->object_version.minor = object_version_minor;
    obj->multi_instances = multi_instances;
    obj->mandatory = mandatory;
}

void lwm2m_registry_add_object_resource(lwm2m_object_definition_t* obj,
                               const uint16_t id,
                               const char * const name,
                               const lwm2m_resource_operations_t operations,
                               const bool multi_inst,
                               const bool mandatory,
                               const lwm2m_data_type_t type) {

    lwm2m_object_definition_resource_t* res = lwm2m_malloc(sizeof(lwm2m_object_definition_resource_t));
    memset(res, 0, sizeof(lwm2m_object_definition_resource_t));
    res->id = id;
    res->name = malloc_str(name);
    res->operations = operations;
    res->multi_inst = multi_inst;
    res->mandatory = mandatory;
    res->type = type;

    if (obj->resources == NULL) {
        obj->resources = res;
    }
    else {
        LWM2M_LIST_ADD(obj->resources, res);
    }
}

