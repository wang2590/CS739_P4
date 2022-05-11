// g++ test_crypto.cpp lib_crypto.cpp -std=c++17 -lcrypto
// openssl genrsa -out private.pem 2048
// openssl rsa -in private.pem -outform PEM -pubout -out public.pem

#include <iomanip>
#include <iostream>
#include <string>

#include "lib_crypto.h"

int main() {
  std::string message = "hello, world";
  RsaPtr priv = CreateRsaWithFilename("private.pem", false);
  RsaPtr pub = CreateRsaWithFilename("public.pem", true);
  std::string sig = SignMessage(message, priv.get());
  std::cout << VerifyMessage(message, sig, pub.get()) << std::endl;

  std::string digest = Sha256Sum(message);
  for (char c : digest) {
    std::cout << std::setw(2) << std::setfill('0') << std::hex
              << (int)(unsigned char)c;
  }
  std::cout << std::endl;
}