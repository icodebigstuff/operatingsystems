// http_post_ssl.c: Demosntrates the HTTP POST functionality which
// allows a page to be accessed and some parameters passed to
// it. Compile this program via
// > gcc -Wall -g -o http_post_ssl http_post_ssl.c -lssl -lcrypto
// and run such as:
// > ./http_post_ssl 31
//
// > ./http_post_ssl 85
//
// A web form to execute similar code is here:
// https://www-users.cs.umn.edu/~kauffman/quotes_submit.html

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <assert.h>
#include <arpa/inet.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT "443"                                  // the port client will be connecting to
#define MAXDATASIZE 1024                            // max number of bytes we can get at once

void get_address_string(struct addrinfo *addr, char buffer[]);

int main(int argc, char *argv[]) {
  if(argc < 2){
    printf("usage: %s <NUM>\n",argv[0]);
    return 0;
  }

  char *hostname = "www-users.cs.umn.edu";
  char *hostfile = "/~kauffman/quotes.php";

  printf("Host: %s\nFile: %s\n",hostname, hostfile);


  // Create a socket, connect it to a server, and create an encrypted version
  // of the socket. Later code will use the variables 'sockfd' and
  // 'ssl_connection' for these as was done in http_get_ssl.c
  int ret;
  struct addrinfo *serv_addr;   // filled by getaddrinfo
  int sockfd;                   // created via socket()
  SSL_CTX *ctx;                 // created via SSL functions
  SSL *ssl_connection;          // created via SSL functions
  ret = getaddrinfo(hostname, PORT, NULL, &serv_addr);
  assert(ret == 0);

  sockfd = socket(serv_addr->ai_family,          // create a socket with the appropriate params
                      serv_addr->ai_socktype,        // to the server
                      serv_addr->ai_protocol);
  assert(ret != -1);

  ret = connect(sockfd,                              // connect the socket to the server so that
                serv_addr->ai_addr,                  // writes will send over the network
                serv_addr->ai_addrlen);
  assert(ret != -1);

  char address_str[INET6_ADDRSTRLEN];                // fill in a string version of the addrss which was resolved
  get_address_string(serv_addr, address_str);        // defined below, fills in buffer with printable address


  freeaddrinfo(serv_addr);                           // all done with this structure

  ////////////////////////////////////////////////////////////////////////////////
  // Initialize and set up a secure connection with the SSL library
  OpenSSL_add_all_algorithms();
  SSL_library_init();
  const SSL_METHOD *method = SSLv23_client_method();
  ctx = SSL_CTX_new(method);
  ssl_connection = SSL_new(ctx);
  assert(ssl_connection != NULL);
  SSL_set_fd(ssl_connection, sockfd);
  SSL_connect(ssl_connection);
  // Setup of SSL is complete. The variable 'ssl_connection' is used
  // like a file descriptor with SSL_write() / SSL_read(). The data
  // transmitted will be automatically encrypted/decrypted.
  ////////////////////////////////////////////////////////////////////////////////


  // Format the string `post_data[]` as
  //   quote_num=<NUM>
  // where <NUM> is the 1st command line argument; keep in mind that
  // argv[] elements are strings and that you do not need to do any error checking.
  char post_data[MAXDATASIZE];
  snprintf(post_data,MAXDATASIZE,"quote_num=%s",argv[1]);


  // The basic format of a POST request is below and ends with data in
  // the key/val form like "quote_num=15".  This format string is used
  // to create the message by filling in parameters shown elsewhere.
  char *request_format =
    "POST %s HTTP/1.1\r\n"        // hostfile
    "Host: %s\r\n"                // hostname
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Connection: close\r\n"       // close after answering
    "Content-Length: %lu\r\n\r\n" // length of following key/val pairs
    "%s\r\n\r\n";                 // key/vale data from variable 'post_data'

  char request[MAXDATASIZE];      // print/format the request data
  snprintf(request, MAXDATASIZE, request_format,
           hostfile, hostname, strlen(post_data), post_data);
  printf("-------\n");
  printf("REQUEST\n");
  printf("-------\n");
  printf("%s",request);


  //Write the request[] to the `ssl_connection`.  Use the
  // SSL_write() function which has identical arguments to the
  // standard write() system call.  Use strlen() to calculate the
  // length of the request.
  int nbytes;
  nbytes = SSL_write(ssl_connection, request, strlen(request));
  assert(nbytes == strlen(request));



  // Receive response data and write() it to the screen. Use the
  // `response[]` buffer below.  Use the SSL_read() function which has
  // identical semantics to read().  Note that responses can be
  // arbitrarily long so it is typical to use a loop to read()
  // data. However, for this simple example, responses are short and
  // can likely be read with a single pass.
  char response[MAXDATASIZE];
  printf("--------\n");
  printf("RESPONSE\n");
  printf("--------\n");

  while(1){
    int nbytes =                                     // receive data data from the server
      SSL_read(ssl_connection, response, MAXDATASIZE);
    assert(nbytes != -1);
    if(nbytes == 0){
      break;
    }
    write(STDOUT_FILENO, response, nbytes);
  }

  printf("-------\n");            // End of response

  SSL_free(ssl_connection);       // cleans up SSL variables
  SSL_CTX_free(ctx);
  close(sockfd);                  // closes/de-allocates socket

  return 0;
}

void get_address_string(struct addrinfo *addr, char buffer[]){
  void *ip_address = NULL;
  if(addr->ai_family == AF_INET){               // ipv4 address
    ip_address = (void *) &((struct sockaddr_in*)addr->ai_addr)->sin_addr;
  }
  else if(addr->ai_family == AF_INET6){         // ipv6 address
    ip_address = (void *) &((struct sockaddr_in6*)addr->ai_addr)->sin6_addr;
  }
  inet_ntop(addr->ai_family, ip_address, buffer, INET6_ADDRSTRLEN);
}
