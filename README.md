# C Networking Examples

## TCP File Transfer

Transfer a plain text or binary file of any size from the client to
the server

## TCP Chat

Simple chat program

## Additional Information

The clients in these examples use "127.0.0.1" for the address and port
8080 as the defaults, but they can be changed at runtime:

    -a <address>
    -p <port>

The servers will most likely bind to "0.0.0.0" and there's no option
to change that yet.

## Compiling

You can compile the c files individually, or use
[meson](https://mesonbuild.com/) to build them all at once:

    meson builddir
    cd builddir
    ninja

## See Also

* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
