#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <crypto.h>

static unsigned char master_key[CRYPTO_KEY_LEN];
static bool key_ready = false;

static const unsigned char SALT[CRYPTO_SALT_LEN] = {
  0x5a, 0x3f, 0x1c, 0x8e, 0x72, 0xb4, 0x09, 0xd6,
  0xae, 0x47, 0xf3, 0x21, 0x90, 0xcc, 0x5d, 0x88
};

static void bytes_to_hex(const unsigned char *in, int len, char *out)
{
  for (int i = 0; i < len; i++)
    sprintf(out + i * 2, "%02x", in[i]);
  out[len * 2] = 0;
}

static void hex_to_bytes(const char *hex, unsigned char *out, int len)
{
  for (int i = 0; i < len; i++)
    sscanf(hex + i * 2, "%02hhx", &out[i]);
}

bool crypto_init(void)
{
  char passphrase[128];
  
  fgets(passphrase, sizeof(passphrase), stdin);
  passphrase[strcspn(passphrase, "\n")] = 0;

  int ok = PKCS5_PBKDF2_HMAC(
    passphrase, strlen(passphrase),
    SALT, CRYPTO_SALT_LEN,
    200000, // iterations
    EVP_sha256(),
    CRYPTO_KEY_LEN, master_key
  );

  memset(passphrase, 0, sizeof(passphrase));
  key_ready = (ok == 1);

  return key_ready;
}

bool crypto_init_with_key(const unsigned char *key, int keylen)
{
  if (keylen != CRYPTO_KEY_LEN) return false;
  memcpy(master_key, key, CRYPTO_KEY_LEN);
  key_ready = true;

  return true;
}

bool crypto_encrypt(const char *plaintext, char **out_ct, char **out_iv, char **out_tag)
{
  if (!key_ready)
    return false;

  unsigned char iv[CRYPTO_IV_LEN];
  unsigned char tag[CRYPTO_TAG_LEN];
  int plen = strlen(plaintext);
  unsigned char *ct = malloc(plen + 16);

  if (!ct)
    return false;

  RAND_bytes(iv, CRYPTO_IV_LEN);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  int len, ct_len;

  EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
  EVP_EncryptInit_ex(ctx, NULL, NULL, master_key, iv);
  EVP_EncryptUpdate(ctx, ct, &len, (unsigned char *)plaintext, plen);
  ct_len = len;
  EVP_EncryptFinal_ex(ctx, ct + len, &len);
  ct_len += len;
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, CRYPTO_TAG_LEN, tag);
  EVP_CIPHER_CTX_free(ctx);

  *out_ct = malloc(ct_len * 2 + 1);
  *out_iv = malloc(CRYPTO_IV_LEN * 2 + 1);
  *out_tag = malloc(CRYPTO_TAG_LEN * 2 + 1);

  bytes_to_hex(ct, ct_len, *out_ct);
  bytes_to_hex(iv, CRYPTO_IV_LEN, *out_iv);
  bytes_to_hex(tag, CRYPTO_TAG_LEN, *out_tag);

  free(ct);

  return true;
}

bool crypto_decrypt(const char *hex_ct, const char *hex_iv, const char *hex_tag, char **out_plain)
{
  if (!key_ready)
    return false;

  int ct_len = strlen(hex_ct) / 2;
  unsigned char *ct = malloc(ct_len);
  unsigned char iv[CRYPTO_IV_LEN];
  unsigned char tag[CRYPTO_TAG_LEN];
  unsigned char *pt = malloc(ct_len + 1);

  hex_to_bytes(hex_ct, ct, ct_len);
  hex_to_bytes(hex_iv, iv, CRYPTO_IV_LEN);
  hex_to_bytes(hex_tag, tag, CRYPTO_TAG_LEN);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  int len, pt_len;

  EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
  EVP_DecryptInit_ex(ctx, NULL, NULL, master_key, iv);
  EVP_DecryptUpdate(ctx, pt, &len, ct, ct_len);
  pt_len = len;
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, CRYPTO_TAG_LEN, tag);

  int ok = EVP_DecryptFinal_ex(ctx, pt + len, &len);
  EVP_CIPHER_CTX_free(ctx);
  free(ct);

  if (ok <= 0)
  {
    free(pt);
    return false;
  }

  pt_len += len;
  pt[pt_len] = 0;
  *out_plain = (char *)pt;

  return true;
}