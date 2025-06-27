/*******************************************************************************
 *
 * Copyright (c) 2015 Bosch Software Innovations GmbH, Germany.
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
 *    Bosch Software Innovations GmbH - Please refer to git log
 *
 *******************************************************************************/

#ifndef TESTS_H_
#define TESTS_H_

#include "CUnit/CUError.h"
/* Needed for preprocessor defines */
#include "internals.h"

struct TestTable {
    const char* name;
    CU_TestFunc function;
};



CU_ErrorCode add_tests(CU_pSuite pSuite, struct TestTable* testTable);

CU_ErrorCode create_cbor_suit(void);

#endif /* TESTS_H_ */
