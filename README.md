This repository demonstrates:

- a C++ client:
  - employing a simple challenge-response authentication schema
  - using OpenSSL to secure the connection after authentication

- a Python server:
  - using `gevent` (Reactor pattern) to allow for a large number of connections (DoS resiliency)
  - implementing secure authentication before SSL to avoid SSL-specific DoS attacks
  - using OpenSSL to secure the connection after authentication
  - accepting arbitrary Python code to execute in a secured fashion
    - communicating over pipes to simplify sandboxing
    - using MacOS sandbox to limit disk and network access
    - using Unix system resource limits to prevent CPU time abuse

# Testing

To ensure server withstands a large number of connections, you can use [tcpkali](https://github.com/satori-com/tcpkali):
```
tcpkali --duration 1000 -c10k -m testing -r 0.5 127.0.0.1:1234
```

# Useful links

* [SPBL, the Apple Sandbox Policy Language](https://www.romab.com/ironsuite/SBPL.html)
* [SSL Renegotiation DOS](https://www.ietf.org/mail-archive/web/tls/current/msg07553.html)
