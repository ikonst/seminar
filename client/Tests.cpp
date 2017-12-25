#include <iostream>
#include <sstream>
#include <string>

#include "Payloads.h"
#include "SecureClient.h"

using namespace std;

void test_unbuffered_io(SecureClient &client) {
    cout << "Starting unbuffered_io test" << endl;
    string s = load_payload("unbuffered_io");
    client.ExecuteCode(s.c_str());

    int counter = 0;
    do {
        ++counter;

        stringstream out_ss;
        out_ss << counter << endl;
        client.Write(out_ss.str());
        cout << "Wrote " << counter << endl;

        string in = client.ReadLine();
        stringstream in_ss(in);
        in_ss >> counter;
        cout << "Got " << counter << endl;
    } while (counter < 10);

    cout << "Finished unbuffered_io test with value " << counter << endl;
}

void test_restrict_network_io(SecureClient &client) {
    cout << "Starting restrict_network_io test" << endl;
    string s = load_payload("restrict_network_io");
    client.ExecuteCode(s.c_str());

    cout << "Finished restrict_network_io test: " << client.ReadLine() << endl;
}
