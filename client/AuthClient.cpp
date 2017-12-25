#include <string>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

#include "openssl/sha.h"
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "AuthClient.h"
#include "Exceptions.h"

using namespace std;

struct AddressInfo {
    AddressInfo(const char *hostname, const char *servname, const struct addrinfo *hints = nullptr):
            _addrinfo(nullptr)
    {
        throw_if_nonzero(getaddrinfo(hostname, servname, hints, &_addrinfo));
    }

    addrinfo* operator->() {
        return _addrinfo;
    }

    ~AddressInfo() {
        freeaddrinfo(_addrinfo);
    }

    addrinfo *_addrinfo;
};

AuthClient::AuthClient(const char* hostname, const char *port):
        _socket(0)
{
    this->_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (this->_socket == 0) {
        throw_errno();
    }
    int set = 1;
    setsockopt(this->_socket, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
    AddressInfo addr_info(hostname, port);
    throw_if_nonzero((connect(this->_socket, addr_info->ai_addr, addr_info->ai_addrlen) != 0));
}

AuthClient::~AuthClient() {
    if (this->_socket != 0) {
        close(this->_socket);
    }
}

size_t AuthClient::ReadIntoBuffer(vector<char>& buffer, size_t offset, size_t length) {
    if (offset > buffer.size()) {
        throw out_of_range("offset");
    }
    if (buffer.size() - offset < length) {
        throw out_of_range("length");
    }
    buffer.resize(offset + length);
    ssize_t actual = read(this->_socket, &buffer[offset], length);
    if (actual < 0) {
        throw_openssl();
    }
    buffer.resize(offset + actual);
    return (size_t)actual;
}

void AuthClient::Write(const unsigned char *str, size_t length) {
    ssize_t ret = write(this->_socket, str, (int)length);
    if (ret < 0) {
        throw_openssl();
    }
}

SecureClient AuthClient::Authenticate(const string &passpharse) {
    vector<char> challenge;

    // read the nonce
    challenge.resize(SHA256_DIGEST_LENGTH);
    size_t actual = this->ReadIntoBuffer(challenge, 0, SHA256_DIGEST_LENGTH);
    if (actual != SHA256_DIGEST_LENGTH) {
        throw runtime_error("Invalid challenge");
    }

    // append the passpharse
    challenge.resize(challenge.size() + passpharse.size());
    copy(passpharse.begin(), passpharse.end(), challenge.begin() + SHA256_DIGEST_LENGTH);

    // calculate response
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, &challenge[0], challenge.size());
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    // send the response to the challenge
    this->Write(hash, SHA256_DIGEST_LENGTH);

    return SecureClient(this->_socket);
}
