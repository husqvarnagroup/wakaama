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

#include "CUnit/CUnit.h"
#include "liblwm2m.h"
#include "tests.h"

typedef enum _lwm2m_resource_operations_ {
    LWM2M_RESOURCES_OPERATIONS_READ,
    LWM2M_RESOURCES_OPERATIONS_WRITE,
    LWM2M_RESOURCES_OPERATIONS_READ_WRITE,
    LWM2M_RESOURCES_OPERATIONS_EXECUTE,
    LWM2M_RESOURCES_OPERATIONS_NONE,
} lwm2m_resource_operations_t;


typedef struct _lwm2m_object_definition_resource_
{
    struct _lwm2m_object_definition_resource_ * next;  // matches lwm2m_list_t::next
    uint16_t               id;    // matches lwm2m_list_t::id
    char *     name;
    lwm2m_resource_operations_t operations;
    bool multi_inst;
    bool mandatory;
    lwm2m_data_type_t type;
} lwm2m_object_definition_resource_t;

void lwm2m_object_resources_free(lwm2m_object_definition_resource_t *res) {
    lwm2m_object_definition_resource_t * next = res;
    while (next != NULL) {
        lwm2m_free(next->name);
        next = next->next;
    }
    LWM2M_LIST_FREE(res);
}

typedef struct _lwm2m_object_version_ {
    uint8_t        major;
    uint8_t        minor;
} lwm2m_object_version_t;

typedef struct _lwm2m_object_definition_ {
    struct _lwm2m_object_definition_ * next;  // matches lwm2m_list_t::next
    uint16_t               obj_id;    // matches lwm2m_list_t::id
    char * name;
    char * urn;
    lwm2m_version_t  lwm2m_version;
    lwm2m_object_version_t object_version;
    bool multi_instances;
    bool            mandatory;
    lwm2m_object_definition_resource_t * resources;
} lwm2m_object_definition_t;

void free_object_definition(lwm2m_object_definition_t *def) {
    lwm2m_object_definition_t * next = def;
    while (next != NULL) {
        lwm2m_free(next->name);
        lwm2m_free(next->urn);
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

void lwm2m_init_object(lwm2m_object_definition_t* obj,
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
    obj->urn = malloc_str(urn);
    obj->lwm2m_version = lwm2m_version;
    obj->object_version.major = object_version_major;
    obj->object_version.minor = object_version_minor;
    obj->multi_instances = multi_instances;
    obj->mandatory = mandatory;
}

void lwm2m_add_object_resource(lwm2m_object_definition_t* obj,
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


/* testing */
static void create_security_object(lwm2m_object_definition_t* sec_obj) {
    lwm2m_init_object(sec_obj,
                      0,
                      "LWM2M Security",
                      "urn:oma:lwm2m:oma:0:1.1",
                      VERSION_1_1,
                      1,
                      1,
                      false,
                      true);

    /* Resources */
    lwm2m_add_object_resource(sec_obj,
                              0,
                              "LWM2M Server URI",
                              LWM2M_RESOURCES_OPERATIONS_WRITE,
                              false,
                              true,
                              LWM2M_TYPE_STRING);
    }

static void test_registry(void) {
    lwm2m_object_definition_t *sec_obj = lwm2m_malloc(sizeof (lwm2m_object_definition_t));
    create_security_object(sec_obj);
    free_object_definition(sec_obj);
}


static struct TestTable table[] = {
    {"test_registry", test_registry},
    {NULL, NULL},
};

CU_ErrorCode create_registry_test_suit(void) {
    CU_pSuite pSuite = NULL;

    pSuite = CU_add_suite("Suite_registry", NULL, NULL);
    if (NULL == pSuite) {
        return CU_get_error();
    }

    return add_tests(pSuite, table);
}
