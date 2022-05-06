#ifndef LIB_CRYPTO_H
#define LIB_CRYPTO_H

#include <string>

std::string SignMessage(const std::string &message,
                        const std::string &private_key_path);

bool VerifyMessage(const std::string &message, const std::string &signature,
                   const std::string &public_key_path);

std::string Sha256Sum(const std::string &message);

#endif
