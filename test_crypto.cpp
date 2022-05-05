// g++ test_crypto.cpp lib_crypto.cpp -std=c++17 -lcrypto
// openssl genrsa -out private.pem 2048
// openssl rsa -in private.pem -outform PEM -pubout -out public.pem

#include <iostream>
#include <string>

#include "lib_crypto.h"

int main() {
  std::string message = "hello, world";
  std::string sig = SignMessage(message, "private.pem");
  std::cout << VerifyMessage(message, sig, "public.pem") << std::endl;
}