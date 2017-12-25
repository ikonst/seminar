#pragma once

#include <cstdio>
#include <vector>
#include <openssl/ossl_typ.h>

#include "SecureClient.h"

class AuthClient {
public:
    AuthClient(const char *hostname, const char *port);
    virtual ~AuthClient();

    SecureClient Authenticate(const std::string &passpharse);

private:
    size_t ReadIntoBuffer(std::vector<char>& buffer, size_t offset, size_t length);
    void Write(const unsigned char *str, size_t length);

private:
    int _socket;
};
