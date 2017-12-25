#include <iostream>
#include <openssl/ssl.h>

#include "AuthClient.h"
#include "Tests.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "Usage: client [hostname] [port]" << endl;
        return 1;
    }

    if (SSL_library_init() < 0) {
        cout << "Failed to initialize OpenSSL" << endl;
        return 1;
    }
    SSL_load_error_strings();

    try {
        AuthClient authClient(argv[1], argv[2]);
        auto secureClient = authClient.Authenticate("foobar");
        test_restrict_network_io(secureClient);
        // test_unbuffered_io(secureClient);
    } catch (const runtime_error& ex) {
        cout << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}