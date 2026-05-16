#ifndef __CRYPTO_H
#define __CRYPTO_H

#include <stdbool.h>

#define CRYPTO_KEY_LEN   32
#define CRYPTO_IV_LEN    12
#define CRYPTO_TAG_LEN   16
#define CRYPTO_SALT_LEN  16

bool crypto_init(void);
bool crypto_init_with_key(const unsigned char *key, int keylen);
bool crypto_encrypt(const char *plaintext, char **out_ct, char **out_iv, char **out_tag);
bool crypto_decrypt(const char *hex_ct, const char *hex_iv, const char *hex_tag, char **out_plain);

#endif