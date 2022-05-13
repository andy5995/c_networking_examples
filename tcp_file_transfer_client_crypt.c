/*
 tcp_file_transfer_client.c
 https://github.com/andy5995/c_networking_examples

 MIT License

 Copyright (c) 2022 Andy Alt and James Sherratt

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*/

#include <errno.h>
#include <libgen.h>             // basename()
#include <limits.h>
// #include <openssl/ssl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "netex.h"

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int
func(int sockfd, const char *file)
{
  FILE *fp = fopen(file, "rb");
  if (fp == NULL)
  {
    strerror(errno);
    exit(errno);
  }
  int r = fseek(fp, 0, SEEK_END);
  if (r != 0)
  {
    strerror(errno);
    exit(errno);
  }
  long len = ftell(fp);
  printf("file length: %li\n", len);
  rewind(fp);

  // basename() may modify the contents of 'file', so create a copy
  char file_orig[PATH_MAX];
  if ((size_t) snprintf(file_orig, sizeof file_orig, "%s", file) >=
      sizeof file_orig)
    fputs("filename truncated", stderr);

  char *file_basename = basename(file_orig);
  printf("Sending %s...\n", file);

  send(sockfd, file_basename, strlen(file_basename) + 1, 0);
  char buff[BUFSIZ];

  do
  {
    size_t num = fread(buff, 1, sizeof(buff), fp);
    if (ferror(fp) != 0)
    {
      fputs("error: fread", stderr);
      exit(-1);
    }
    int rsend = tcp_sendall(sockfd, buff, &num);
    if (rsend == -1)
      return rsend;
    printf("bytes sent: %li\r", num);
  }
  while (feof(fp) == 0);

  putchar('\n');
  bzero(buff, sizeof(buff));
  fputs("Server replied: ", stdout);
  int n_bytes_recvd;
  while ((n_bytes_recvd = recv(sockfd, buff, sizeof(buff), 0)) != 0)
  {
    fputs(buff, stdout);
    *buff = '\0';
  }

  if (n_bytes_recvd < 0)
    perror("recv() failed");

  if (fclose(fp) == EOF)
  {
    perror("fclose");
  }

  return strstr(buff, "already exists") != NULL;
}


static void
show_usage(const char *prgname)
{
  printf("Usage: %s [OPTIONS]\n\n", prgname);
  puts("\
  -a <address>\n\
  -p <port>\n\
  -f <file>\n");
  return;
}


int
main(int argc, char *argv[])
{
  int opt;
  char *file = NULL;

  while ((opt = getopt(argc, argv, "f:a:p:h")) != -1)
  {
    switch (opt)
    {
    case 'f':
      file = optarg;
      break;
    case 'p':
      conn_inf.port = optarg;
      break;
    case 'a':
      conn_inf.host = optarg;
      break;
    case 'h':
    default:
      show_usage(argv[0]);
      return 0;
    }
  }

  if (file == NULL)
  {
    fputs("A file must be specified (-f <filename>)\n", stderr);
    exit(EXIT_FAILURE);
  }

  int res = get_tcp_client_sockfd();
  if (res < 0)
    return res;

  int f_exists = func(conn_inf.sockfd, file);

  puts("\nClosing socket");
  if (close(conn_inf.sockfd) != 0)
    perror("close() failed");

  return f_exists;
}
