#include <stdio.h>
#include <stdint.h>
#include "ber_tlv_parser.h"

#define INITIAL_OBJ_LEN     13

static void print_spaces(uint32_t spaces) {
    for (uint32_t i = 0; i < spaces; i++) {
        printf(" ");
    }
}

static int print_tlv_info(tlv_t tlv) {
    /* Sanity check */
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    printf("\n\n");
    tlv_t node = tlv;
    uint32_t number_of_spaces = 0, i = 0;
    while(node) {
        print_spaces(number_of_spaces);
        printf("TAG - 0x%X (", node->tag);
        switch (tlv_get_tag_class(node, node->tag))
        {
        case CLASS_UNIVERSAL:
            printf("universal");
            break;
        case CLASS_APPLICATION:
            printf("application");
            break;
        case CLASS_CONTEXT:
            printf("context-specific");
            break;
        case CLASS_PRIVATE:
            printf("private");
            break;
        
        default:
            printf("not found");
            break;
        }
        printf(" class, ");
        if (tlv_is_constructed(node, node->tag)) {
            printf("constructed");
        } else if (tlv_is_primitive(node, node->tag)) {
            printf("primitive");
        }
        printf(")\n");

        print_spaces(number_of_spaces);
        printf("LEN - %d bytes\n", node->length);
        if (node->length > 0) {
            if (tlv_is_primitive(node, node->tag)) {
                for (i = 0; i < node->length; i++) {
                    if (i == 0) {
                        print_spaces(number_of_spaces);
                        printf("VAL -");
                    }
                    printf(" 0x%.2X", node->value[i]);
                }
                printf("\n");
            } else {
                number_of_spaces+=2;
            }
        }
        printf("\n");
        node = tlv_get_next_tlv(node);
    }
    return TLV_SUCCESS;
}

int main(void) {
    uint8_t tlvObject[INITIAL_OBJ_LEN] = {
        0xE1, 0x0B, 0xC1, 0x03, 0x01, 0x02,
        0x03, 0xC2, 0x00, 0xC3, 0x02, 0xAA,
        0xBB
    };

    // Parse TLV buffer to a structure
    tlv_t tlv = tlv_parse((uint8_t *)tlvObject, INITIAL_OBJ_LEN);

    // Print TLV information
    print_tlv_info(tlv);

    // Release all resources allocated
    tlv_release(tlv);
    return 0;
}