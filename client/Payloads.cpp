#include <string>
#include <sstream>
#include <fstream>

using namespace std;

string load_payload(const char *payload_name) {
    ifstream f;
    auto filename = string("payloads/") + payload_name + ".py";
    f.open(filename);
    if (!f.is_open()) {
        throw runtime_error("Unable to open payload " + filename);
    }
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}
