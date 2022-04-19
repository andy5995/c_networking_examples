# C Networking Examples

## TCP File Transfer

To compile:

    cc -Wall example_tcp_file_transfer_client.c -o example_tcp_file_transfer_client
    cc -Wall example_tcp_file_transfer_server.c -o example_tcp_file_transfer_server

Or use meson:

    meson builddir
    cd builddir
    ninja
