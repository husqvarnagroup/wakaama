#include "er-coap-13/er-coap-13.h"

#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > UINT16_MAX) {
        return 0;
    }

    uint8_t non_const_data[size];
    memcpy(non_const_data, data, size);

    coap_packet_t coap_pkt;
    memset(&coap_pkt, 0, sizeof(coap_packet_t));
    coap_parse_message(&coap_pkt, non_const_data, (uint16_t)size);
    coap_free_header(&coap_pkt);

    return 0;
}
