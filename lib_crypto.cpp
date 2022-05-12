#include "lib_crypto.h"

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

#include <cassert>

namespace {

// http://hayageek.com/rsa-encryption-decryption-openssl-c/

int padding = RSA_PKCS1_PADDING;

int public_encrypt(unsigned char *data, int data_len, RSA *rsa,
                   unsigned char *encrypted) {
  int result = RSA_public_encrypt(data_len, data, encrypted, rsa, padding);
  return result;
}
int private_decrypt(unsigned char *enc_data, int data_len, RSA *rsa,
                    unsigned char *decrypted) {
  int result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, padding);
  return result;
}

int private_encrypt(const unsigned char *data, int data_len, RSA *rsa,
                    unsigned char *encrypted) {
  int result = RSA_private_encrypt(data_len, data, encrypted, rsa, padding);
  return result;
}
int public_decrypt(const unsigned char *enc_data, int data_len, RSA *rsa,
                   unsigned char *decrypted) {
  int result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, padding);
  return result;
}

}  // namespace

RsaPtr CreateRsa(const std::string &key, bool public_key) {
  BIO *keybio = BIO_new_mem_buf(key.c_str(), -1);
  if (keybio == NULL) {
    printf("Failed to create key BIO");
    return RsaPtr(nullptr, RSA_free);
  }

  RSA *rsa;
  if (public_key) {
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
  } else {
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
  }

  BIO_free(keybio);
  return RsaPtr(rsa, RSA_free);
}

RsaPtr CreateRsaWithFilename(const std::string &filename, bool public_key) {
  FILE *fp = fopen(filename.c_str(), "rb");

  if (fp == NULL) {
    printf("Unable to open file %s \n", filename.c_str());
    return RsaPtr(nullptr, RSA_free);
  }
  RSA *rsa = RSA_new();

  if (public_key) {
    rsa = PEM_read_RSA_PUBKEY(fp, &rsa, NULL, NULL);
  } else {
    rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
  }

  fclose(fp);
  return RsaPtr(rsa, RSA_free);
}

std::string SignMessage(std::string message, RSA *rsa) {
  message = Sha256Sum(message);
  assert(message.size() <= 2048 / 8);
  std::string result(4098, '\0');
  int len = private_encrypt(
      reinterpret_cast<const unsigned char *>(message.c_str()), message.size(),
      rsa, reinterpret_cast<unsigned char *>(result.data()));
  result.resize(len);
  return result;
}

bool VerifyMessage(std::string message, const std::string &signature,
                   RSA *rsa) {
  message = Sha256Sum(message);
  std::string result(4098, '\0');
  int len = public_decrypt(
      reinterpret_cast<const unsigned char *>(signature.c_str()),
      signature.size(), rsa, reinterpret_cast<unsigned char *>(result.data()));
  if (len == -1) return false;
  result.resize(len);
  return result == message;
}

std::string Sha256Sum(const std::string &message) {
  std::string result(SHA256_DIGEST_LENGTH, '\0');
  SHA256(reinterpret_cast<const unsigned char *>(message.c_str()),
         message.size(), reinterpret_cast<unsigned char *>(result.data()));
  return result;
}
