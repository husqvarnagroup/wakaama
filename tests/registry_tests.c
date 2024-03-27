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

enum lwm2m_resource_operations_t {
    LWM2M_RESOURCES_OPERATIONS_READ,
    LWM2M_RESOURCES_OPERATIONS_WRITE,
    LWM2M_RESOURCES_OPERATIONS_READ_WRITE,
    LWM2M_RESOURCES_OPERATIONS_EXECUTE,
    LWM2M_RESOURCES_OPERATIONS_NONE,
};


typedef struct _lwm2m_object_definition_resource_
{
    struct _lwm2m_object_definition_resource_ * next;  // matches lwm2m_list_t::next
    uint16_t               id;    // matches lwm2m_list_t::id
    char *     name;
    enum lwm2m_resource_operations_t operations;
    bool multiInst;
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

typedef struct _lwm2m_object_definition_ {
    struct _lwm2m_object_definition_ * next;  // matches lwm2m_list_t::next
    uint16_t               obj_id;    // matches lwm2m_list_t::id
    char * name;
    char * urn;
    uint8_t        lwm2mVersionMajor;
    uint8_t        lwm2mVersionMinor;
    uint8_t        objectVersionMajor;
    uint8_t        objectVersionMinor;
    bool multipleInstances;
    bool            mandatory;
    lwm2m_object_definition_resource_t * resources;
} lwm2m_object_definition_t;

void free_object_definition(lwm2m_object_definition_t *def) {
    lwm2m_object_definition_t * next = def;
    while (next != NULL) {
        lwm2m_free(next->name);
        lwm2m_free(next->urn);
        //lwm2m_object_resources_free(next->resources);
        next = next->next;
    }
    LWM2M_LIST_FREE(def);
}


static inline char* malloc_str(const char* const str) {
    char* str_on_heap = (char*) lwm2m_malloc(strlen(str) + 1);
    strcpy(str_on_heap, str);
    return str_on_heap;
}

static void create_security_object(lwm2m_object_definition_t* sec_obj) {
    memset(sec_obj, 0, sizeof(lwm2m_object_definition_t));
    sec_obj->name = malloc_str("LWM2M Security");
    sec_obj->obj_id = 0;
    sec_obj->urn = malloc_str("urn:oma:lwm2m:oma:0:1.1");
    sec_obj->lwm2mVersionMajor = 1;
    sec_obj->lwm2mVersionMinor = 1;
    sec_obj->objectVersionMajor = 1;
    sec_obj->objectVersionMinor = 1;
    sec_obj->multipleInstances = false;
    sec_obj->mandatory = true;

    /* Resources */
    lwm2m_object_definition_resource_t* res0 = lwm2m_malloc(sizeof(lwm2m_object_definition_resource_t));
    memset(res0, 0, sizeof(lwm2m_object_definition_resource_t));
    res0->id = 0;
    res0->name = malloc_str("LWM2M Server URI");
    res0->multiInst = false;
    res0->mandatory = true;
    res0->type = LWM2M_TYPE_STRING;


    LWM2M_LIST_ADD(sec_obj->resources, res0);

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
