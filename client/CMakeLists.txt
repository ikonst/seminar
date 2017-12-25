cmake_minimum_required(VERSION 3.9)
project(client)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(client
        main.cpp
        Payloads.cpp
        Exceptions.cpp
        AuthClient.cpp
        SecureClient.cpp
        Payloads.cpp
        Tests.cpp)
target_link_libraries(client ${OPENSSL_LIBRARIES})
