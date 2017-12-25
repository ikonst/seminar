#pragma once

#include <cstdio>
#include <vector>
#include <openssl/ossl_typ.h>

class SecureClient {
public:
    explicit SecureClient(int socket);
    SecureClient(SecureClient &&other) noexcept;
    SecureClient& operator=(SecureClient && other);
    virtual ~SecureClient();

    size_t ReadIntoBuffer(std::vector<char>& buffer, size_t offset, size_t length);
    std::vector<char> Read(size_t length);
    std::string ReadUntil(char c);
    std::string ReadLine();
    void Write(const unsigned char *str, size_t length);
    void Write(const std::string &str);
    void ExecuteCode(const char *code);

private:
    SecureClient();

private:
    int _socket;
    SSL_CTX *_ssl_ctx;
    SSL *_ssl;
};
