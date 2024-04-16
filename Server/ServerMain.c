#include "ThreadPool.h"
int exitPipe[2];
//用于退出，让管道读就绪
void sigFunc(int signum){
    printf("sigNum = %d\n",signum);
    write(exitPipe[1],"1",1);
}

int main(){
    char*argv[]={"0","192.168.0.103","58888","4"};
//int main(int argc, char*argv[]){
    //ArgsCheck(argc,4);
    //初始化退出管道
    pipe(exitPipe);
    //
    if(fork()!=0){
        close(exitPipe[0]);
        signal(SIGUSR1,sigFunc);
        wait(NULL);//子进程结束时父进程才会结束
        exit(0);
    }
    close(exitPipe[1]);
    MYSQL *conn;
    SQL_Connect(&conn);
    int workNum=atoi(argv[3]);
    threadPool_t threadPool;
    threadPool_init(&threadPool,workNum);
    makeWorker(&threadPool);
    int sockFd;
    tcpInit(&sockFd,argv[1],argv[2]);
    int epfd=epollCreator();
    epollAdd(sockFd,epfd);
    epollAdd(exitPipe[0],epfd);
    
    struct epoll_event readyArr[SocketNum];
    while(1){
        int readyNum=epoll_wait(epfd,readyArr,SocketNum,-1);
        puts("epoll_wait returns");
        for(int i=0;i<readyNum;i++){
            if(readyArr[i].data.fd==sockFd){
                int netFd=accept(sockFd,NULL,NULL);
                //epollAdd(netFd,epfd);
                pthread_mutex_lock(&threadPool.taskQueue.mutex);
                taskEnqueue(&threadPool.taskQueue,netFd);
                puts("New task allocated!");
                pthread_cond_signal(&threadPool.taskQueue.cond);
                pthread_mutex_unlock(&threadPool.taskQueue.mutex);
            }
            else if(readyArr[i].data.fd==exitPipe[0]){
                puts("Killing threads!");
                threadPool.exitFlag=1;
                pthread_cond_broadcast(&threadPool.taskQueue.cond);

            }
        }
    }
}