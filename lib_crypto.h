#ifndef LIB_CRYPTO_H
#define LIB_CRYPTO_H

#include <openssl/rsa.h>

#include <memory>
#include <string>

#include "common.pb.h"

typedef std::unique_ptr<RSA, decltype(RSA_free) *> RsaPtr;

RsaPtr CreateRsa(const std::string &key, bool public_key);

RsaPtr CreateRsaWithFilename(const std::string &filename, bool public_key);

std::string SignMessage(std::string message, RSA *rsa);

template <class T>
int SignMessage(const T &proto_cmd, RSA *rsa, common::SignedMessage *result) {
  std::string serilized_cmd = proto_cmd.SerializeAsString();
  if (serilized_cmd == "") return -1;

  result->set_message(serilized_cmd);
  result->set_signature(SignMessage(serilized_cmd, rsa));

  return 0;
}

bool VerifyMessage(std::string message, const std::string &signature, RSA *rsa);

template <class T>
bool VerifyAndDecodeMessage(const common::SignedMessage &message, RSA *rsa,
                            T *result) {
  bool res = VerifyMessage(message.message(), message.signature(), rsa);
  if (!res) return false;
  if (!result->ParseFromString(message.message())) return false;
  return true;
}

std::string Sha256Sum(const std::string &message);

#endif
