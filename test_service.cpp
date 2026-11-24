#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#define DBG(x) std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] " \
                           << #x << " = " << (x) << "\n"

using namespace std;


class TestClient {
  public:
  int fd;

  TestClient() {
    const char *host = "127.0.0.1";
    int port         = 8080;

    addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(host, std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
      throw std::runtime_error("cannot create client (get addr info)");
    }

    fd = -1;
    for (addrinfo *p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) { 
      fprintf(stderr, "[%s:%d] connect failed\n", __FILE__, __LINE__);
      throw std::runtime_error("cannot create client");
    }
  }

  ~TestClient() {
    close(fd);
  }

  int send_message(string_view sv) {
    string msg = "test msg ";
    msg += sv;
    return write(fd, msg.c_str(), strlen(msg.c_str()));
  }
};


int main(int argc, char **argv) {

  for (int i = 0; i < 1 << 16; ++i) {
    TestClient tc;
    tc.send_message(to_string(i));
  }












    return 0;
}
