#include <kana.h>
#define SocketNum 50
//任务结点
typedef struct task_s{
    int netFd;
    struct task_s *pNext;
}task_t;
//任务队列
typedef struct taskQueue_s{
    task_t *pFront;
    task_t *pRear;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}taskQueue_t;
//线程池
typedef struct threadPool_s{
    pthread_t *tid;
    int threadNum;
    taskQueue_t taskQueue;
    int exitFlag;
}threadPool_t;
//文件小火车
 typedef struct train_s{
    int dataLength;
    int type;
    char buf[1024];
 }train_t;

int taskEnqueue(taskQueue_t *pTaskQueue,int netFd);
int taskDequeue(taskQueue_t *pTaskQueue);
int threadPool_init(threadPool_t *pThreadPool, int workerNum);
int makeWorker(threadPool_t*pThreadPool);
int tcpInit(int *pSockFd,char* ip,char*port);
int epollCreator();
int epollAdd(int fd,int epfd);
int epollDel(int fd,int epfd);
int transFile(int netFd);
int SQL_Connect(MYSQL **conn);
int recvn(int netFd, void*pstart,int len);
char* Server_ls(char dirname[]);