#include "Exceptions.h"

#include <cstring>
#include <stdexcept>
#include <sys/errno.h>

#include <openssl/err.h>

using namespace std;

void throw_if_nonzero(int result) {
    if (result != 0) {
        throw_errno();
    }
}

void throw_errno() {
    if (errno != 0) {
        throw runtime_error(strerror(errno));
    }
}

void throw_openssl() {
    auto err = ERR_get_error();
    const char *str = ERR_error_string(err, nullptr);
    throw runtime_error(str ? str : "unknown OpenSSL error");
}
