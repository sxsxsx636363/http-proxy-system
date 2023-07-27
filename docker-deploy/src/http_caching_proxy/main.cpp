#include "http_server.h"
#include "logger.h"

int main() {
    // Start creating a daemon
    // reference: https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149
    pid_t pid;
    /* Fork off the parent process */
    pid = fork();
    /* An error occurred */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    /* On success: The child process becomes session leader */
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    /* Catch, ignore and handle signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    /* Fork off for the second time*/
    pid = fork();
    /* An error occurred */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    /* Set new file permissions */
    umask(0);
    /*Point stdin/err/out at /dev/null*/
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");
    /* Close all open file descriptors */
    // int x;
    // for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
    //     close (x);
    // }

    HttpServer* server = new HttpServer();
    server->startServer();

    return 0;
}