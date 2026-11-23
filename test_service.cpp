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

int main(int argc, char **argv) {
    const char *host = "127.0.0.1";
    int port         = 8080;
    const char *msg  = "hello socket\n";


    addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(host, std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        return 1;
    }

    int fd = -1;
    for (addrinfo *p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) { fprintf(stderr, "[%s:%d] connect failed\n", __FILE__, __LINE__); return 1; }




    return 0;
}
