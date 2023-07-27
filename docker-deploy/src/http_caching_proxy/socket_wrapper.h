#ifndef SOCKET_WRAPPER
#define SOCKET_WRAPPER

#include <sys/socket.h>
#include <unistd.h>

class SocketWrapper {
public:
    SocketWrapper(int fd): sockFd(fd) {}
    ~SocketWrapper() { close(sockFd); }
    int get() { return sockFd; }

private:
    int sockFd;
};


#endif