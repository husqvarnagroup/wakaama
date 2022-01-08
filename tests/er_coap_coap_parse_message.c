#include "er-coap-13/er-coap-13.h"

#include "tests.h"

static coap_packet_t coap_pkt;

static void test_regression1(void) {
    uint8_t data[] = {0x6E, 0x8D};

    CU_ASSERT_EQUAL(BAD_REQUEST_4_00, coap_parse_message(&coap_pkt, data, sizeof(data)))
}

static void test_empty_message(void) {
    uint8_t data[] = {
        0x40, // version 1, no options, no tokens
        0x00, // Empty message
        0x00, // Message ID 0x0000
        0x00,
    };

    CU_ASSERT_EQUAL(NO_ERROR, coap_parse_message(&coap_pkt, data, sizeof(data)))
}

static void test_empty_message_with_superfluous_data(void) {
    uint8_t data[] = {
        0x40, // version 1, no options, no tokens
        0x00, // Empty message
        0x00, // Message ID 0x0000
        0x00,
        0xFF, // Illegal payload marker
    };

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), BAD_REQUEST_4_00);
}

static void test_field_version_0(void) {
    uint8_t data[] = {0x00 /* Version 0 */
                      ,
                      0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), BAD_REQUEST_4_00);
}

static void test_field_version_1(void) {
    uint8_t data[] = {0x40 /* Version 1 */, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.version, 1);
}

static void test_field_version_2(void) {
    uint8_t data[] = {0x80 /* Version 2*/, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), BAD_REQUEST_4_00);
}

static void test_field_version_3(void) {
    uint8_t data[] = {0xC0 /* Version 2*/, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), BAD_REQUEST_4_00);
}

static void test_field_type_confirmable(void) {
    uint8_t data[] = {0x40, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.type, COAP_TYPE_CON);
}

static void test_field_type_non_confirmable(void) {
    uint8_t data[] = {0x50, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.type, COAP_TYPE_NON);
}

static void test_field_type_acknowledgement(void) {
    uint8_t data[] = {0x60, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.type, COAP_TYPE_ACK);
}

static void test_field_type_reset(void) {
    uint8_t data[] = {0x70, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.type, COAP_TYPE_RST);
}

static void test_field_token_length_min(void) {
    uint8_t data[] = {0x40, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.token_len, 0);
}

static void test_field_token_length_max(void) {
    uint8_t data[] = {0x48, 0x00, 0x00, 0x00,
                      /* Followed by 8 bytes of tokens */
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), NO_ERROR);
    CU_ASSERT_EQUAL(coap_pkt.token_len, 8);
}

static void test_field_token_length_reserved(void) {
    uint8_t data[] = {0x49, 0x00, 0x00, 0x00,
                      /* Followed by 9 bytes of tokens */
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    CU_ASSERT_EQUAL(coap_parse_message(&coap_pkt, data, sizeof(data)), BAD_REQUEST_4_00);
}

static struct TestTable table[] = {
    {"Regression #1", test_regression1},
    {"Empty message", test_empty_message},
    {"Empty message with illegal, extra data", test_empty_message_with_superfluous_data},
    {"Field Version: 0", test_field_version_0},
    {"Field Version: 1", test_field_version_1},
    {"Field Version: 2", test_field_version_2},
    {"Field Version: 3", test_field_version_3},
    {"Field Type: Confirmable", test_field_type_confirmable},
    {"Field Type: Non-confirmable", test_field_type_non_confirmable},
    {"Field Type: Acknowledgement", test_field_type_acknowledgement},
    {"Field Type: Reset", test_field_type_reset},
    {"Field Token Lenght: Minimum", test_field_token_length_min},
    {"Field Token Lenght: Maximum", test_field_token_length_max},
    {"Field Token Lenght: Reserved", test_field_token_length_reserved},
    // GET
    // PUT
    // DELETE
    // POST
    // OPTIONs:
    // - OBSERVE
    // TRACE
    // CONNECT
    // Non-Understood Method
    // Max sized UDP (65,535 bytes (8-byte header + 65,527 bytes of data), but no jumbo frames)
    {NULL, NULL},
};

CU_ErrorCode create_er_coap_parse_message_suit(void) {
    CU_pSuite pSuite = CU_add_suite("Suite_CoapParseMessage", NULL, NULL);

    if (NULL == pSuite) {
        return CU_get_error();
    }

    return add_tests(pSuite, table);
}
