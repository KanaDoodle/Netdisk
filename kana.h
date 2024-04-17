#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <strings.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <mysql/mysql.h>
#include <dirent.h>
#include <termios.h>

#define N 4096

#define ArgsCheck(argc, num)                \
    {                                       \
        if (argc != num)                    \
        {                                   \
            fprintf(stderr, "args error!"); \
            return -1;                      \
        }                                   \
    }
#define ErrorCheck(ret, num, msg) \
    {                             \
        if (ret == num)           \
        {                         \
            perror(msg);          \
            return -1;            \
        }                         \
    }
#define ReturnCheck(ret, msg)        \
    {                                \
        if (ret == 0)                \
        {                            \
            puts("Chat is over!\n"); \
            break;                   \
        }                            \
    }

#define ThreadCheck(ret, msg)                             \
    {                                                     \
        if (ret != 0)                                     \
        {                                                 \
            fprintf(stderr, "%s:%s", msg, strerror(ret)); \
        }                                                 \
    }
