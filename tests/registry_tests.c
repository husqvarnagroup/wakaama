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

static void create_security_object(lwm2m_object_definition_t* sec_obj) {
    lwm2m_registry_init_object(sec_obj,
                      0,
                      "LWM2M Security",
                      "urn:oma:lwm2m:oma:0:1.1",
                      VERSION_1_1,
                      1,
                      1,
                      false,
                      true);

    /* Resources */
    lwm2m_registry_add_object_resource(sec_obj,
                              0,
                              "LWM2M Server URI",
                              LWM2M_RESOURCES_OPERATIONS_WRITE,
                              false,
                              true,
                              LWM2M_TYPE_STRING);
}


typedef struct _lwm2m_object_registry_ {
    struct _lwm2m_object_registry_ *next; // matches lwm2m_list_t::next
    uint16_t id;                          // matches lwm2m_list_t::id
    lwm2m_object_definition_t *registry;
} lwm2m_object_registry_t;


static lwm2m_object_registry_t* object_registries = NULL;

void lwm2m_add_registry(lwm2m_object_registry_t *registry) {
    uint16_t id = lwm2m_list_newId((lwm2m_list_t *)object_registries);
    registry->id = id;

    LWM2M_LIST_ADD(object_registries, registry);
}

static void test_registry(void) {
    lwm2m_object_definition_t *sec_obj = lwm2m_malloc(sizeof (lwm2m_object_definition_t));
    create_security_object(sec_obj);
    lwm2m_registry_free_object_definition(sec_obj);
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
