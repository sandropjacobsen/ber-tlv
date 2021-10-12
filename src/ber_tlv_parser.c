/*============================================================================
 *
 *      ber_tlv_parser.c
 *
 *==========================================================================*/
/*============================================================================
 *
 * Author(s): Sandro P. Jacobsen
 * Date: 09/10/2021
 * Description: BER-TLV library
 *
 *==========================================================================*/

#include "ber_tlv_parser.h"
#include <string.h>

#define UNUSED(x) (void)(x)

#define TLV_MASK_UNIVERSAL        	0x00
#define TLV_MASK_APPLICATION      	0x40
#define TLV_MASK_CONTEXT_SPECIFIC 	0x80
#define TLV_MASK_PRIVATE          	0xC0

#define TLV_MASK_CONSTRUCTED    	0x20
#define TLV_MASK_TAG_FIRST_BYTE   	0X1F
#define TLV_MASK_LENGTH_SECOND_BYTE	0x7F

static tlv_t find_last_node(tlv_t tlv) {
    tlv_t last_node = tlv;
    while (last_node->next) {
        last_node = (tlv_t)last_node->next;
    }
    return last_node;
}

static tlv_t find_tag(tlv_t tlv, uint16_t tag) {
    tlv_t tag_node = tlv;
    while (tag_node) {
        if (tag_node->tag == tag) {
            return tag_node;
        }
        tag_node = (tlv_t)tag_node->next;
    }
    return NULL;
}

static uint8_t number_of_bytes_tag(uint16_t tag) {
    if (tag > 0xFF) {
        return 2;
    }
    return 1;
}

static uint8_t number_of_bytes_length(uint32_t length) {
    if (length > 0x7FFF) {
        return 3;
    } else if (length > 0x7F) {
        return 2;
    }
    return 1;
}

tlv_t tlv_create(void) {
    tlv_t tlv = (tlv_struct *)malloc(sizeof (tlv_struct));
    if (tlv != NULL) {
        tlv->tag = 0;
        tlv->length = 0;
        tlv->value = NULL;
        tlv->next = NULL;
    }
    return tlv;
}

void tlv_release(tlv_t tlv) {
    //sanity checks
    if (!tlv) {
        return;
    }
    if(tlv->next) {
        tlv_release((tlv_t)tlv->next);
    } else {
        if(!tlv->value) {
            free(tlv->value);
        }
        free(tlv);
    }
}

uint8_t tlv_add_next_tag(tlv_t tlv, uint16_t tag) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    tlv_t last_node = tlv;
    while (last_node->next)
    {
        last_node = (tlv_t)last_node->next;
    }
    last_node->next = tlv_create();
    if (!last_node->next) {
        return TLV_LOW_MEMORY;
    }
    return tlv_set_tag((tlv_t)last_node->next, tag);
}

uint8_t tlv_set_tag(tlv_t tlv, uint16_t tag) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    tlv->tag = tag;
    return TLV_SUCCESS;
}

uint8_t tlv_set_value(tlv_t tlv, uint16_t tag, uint8_t *value, uint32_t value_length) {
    //sanity checks
    if (!tlv || !value || !value_length) {
        return TLV_INV_PARAMETER;
    }
    uint8_t *new_value = calloc(value_length, sizeof(uint8_t));
    if (!new_value) {
        return TLV_LOW_MEMORY;
    }

    memcpy(new_value, value, value_length);

    tlv_t tag_node = find_tag(tlv, tag);
    if (!tag_node) {
        free(new_value);
        return TLV_TAG_NOT_FOUND;
    }
    if (tag_node->value) {
        free(tag_node->value); // free an allocated memory
    }
    tag_node->value = new_value;
    tag_node->length = value_length;
    return TLV_SUCCESS;
}

uint8_t tlv_get_tag(tlv_t tlv, uint16_t *tag) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    *tag = tlv->tag;
    return TLV_SUCCESS;
}

uint8_t tlv_get_length(tlv_t tlv, uint16_t tag, uint32_t *length) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    tlv_t tag_node = find_tag(tlv, tag);
    if (!tag_node) {
        return TLV_TAG_NOT_FOUND;
    }
    *length = tag_node->length;
    return TLV_SUCCESS;
}

uint8_t tlv_get_value(tlv_t tlv, uint16_t tag, uint8_t *value) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    tlv_t tag_node = find_tag(tlv, tag);
    if (!tag_node) {
        return TLV_TAG_NOT_FOUND;
    }
    value = tag_node->value;
    return TLV_SUCCESS;
}

tlv_t tlv_get_next_tlv(tlv_t tlv) {
    if (!tlv) {
        return NULL;
    }
    return (tlv_t)tlv->next;
}

uint8_t tlv_get_tag_class(tlv_t tlv, uint16_t tag) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    uint8_t tag_shift = 0;
    tlv_t tag_node = find_tag(tlv, tag);
    if (!tag_node) {
        return TLV_TAG_NOT_FOUND;
    }
    if (tag > 0xFF) { // tag with 2 bytes
        tag_shift = 8;
    }
    if ((tag_node->tag & (TLV_MASK_PRIVATE << tag_shift)) == (TLV_MASK_PRIVATE << tag_shift)) {
        return CLASS_PRIVATE;
    } else if ((tag_node->tag & (TLV_MASK_CONTEXT_SPECIFIC << tag_shift)) == (TLV_MASK_CONTEXT_SPECIFIC << tag_shift)) {
        return CLASS_CONTEXT;
    } else if ((tag_node->tag & (TLV_MASK_APPLICATION << tag_shift)) == (TLV_MASK_APPLICATION << tag_shift)) {
        return CLASS_APPLICATION;
    } else {
        return CLASS_UNIVERSAL;
    }
}

uint8_t tlv_is_constructed(tlv_t tlv, uint16_t tag) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    uint8_t tag_shift = 0;
    tlv_t tag_node = find_tag(tlv, tag);
    if (!tag_node) {
        return FALSE;
    }
    if (tag > 0xFF) { // tag with 2 bytes
        tag_shift = 8;
    }
    if (((tag_node->tag & (0xFF << tag_shift)) & (TLV_MASK_CONSTRUCTED << tag_shift)) != 0) {
        return TRUE;
    }
    return FALSE;
}

uint8_t tlv_is_primitive(tlv_t tlv, uint16_t tag) {
    //sanity checks
    if (!tlv) {
        return TLV_INV_PARAMETER;
    }
    uint8_t tag_shift = 0;
    tlv_t tag_node = find_tag(tlv, tag);
    if (!tag_node) {
        return TLV_TAG_NOT_FOUND;
    }
    if (tag > 0xFF) { // tag with 2 bytes
        tag_shift = 8;
    }
    if (((tag_node->tag & (0xFF << tag_shift)) & (TLV_MASK_CONSTRUCTED << tag_shift)) == 0) {
        return TRUE;
    }
    return FALSE;
}

uint8_t tlv_serialize(tlv_t tlv, uint8_t *data, uint32_t *data_length) {
    // TODO: Needs to be implemented
    UNUSED(tlv);
    UNUSED(data);
    UNUSED(data_length);
    return TLV_COMMAND_NOT_AVAILABLE;
}

tlv_t tlv_parse(uint8_t *data, uint32_t data_length) {
    uint32_t counter = 0;
    uint32_t length_size = 0;
    tlv_t tlv = tlv_create();

    if(data != NULL && data_length > 0)	{
        while(data[counter]==0) {
            counter++;
        }
        // if bits b5 - b1 of the first byte equal '11111'
        tlv->tag = data[counter];
        if((data[counter] & TLV_MASK_TAG_FIRST_BYTE) == TLV_MASK_TAG_FIRST_BYTE ) {
            counter++;
            tlv->tag <<= 8;
            tlv->tag |= data[counter];
        }
        counter++;

        // 1 byte Length field
        if(data[counter] <= TLV_MASK_LENGTH_SECOND_BYTE) {
            tlv->length = data[counter++];
        } else {
            switch(data[counter]) {
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x84:
                length_size = data[counter] & TLV_MASK_LENGTH_SECOND_BYTE;
                do {
                    tlv->length <<= 8;
                    tlv->length |= data[++counter];
                    length_size--;
                } while (length_size > 0);
                counter++;
                break; 

            default : 
                break;
            }
        }
        //parse Value field
        if(tlv->length > 0) {
            if(tlv_is_constructed(tlv, tlv->tag)) {
                tlv_t next_node = tlv;
                uint32_t next_bytes = 0;
                while (next_node && (tlv->length > next_bytes)) {
                    next_node->next = tlv_parse(&data[counter+next_bytes], (tlv->length - next_bytes));
                    if (next_node->next) {
                        next_bytes += tlv_get_next_tlv(next_node)->length;
                        next_bytes+=number_of_bytes_tag(tlv_get_next_tlv(next_node)->tag);
                        next_bytes+=number_of_bytes_length(tlv_get_next_tlv(next_node)->length);
                    }
                    next_node = next_node->next;
                }
            }
            tlv_set_value(tlv, tlv->tag, &data[counter], tlv->length);
            counter += tlv->length;
            // Check if exist an primitive in the same level as a constructed
            while ((data_length > counter) && (data[counter] != 0)) {
                tlv_t next_node = find_last_node(tlv);
                uint32_t next_bytes = 0;
                next_node->next = tlv_parse(&data[counter], (data_length - counter));
                if (next_node->next) {
                    counter += tlv_get_next_tlv(next_node)->length;
                    counter+=number_of_bytes_tag(tlv_get_next_tlv(next_node)->tag);
                    counter+=number_of_bytes_length(tlv_get_next_tlv(next_node)->length);
                } else {
                    break;
                }
                next_node = next_node->next;
            }
        }
    }
    return tlv;
}
