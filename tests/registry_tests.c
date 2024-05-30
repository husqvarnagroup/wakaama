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

// TODO remove!
lwm2m_object_definition_list_t* lwm2m_registry_initialize(void);

static void test_security_object(void) {
    lwm2m_object_definition_list_t *list = lwm2m_registry_initialize();
    lwm2m_object_definition_t* sec_obj = list->object;


    CU_ASSERT_STRING_EQUAL(sec_obj->name, "LWM2M Security");
    CU_ASSERT_EQUAL(sec_obj->obj_id, 0);
    lwm2m_object_version_t obj_version = sec_obj->object_version;
    CU_ASSERT_EQUAL(obj_version.major, 1);
    CU_ASSERT_EQUAL(obj_version.minor, 1);
    CU_ASSERT_EQUAL(sec_obj->lwm2m_version, VERSION_1_1);

    lwm2m_registry_free_object_definitions(list);
}


static struct TestTable table[] = {
    {"test_security_object", test_security_object},
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
