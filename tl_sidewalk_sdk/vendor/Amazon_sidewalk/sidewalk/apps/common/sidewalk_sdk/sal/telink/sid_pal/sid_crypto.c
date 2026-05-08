/********************************************************************************************************
 * @file    sid_crypto.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    11,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
/** @file sid_crypto.c
 *  @brief Sidewalk cryptography interface implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_crypto_ifc.h>
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_crypto_keys.h>
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

#include <psa/crypto.h>
#include <psa/crypto_extra.h>
#include "lib/include/pke/ed25519.h"
#include "lib/include/pke/ed25519.h"
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif

#include "app_mem.h"
#include "app_buffer.h"

#define BYTE_TO_BITS(_byte) (_byte << 3)
#define BITS_TO_BYTE(_bits) (_bits >> 3)

/* AES key length in bytes. */
#define AES_128_KEY_LENGTH (16)

/* EC key length in bits. */
#define CURVE25519_KEY_LEN_BITS (255)
#define SECP256R1_KEY_LEN_BITS (256)
#define ED25519_KEY_LEN_BITS (255)


#define MAX_SIGN_BUF_LEN 64
/* Crypto initialization global flag. */
static bool is_initialized = false;


//_attribute_ble_data_retention_ static uint8_t ed25519_public_key[32];
//_attribute_ble_data_retention_ static uint8_t ed25519_private_key[64];
//_attribute_ble_data_retention_ static bool ed25519_keys_present = false;
//
//
//_attribute_ble_data_retention_ static uint8_t x25519_public_key[32];
//_attribute_ble_data_retention_ static uint8_t x25519_private_key[64];
//_attribute_ble_data_retention_ static bool x25519_keys_present = false;

int mbedtls_platform_set_calloc_free( void * (*calloc_func)( size_t, size_t ),
                              void (*free_func)( void * ) );
sid_error_t sid_pal_crypto_init(void)
{

    trng_dig_en();
    //pke_dig_en();
    hash_dig_en();
    //ske_dig_en();
//    memset(ed25519_public_key,0,sizeof(ed25519_public_key));
//    memset(ed25519_private_key,0,sizeof(ed25519_private_key));
//    memset(x25519_public_key,0,sizeof(x25519_public_key));
//    memset(x25519_private_key,0,sizeof(x25519_private_key));
//
//    ed25519_keys_present = false;
//    x25519_keys_present = false;

    is_initialized = true;
    TL_LOG_D("Init success!");
    return  0;
}

sid_error_t sid_pal_crypto_deinit(void)
{
    trng_dig_dis();
    is_initialized = false;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_crypto_rand(uint8_t *rand, size_t size)
{
    uint32_t res;
    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!rand) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!size) {
        return SID_ERROR_INVALID_ARGS;
    }

    size_t bytes_filled = 0;

    while (bytes_filled + sizeof(unsigned int) <= size) {
        unsigned int random_val = trng_rand();
        rand[bytes_filled++] = (uint8_t)(random_val & 0xFF);
        rand[bytes_filled++] = (uint8_t)((random_val >> 8) & 0xFF);
        rand[bytes_filled++] = (uint8_t)((random_val >> 16) & 0xFF);
        rand[bytes_filled++] = (uint8_t)((random_val >> 24) & 0xFF);
    }
    if (bytes_filled < size) {
        unsigned int random_val = trng_rand();
        for (size_t i = 0; bytes_filled < size; i++) {
            rand[bytes_filled++] = (uint8_t)((random_val >> (i * 8)) & 0xFF);
        }
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t *params)
{
    uint32_t res;
    HASH_ALG alg_sha;

    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->data || !params->digest) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->data_size || !params->digest_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    switch (params->algo) {
    case SID_PAL_HASH_SHA256:
        alg_sha = HASH_SHA256;
        break;
    case SID_PAL_HASH_SHA512:
        void sha512_hash(const unsigned char *message, unsigned int byteLen, unsigned char digest[64]);
        sha512_hash(params->data, params->data_size, params->digest);
        return SID_ERROR_NONE;
    default:
        return SID_ERROR_NOSUPPORT;
    }
    res = hash(alg_sha, params->data, params->data_size, params->digest);

    if (res == HASH_SUCCESS) {
        return SID_ERROR_NONE;
    } else {
        return SID_ERROR_GENERIC;
    }
}



sid_error_t sid_pal_crypto_hw_hmac(sid_pal_hmac_params_t *params)
{
    uint32_t res;
    HASH_ALG alg_sha;

    switch (params->algo) {
    case SID_PAL_HASH_SHA256:
        alg_sha = HASH_SHA256;
        break;
    case SID_PAL_HASH_SHA512:
        void hmac_sha512(unsigned char *key, unsigned int keyByteLen, unsigned char *msg, unsigned int msgByteLen, unsigned char mac[64]);

        hmac_sha512(params->key, params->key_size, params->data, params->data_size, params->digest);
        return SID_ERROR_NONE;
        break;
    default:
        return SID_ERROR_NOSUPPORT;
    }

    res = hmac(alg_sha, params->key, 0, params->key_size, params->data, params->data_size, params->digest);
    if (res == HASH_SUCCESS) {
        return SID_ERROR_NONE;
    } else {
        return SID_ERROR_GENERIC;
    }
}

sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t *params)
{

    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->data || !params->digest) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->key_size || !params->data_size || !params->digest_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    TL_LOG_D("sid_pal_crypto_hmac  %d %d %d",params->key_size,params->data_size,params->algo);
    return sid_pal_crypto_hw_hmac(params);

}

#define  PAL_AES_BLOCK_SIZE 16
#define PAL_AES_BLOCK_SIZE_MAX_SIZE  1152
#if AMAZON_DIAG_DEMO || AMAZON_DUT_DEMO
__attribute__((section(".retention_data"))) uint8_t temp_in_buf[PAL_AES_BLOCK_SIZE_MAX_SIZE] ;
#else
__attribute__((section(".ram_data_ble"))) uint8_t temp_in_buf[PAL_AES_BLOCK_SIZE_MAX_SIZE] ;
#endif
sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t *params)
{
    size_t key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
    SKE_ALG alg;
    int32_t res = 0;
    uint8_t *ptr_out = NULL;
    uint8_t *ptr_in = NULL;
    uint32_t  out_size = PAL_AES_BLOCK_SIZE;
    SKE_CRYPTO crypto= 0;

    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->in || !params->out ||
        ((SID_PAL_AES_CTR_128 == params->algo) && !params->iv)) {
        return SID_ERROR_NULL_POINTER;
    }
    TL_LOG_D("sid_pal_crypto_aes_crypt  %d %d %d %d",params->in_size,params->out_size,params->algo,params->mode);
    if (!params->in_size || !params->out_size || (params->out_size < 16  && params->algo != SID_PAL_AES_CTR_128) ) {
        return SID_ERROR_INVALID_ARGS;
    }

    switch (params->algo) {
    case SID_PAL_AES_CMAC_128:
        alg = PSA_ALG_CMAC;
        key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
        break;
    case SID_PAL_AES_CTR_128:
        alg = PSA_ALG_CTR;
        key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);

        break;
    default:
        return SID_ERROR_NOSUPPORT;
    }

    if ((key_len != params->key_size) ||
        ((SID_PAL_AES_CTR_128 == params->algo) &&
         params->iv_size != PSA_CIPHER_IV_LENGTH(PSA_KEY_TYPE_AES, alg))) {
        // Log is only for debug purpose, in other case use error code.
        TL_LOG_E("Incorrect %s length.", (key_len != params->key_size) ? "key" : "IV");
        return SID_ERROR_INVALID_ARGS;
    }

    switch (params->mode) {
    case SID_PAL_CRYPTO_ENCRYPT:
        crypto = SKE_CRYPTO_ENCRYPT;
        break;
    case SID_PAL_CRYPTO_DECRYPT:
        crypto = SKE_CRYPTO_DECRYPT;
        break;

    case SID_PAL_CRYPTO_MAC_CALCULATE:
        res = ske_lp_cmac(SKE_ALG_AES_128, SKE_GENERATE_MAC, params->key, 0, params->in, params->in_size,
            params->out, params->out_size);
        break;

    default:
        return SID_ERROR_INVALID_ARGS;
    }

    if(params->algo == SID_PAL_AES_CTR_128)
    {
         uint32_t  out_size = params->in_size & 0xf;
         if(out_size != 0)
         {

             if(params->in_size  > PAL_AES_BLOCK_SIZE_MAX_SIZE)
             {
                 sid_pal_assert(__LINE__, __FILE__);
             }
            uint32_t skt_len = (params->in_size &0xFFFFFFF0) + 16;
             memset(temp_in_buf,0,PAL_AES_BLOCK_SIZE_MAX_SIZE);
             memcpy(temp_in_buf,params->in,params->in_size);
             res = ske_lp_crypto(SKE_ALG_AES_128, SKE_MODE_CTR, crypto, params->key, 0, params->iv,
                     temp_in_buf,temp_in_buf,skt_len);
             memcpy(params->out,temp_in_buf,params->in_size);
         }

         else
         {
              res = ske_lp_crypto(SKE_ALG_AES_128, SKE_MODE_CTR, crypto, params->key, 0, params->iv,
                      params->in,params->out,params->in_size);
         }

    }

    if (res == SKE_SUCCESS) {
        return SID_ERROR_NONE;
    } else {
        return res;
    }
}

sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t *params)
{
    uint32_t res;
    size_t key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
    SKE_CRYPTO en_de;
    uint32_t alg;
    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    TL_LOG_D("sid_pal_crypto_aead_crypt  %d %d %d %d",params->in_size,params->out_size,params->algo,params->mode);

    if (!params || !params->key || !params->in || !params->out || !params->aad ||
        !params->mac) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->in_size || !params->aad_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    switch (params->algo) {
    case SID_PAL_AEAD_GCM_128:
        alg = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, params->mac_size);
        key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
        break;
    case SID_PAL_AEAD_CCM_128:
        alg = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, params->mac_size);
        key_len = BYTE_TO_BITS(AES_128_KEY_LENGTH);
        break;
    case SID_PAL_AEAD_CCM_STAR_128:
    default:
        return SID_ERROR_NOSUPPORT;
    }

    if (key_len != params->key_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    if ((NULL != params->iv) &&
        (params->iv_size != PSA_AEAD_NONCE_LENGTH(PSA_KEY_TYPE_AES, alg))) {
        return SID_ERROR_INVALID_ARGS;
    }
    switch (params->mode) {
    case SID_PAL_CRYPTO_ENCRYPT:
        en_de = SKE_CRYPTO_ENCRYPT;
        break;
    case SID_PAL_CRYPTO_DECRYPT:
        en_de = SKE_CRYPTO_DECRYPT;
        break;
    default:
        return SID_ERROR_INVALID_ARGS;
    }

    switch (params->algo) {
    case SID_PAL_AEAD_GCM_128:
        res = ske_lp_gcm_crypto(SKE_ALG_AES_128, en_de, params->key, 0, params->iv, params->iv_size,
            params->aad, params->aad_size, params->in, params->out, params->in_size,
            params->mac, params->mac_size);
        break;
    case SID_PAL_AEAD_CCM_128:
        res = ske_lp_ccm_crypto(SKE_ALG_AES_128, en_de, params->key, 0, params->iv,
            params->mac_size, 15-params->iv_size, params->aad, params->aad_size, params->in, params->out, params->in_size,
            params->mac);
        break;
    case SID_PAL_AEAD_CCM_STAR_128:
    default:
        return SID_ERROR_NOSUPPORT;
    }

    if (res == SKE_SUCCESS) {
        return SID_ERROR_NONE;
    } else {
        return res;
    }
}


static sid_error_t sid_pal_crypto_256r1_ecc_dsa(sid_pal_dsa_params_t *params)
{
     int res = 0;
     unsigned char digest[32];
     unsigned char digest_byte_len = 32;

     memset_(digest, 0, digest_byte_len);
     res = sha256(params->in,params->in_size, digest);
     if (HASH_SUCCESS != res) {
         TL_LOG_E("sha256fail %d ",res);
         return res;
     }
     {
      if (params->mode == SID_PAL_CRYPTO_SIGN) {


        // Always sign with the pre-generated private key.
        res = ecdsa_sign(secp256r1,digest,
                digest_byte_len,NULL,
                params->key ,params->signature);
        if (res == ECDSA_SUCCESS) {
//            params->sig_size = 64;
          return SID_ERROR_NONE;
        } else {
          return SID_ERROR_GENERIC;
        }
      } else {
        // Verify signature using the actual incoming public key.
        int verifyResult = ecdsa_verify(secp256r1,
                digest,
                digest_byte_len,
                                                   params->key,
                                                   params->signature);


        if (verifyResult == ECDSA_SUCCESS) {
          return SID_ERROR_NONE;
        } else {
            TL_LOG_E("ecdsa_verify fail %d ",verifyResult);
          return SID_ERROR_GENERIC;
        }
      }
    }

}

sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t *params)
{
    size_t key_len;
    uint8_t key_offset = 0;
    size_t key_size = 0;

    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->in || !params->signature) {
        return SID_ERROR_NULL_POINTER;
    }

    if (0 == params->in_size || 0 == params->key_size) {
        return SID_ERROR_INVALID_ARGS;
    }
    TL_LOG_D("sid_pal_crypto_ecc_dsa keysize %d %d %d %d ",params->key_size,params->in_size,params->sig_size, params->algo);
    key_size = params->key_size;
    switch (params->algo) {
    case SID_PAL_EDDSA_ED25519:
//        if ((params->key_size == 32) && (params->sig_size == 64))
        {
          if (params->mode == SID_PAL_CRYPTO_SIGN) {
            int res;
            // Always sign with the pre-generated private key.
            res = ed25519_sign(Ed25519_DEFAULT,
                    params->key,
                                      NULL,
                                      NULL,0,
                                      params->in,
                                      params->in_size,params->signature);
            if (res == ECDSA_SUCCESS) {
              return SID_ERROR_NONE;
            }
            else {
              return SID_ERROR_GENERIC;
            }
          }

          else {
            // Verify signature using the actual incoming public key.
            int verifyResult = ed25519_verify(Ed25519_DEFAULT,
                                                       params->key,
                                                       NULL,0,
                                                       params->in,
                                                       params->in_size,
                                                       params->signature);


            if (verifyResult == ECDSA_SUCCESS) {
              return SID_ERROR_NONE;
            } else {
              return SID_ERROR_GENERIC;
            }
          }
        }
        break;
    case SID_PAL_ECDSA_SECP256R1:
        return sid_pal_crypto_256r1_ecc_dsa(params);

    default:
        return SID_ERROR_NOSUPPORT;
    }
}


static sid_error_t sid_pal_crypto_256r1_ecc_ecdh(sid_pal_ecdh_params_t *params)
{
    uint32_t res;

    res = ecdh_compute_key(secp256r1, params->prk,params->puk,params->shared_secret, params->shared_secret_sz, NULL);
    if (res == ECDH_SUCCESS) {
        return SID_ERROR_NONE;
    } else {
        return SID_ERROR_GENERIC;
    }
}

sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params)
{
    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->prk || !params->puk || !params->shared_secret) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->prk_size || !params->puk_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    TL_LOG_D("sid_pal_crypto_ecc_ecdh  %d %d %d",params->puk_size,params->prk_size,params->algo);
    switch (params->algo) {
    case SID_PAL_ECDH_SECP256R1:
        return sid_pal_crypto_256r1_ecc_ecdh(params);
    case SID_PAL_ECDH_CURVE25519:
        int ret = x25519_compute_key(params->prk,params->puk,params->shared_secret, params->shared_secret_sz,NULL);
        if (ret == 0) {
            return SID_ERROR_NONE;
        } else {
            return SID_ERROR_GENERIC;
        }
    default:
        return SID_ERROR_NOSUPPORT;
    }
}



static sid_error_t sid_pal_crypto_ecc_256r1_key_gen(sid_pal_ecc_key_gen_params_t *params)
{
 TL_LOG_D("secp256r1_curve_dat %d %d!",params->prk_size,params->puk_size);
    uint32_t res;
    res = eccp_getkey(&secp256r1, params->prk, params->puk);
    if (res == PKE_SUCCESS) {
        return SID_ERROR_NONE;
    } else {
        return SID_ERROR_GENERIC;
    }

}

sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params)
{
    uint32_t res;
    size_t key_len;
    uint8_t pub_key_offset = 0;

    if (!is_initialized) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->prk || !params->puk) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->puk_size || !params->prk_size) {
        return SID_ERROR_INVALID_ARGS;
    }
    TL_LOG_D("sid_pal_crypto_ecc_key_gen  %d %d %d",params->puk_size,params->prk_size,params->algo);

    switch (params->algo) {
    case SID_PAL_EDDSA_ED25519:
        res = ed25519_getkey(params->prk,params->puk);
        if (res == EdDSA_SUCCESS) {
            return SID_ERROR_NONE;
        } else {
            return SID_ERROR_GENERIC;
        }
        break;
    case SID_PAL_ECDSA_SECP256R1:
    case SID_PAL_ECDH_SECP256R1:
         return sid_pal_crypto_ecc_256r1_key_gen(params);
        break;
    case SID_PAL_ECDH_CURVE25519:

            TL_LOG_W("SID_PAL_ECDH_CURVE25519 %d %d!",params->prk_size,params->puk_size);
            res = x25519_getkey(params->prk,params->puk);
            if (res == 0) {
//            memcpy(params->puk, x25519_public_key, params->puk_size);
//            memcpy(params->prk, x25519_private_key, params->prk_size);
//            x25519_keys_present = true;
              return SID_ERROR_NONE;
            } else {
              return SID_ERROR_GENERIC;
            }

    default:
        return SID_ERROR_NOSUPPORT;
    }


}

