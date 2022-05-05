#include "lib_crypto.h"

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>

#include <cassert>

namespace {

// http://hayageek.com/rsa-encryption-decryption-openssl-c/

int padding = RSA_PKCS1_PADDING;

RSA *createRSAWithFilename(const char *filename, int public_key) {
  FILE *fp = fopen(filename, "rb");

  if (fp == NULL) {
    printf("Unable to open file %s \n", filename);
    return NULL;
  }
  RSA *rsa = RSA_new();

  if (public_key) {
    rsa = PEM_read_RSA_PUBKEY(fp, &rsa, NULL, NULL);
  } else {
    rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
  }

  fclose(fp);
  return rsa;
}

int public_encrypt(unsigned char *data, int data_len, char *key_path,
                   unsigned char *encrypted) {
  RSA *rsa = createRSAWithFilename(key_path, 1);
  int result = RSA_public_encrypt(data_len, data, encrypted, rsa, padding);
  RSA_free(rsa);
  return result;
}
int private_decrypt(unsigned char *enc_data, int data_len, char *key_path,
                    unsigned char *decrypted) {
  RSA *rsa = createRSAWithFilename(key_path, 0);
  int result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, padding);
  RSA_free(rsa);
  return result;
}

int private_encrypt(const unsigned char *data, int data_len,
                    const char *key_path, unsigned char *encrypted) {
  RSA *rsa = createRSAWithFilename(key_path, 0);
  int result = RSA_private_encrypt(data_len, data, encrypted, rsa, padding);
  RSA_free(rsa);
  return result;
}
int public_decrypt(const unsigned char *enc_data, int data_len,
                   const char *key_path, unsigned char *decrypted) {
  RSA *rsa = createRSAWithFilename(key_path, 1);
  int result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, padding);
  RSA_free(rsa);
  return result;
}

}  // namespace

std::string SignMessage(const std::string &message,
                        const std::string &private_key_path) {
  assert(message.size() <= 2048 / 8);
  std::string result(4098, '\0');
  int len =
      private_encrypt(reinterpret_cast<const unsigned char *>(message.c_str()),
                      message.size(), private_key_path.c_str(),
                      reinterpret_cast<unsigned char *>(result.data()));
  result.resize(len);
  return result;
}

bool VerifyMessage(const std::string &message, const std::string &signature,
                   const std::string &public_key_path) {
  std::string result(4098, '\0');
  int len =
      public_decrypt(reinterpret_cast<const unsigned char *>(signature.c_str()),
                     signature.size(), public_key_path.c_str(),
                     reinterpret_cast<unsigned char *>(result.data()));
  if (len == -1) return false;
  result.resize(len);
  return result == message;
}
