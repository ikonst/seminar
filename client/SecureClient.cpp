#include <string>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "SecureClient.h"
#include "Exceptions.h"

using namespace std;

SecureClient::SecureClient(int socket): SecureClient()
{
    this->_socket = dup(socket);
    this->_ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    this->_ssl = SSL_new(this->_ssl_ctx);
    SSL_set_fd(this->_ssl, this->_socket);
    int ret = SSL_connect(this->_ssl);
    if (ret != 1) {
        throw_openssl();
    }
}

SecureClient::SecureClient(): _socket(0), _ssl(nullptr), _ssl_ctx(nullptr) {
}

SecureClient::SecureClient(SecureClient &&other) noexcept
        : SecureClient()
{
    swap(*this, other);
}

SecureClient& SecureClient::operator=(SecureClient &&other) {
    swap(*this, other);
    return *this;
}

SecureClient::~SecureClient() {
    SSL_free(this->_ssl);
    SSL_CTX_free(this->_ssl_ctx);
    if (this->_socket != 0) {
        close(this->_socket);
    }
}

size_t SecureClient::ReadIntoBuffer(vector<char>& buffer, size_t offset, size_t length) {
    if (offset > buffer.size()) {
        throw out_of_range("offset");
    }
    if (buffer.size() - offset < length) {
        throw out_of_range("length");
    }
    buffer.resize(offset + length);
    ssize_t actual = SSL_read(this->_ssl, &buffer[offset], (int)length);
    if (actual < 0) {
        throw_openssl();
    }
    buffer.resize(offset + actual);
    return (size_t)actual;
}

vector<char> SecureClient::Read(size_t length) {
    vector<char> v;
    v.resize(length);
    size_t actual = this->ReadIntoBuffer(v, 0, length);
    v.resize(actual);
    return v;
}

string SecureClient::ReadUntil(char value) {
    stringstream s;
    ssize_t actual;
    char c;
    do {
        actual = SSL_read(this->_ssl, &c, 1);
        if (actual < 0) {
            throw_openssl();
        } else if (actual == 1 && c != value) {
            s << c;
        }
    } while (actual == 1 && c != value);
    return s.str();
}

std::string SecureClient::ReadLine() {
    return this->ReadUntil('\n');
}

void SecureClient::Write(const unsigned char *str, size_t length) {
    ssize_t ret = SSL_write(this->_ssl, str, (int)length);
    if (ret < 0) {
        throw_openssl();
    }
}

void SecureClient::Write(const string &str) {
    ssize_t ret = SSL_write(this->_ssl, str.c_str(), (int)str.size());
    if (ret < 0) {
        throw_openssl();
    }
}

void SecureClient::ExecuteCode(const char *code) {
    this->Write(code);
    this->Write("\nEND CODE\n");
}
