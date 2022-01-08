#include "er-coap-13/er-coap-13.h"

#include "tests.h"

static void test_regression1(void) {
    uint8_t data[] = {'\x6e', '\x8d'};

    coap_packet_t coap_pkt;
    CU_ASSERT_EQUAL(BAD_REQUEST_4_00, coap_parse_message(&coap_pkt, data, sizeof(data)))
}

static struct TestTable table[] = {
    {"Regression #1:", test_regression1},
    {NULL, NULL},
};

CU_ErrorCode create_er_coap_parse_message_suit(void) {
    CU_pSuite pSuite = CU_add_suite("Suite_CoapParseMessage", NULL, NULL);

    if (NULL == pSuite) {
        return CU_get_error();
    }

    return add_tests(pSuite, table);
}
