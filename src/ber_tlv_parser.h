/*============================================================================
 *
 *      ber_tlv_parser.h
 *
 *==========================================================================*/
/*============================================================================
 *
 * Author(s): Sandro P. Jacobsen
 * Date: 09/10/2021
 * Description: Header of a BER-TLV library
 *
 *==========================================================================*/

#ifndef __BER_TLV_PARSER_H__
#define __BER_TLV_PARSER_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Indicates TRUE.*/
#define TRUE						1
/** Indicates FALSE.*/
#define FALSE						0

/** Success.*/
#define TLV_SUCCESS         		0
/** Invalid parameter.*/
#define TLV_INV_PARAMETER   		100
/** Low memory available.*/
#define TLV_LOW_MEMORY	   			101
/** Tag not found.*/
#define TLV_TAG_NOT_FOUND	   		102
/** Command not implemented yet.*/
#define TLV_COMMAND_NOT_AVAILABLE	103

/** Enum to indicate the TLV Class */
enum tlv_tag_class
{
    CLASS_UNIVERSAL = 0,
    CLASS_APPLICATION = 1,
    CLASS_CONTEXT = 2,
    CLASS_PRIVATE = 3
};

/** Struct of a BER-TLV */
typedef struct
{
    /** Tag of the TLV, up to 2 bytes */
	uint16_t tag;
    /** Length of the TLV up to 4 bytes*/
	uint32_t length;
    /** Value of the TLV*/
	uint8_t* value;
    /** Pointer to the next TLV structure*/
	void* next;
} tlv_struct;

typedef tlv_struct * tlv_t;

/**
 * @brief
 * Create an empty TLV structure with default values.
 * @return Return the TLV structure created.
 */
tlv_t tlv_create(void);

/**
 * @brief
 * Release all resources from a TLV structure.
 * @param[in] tlv
 * TLV structure to be released.
 */
void tlv_release(tlv_t tlv);

/**
 * @brief
 * Add a new tag node to an existent TLV structure.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * The tag value to be added to the structure.
 * @return #TLV_SUCCESS to indicate success,
 * @return #TLV_INV_PARAMETER to indicate an invalid parameter,
 * @return #TLV_LOW_MEMORY to indicate low memory.
 */
uint8_t tlv_add_next_tag(tlv_t tlv, uint16_t tag);

/**
 * @brief
 * Set a tag value to the TLV structure.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * The tag value to set to the structure.
 * @return #TLV_SUCCESS to indicate success,
 * @return #TLV_INV_PARAMETER to indicate an invalid parameter.
 */
uint8_t tlv_set_tag(tlv_t tlv, uint16_t tag);

/**
 * @brief
 * Set a TLV value to a specific tag
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * The tag value to add a new value.
 * @param[in] value
 * The value to be added to the tag.
 * @param[in] value_length
 * The value length.
 * @return #TLV_SUCCESS to indicate success,
 * @return #TLV_INV_PARAMETER to indicate an invalid parameter,
 * @return #TLV_LOW_MEMORY to indicate low memory,
 * @return #TLV_TAG_NOT_FOUND to indicate that the tag value was not found in the TLV structure.
 */
uint8_t tlv_set_value(tlv_t tlv, uint16_t tag, uint8_t *value, uint32_t value_length);

/**
 * @brief
 * Get the tag information.
 * @param[in] tlv
 * The TLV structure.
 * @param[out] tag
 * The tag information.
 * @return #TLV_SUCCESS to indicate success,
 * @return #TLV_INV_PARAMETER to indicate an invalid parameter,
 * @return #TLV_TAG_NOT_FOUND to indicate that the tag value was not found in the TLV structure.
 */
uint8_t tlv_get_tag(tlv_t tlv, uint16_t *tag);

/**
 * @brief
 * Get the tag length.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * Tag that should be checked.
 * @param[out] length
 * The length of the tag informed.
 * @return #TLV_SUCCESS to indicate success,
 * @return #TLV_INV_PARAMETER to indicate an invalid parameter,
 * @return #TLV_TAG_NOT_FOUND to indicate that the tag value was not found in the TLV structure.
 */
uint8_t tlv_get_length(tlv_t tlv, uint16_t tag, uint32_t *length);

/**
 * @brief
 * Get the tag value.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * Tag that should be checked.
 * @param[out] value
 * The pointer to the value of the tag informed.
 * @return #TLV_SUCCESS to indicate success,
 * @return #TLV_INV_PARAMETER to indicate an invalid parameter,
 * @return #TLV_TAG_NOT_FOUND to indicate that the tag value was not found in the TLV structure.
 */
uint8_t tlv_get_value(tlv_t tlv, uint16_t tag, uint8_t *value);

/**
 * @brief
 * Get the next node of TLV structure.
 * @param[in] tlv
 * The TLV structure.
 * @return The TLV structure of the next node,
 */
tlv_t tlv_get_next_tlv(tlv_t tlv);

/**
 * @brief
 * Get the tag class.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * The tag to be checked.
 * @return The class of the tag informed.
 */
uint8_t tlv_get_tag_class(tlv_t tlv, uint16_t tag);

/**
 * @brief
 * Check if the tag is constructed.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * The tag to be checked.
 * @return #TRUE to indicate that is constructed,
 * @return #FALSE otherwise.
 */
uint8_t tlv_is_constructed(tlv_t tlv, uint16_t tag);

/**
 * @brief
 * Check if the tag is primitive.
 * @param[in] tlv
 * The TLV structure.
 * @param[in] tag
 * The tag to be checked.
 * @return #TRUE to indicate that is primitive,
 * @return #FALSE otherwise.
 */
uint8_t tlv_is_primitive(tlv_t tlv, uint16_t tag);

/**
 * @brief
 * Serialize a TLV structure in a sequency of bytes.
 * @param[in] tlv
 * The TLV structure.
 * @param[out] data
 * The TLV data serialized.
 * @param[out] data_length
 * The TLV data size.
 * @return #TLV_COMMAND_NOT_AVAILABLE to indicate the command is not implemented yet.
 */
uint8_t tlv_serialize(tlv_t tlv, uint8_t *data, uint32_t *data_length);

/**
 * @brief
 * Parse a TLV buffer and create a TLV structure.
 * @param[in] data
 * The TLV data in sequency of bytes.
 * @param[in] data_length
 * The TLV data length
 * @return The TLV structure with data parsed.
 */
tlv_t tlv_parse(uint8_t *data, uint32_t data_length);

#ifdef __cplusplus
}
#endif

#endif /* __BER_TLV_PARSER_H__ */