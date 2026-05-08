/*
 * Copyright 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_BITMASK_IFC_H
#define SID_BITMASK_IFC_H

#include <stdint.h>
#include <stdlib.h>

#include <sid_error.h>
#include <sid_parser_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t size_bits;   // Size of the bitmask in bits
    uint8_t size_words;   // Size of the bitmask in words
    uint32_t *mask;       // Pointer to the bitmask words array the bits are stored in
    size_t mask_len;      // Len of the mask
    size_t offset;        // Size_t of data in bytes that was written to read from
} sid_bitmask_t;

/**
 * Create a bitmask object
 *
 * @param [in,out]  bitmask Pointer to the created object to be returned in
 * @param [in]      bitmask_size_bits Size of the bitmask to be created
 * @param [in,out]  buffer Buffer to be created that the bitmask pointers
 *
 * @returns SID_ERROR_NONE on success or an appropriate error code
 *
 * @sa sid_bitmask_deinit
 */
sid_error_t sid_bitmask_init(sid_bitmask_t *bitmask, size_t bitmask_size_bits, uint32_t *buffer);

/**
 * Deinit a bitmask object
 *
 * @param [in] bitmask Pointer to the @c sid_bitmask_t object to deinit
 *
 * @sa sid_bitmask_init
 */
void sid_bitmask_deinit(sid_bitmask_t *bitmask);

/**
 * Clear all bits of the bitmask
 *
 * @param [in,out] bitmask Pointer to the @c sid_bitmask_t to clear the bits in
 */
void sid_bitmask_clear(sid_bitmask_t *bitmask);

/**
 * Clear bits according to bit set in another bitmask
 * The function clears bits in @p dst which are set in @p src effectively equivalent to *dst &= ~*src
 *
 * @param [out] dst Pointer to the @c sid_bitmask_t object to clear the bits in
 * @param [in]  src Pointer to the @c sid_bitmask_t object to retrieve set bit from
 *
 * @returns SID_ERROR_NONE on success or an appropriate error code
 */
sid_error_t sid_bitmask_clear_by_bitmask(sid_bitmask_t *dst, const sid_bitmask_t *src);

/**
 * Invert bit states in a bitmask
 * The function inverts each bit in the @p bitmask effectively equivalent to *bitmask = ~*bitmask;
 *
 * @param [in,out] bitmask Pointer to the @c sid_bitmask_t to invert bits in
 */
void sid_bitmask_invert(sid_bitmask_t *bitmask);

/**
 * Get a single bit state
 *
 * @param [in] bitmask Pointer to the @c sid_bitmask_t to get the bit state from
 * @param [in] bit Number of the bit to get
 *
 * returns uint8_t variable to return bit state in
 */
uint8_t sid_bitmask_bit_get(sid_bitmask_t *bitmask, size_t bit);

/**
 * Set a single bit state
 *
 * @param [out] bitmask Pointer to the @c sid_bitmask_t to set the bit in
 * @param [in] bit Number of the bit to set
 */
void sid_bitmask_bit_set(sid_bitmask_t *bitmask, size_t bit);

/**
 * Clear a single bit
 *
 * @param [out] bitmask Pointer to the @c sid_bitmask_t to set the bit in
 * @param [in] bit Number of the bit to clear
 */
void sid_bitmask_bit_clear(sid_bitmask_t *bitmask, size_t bit);

/**
 * Get the first clear bit number in the bitmask
 * The function returns the leftmost (MSB) bit equal to 0 from the @c sid_bitmask_t object.
 *
 * @param [in] bitmask Pointer to the @c sid_bitmask_t object to operate on
 *
 * @returns leftmost (MSB) bit equal to 0 from the @c sid_bitmask_t object
 */
size_t sid_bitmask_bit_get_first_clear(sid_bitmask_t *bitmask);

/**
 * Serialize the bitmask into a data buffer
 * The function tries to write all the bitmask bits into the @p buffer of state provided. If the size of the buffer is
 * not enough, the variable to which @c buffer_len within state points is updated with the minimal buffer size required
 * and SID_ERROR_BUFFER_OVERFLOW error code is generated. No data is written in this case into the @p buffer.
 *
 * @param [in] bitmask Pointer to the @c sid_bitmask_t to be serialized
 * @param [in,out] state Pointer to the sid_parse_state
 *
 * @sa sid_bitmask_deserialize
 */
void sid_bitmask_serialize(sid_bitmask_t *bitmask, struct sid_parse_state *const state);

/**
 * Deserialize bitmask data into the current bitmask
 * The function tries to read bitmask bits from the @p buffer of size provided by @c buffer_len in state replacing the
 * current
 * @c sid_bitmask_t object data with the data read. If the buffer provided is not enough to read all the @p
 * bitmask_bits,error code of SID_ERROR_BUFFER_OVERFLOW generates and the original bitmask state is preserved.
 *
 * @param [out]     bitmask      Pointer to the @c sid_bitmask_t into which data is to be deserialized
 * @param [in]      state        Pointer to the sid_parse_state
 *
 * @sa sid_bitmask_serialize
 */
void sid_bitmask_deserialize(sid_bitmask_t *bitmask, struct sid_parse_state *const state);

/**
 * Copy source bitmask data into a destination bitmask data
 *
 * This function copies mask from src_bitmask to mask contained in dst_bitmask
 *
 * @param [out]     dst_bitmask      Pointer to the @c sid_bitmask_t where the mask is to be copied
 * @param [in]      srt_bitmask      Pointer to the @c sid_bitmask_t of data to be copied
 *
 * @returns SID_ERROR_NONE on success or error code
 */
sid_error_t sid_bitmask_copy_bitmask(sid_bitmask_t *const dst_bitmask, const sid_bitmask_t *const src_bitmask);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SID_BITMASK_IFC_H */
