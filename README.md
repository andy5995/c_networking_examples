[![C/C++ CI](https://github.com/andy5995/c_networking_examples/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/andy5995/c_networking_examples/actions/workflows/c-cpp.yml)

# C Networking Examples

TCP and UDP examples of networking in C

Website: https://github.com/andy5995/c_networking_examples

## TCP File Transfer

Transfer a plain text or binary file of any size from the client to
the server

## TCP Chat

Simple chat program

## TCP Chat server with multiple connections

Handles multiple connections. The server will tell you when clients
connect and disconnect. Every client will see the message sent by any
other single client. Use any telnet program to connect.

Adapted from [pollserver.c](https://beej.us/guide/bgnet/html/#fnref29)


## UDP echo server/client

The server echoes a message sent by the client back to the client.

    client usage: ./udp_echo_client host port msg...
    server usage: ./udp_echo_server port

## Additional Information

Unless otherwise specified, the clients in these examples use
"127.0.0.1" for the address and port 8080 as the defaults, but they
can be changed at runtime:

    -a <address>
    -p <port>

The servers will most likely bind to "0.0.0.0" and there's no option
to change that yet.

## Compiling

You can compile the c files individually:

    cc -Wall 'example.c' netex.c -o 'example'

or use [meson](https://mesonbuild.com/) to build them all at once:

    meson builddir
    cd builddir
    ninja

## See Also

* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
