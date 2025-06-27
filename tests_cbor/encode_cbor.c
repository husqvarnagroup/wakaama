#include "internals.h"
#include "liblwm2m.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commandline.h"

#include "CUnit/Basic.h"
#include "../tests/tests.h"

static void cbor_encode_three_byte_unsigned(void) {
    lwm2m_data_t dataP = { .type = LWM2M_TYPE_UNSIGNED_INTEGER, .value.asUnsigned = 65535 };

    uint8_t buffer[10] = { 0 };
    int ret = cbor_put_singular(buffer, sizeof(buffer), &dataP);
    CU_ASSERT_EQUAL(ret, 0);
}
static struct TestTable table[] = {
    {"cbor_encode_three_byte_unsigned", cbor_encode_three_byte_unsigned}, {NULL, NULL},
};

CU_ErrorCode create_cbor_suit(void) {
    CU_pSuite pSuite = NULL;

    pSuite = CU_add_suite("Suite_CBOR", NULL, NULL);
    if (NULL == pSuite) {
        return CU_get_error();
    }

    return add_tests(pSuite, table);
}
