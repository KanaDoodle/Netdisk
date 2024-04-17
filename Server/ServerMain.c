#include "ThreadPool.h"
int exitPipe[2];
//用于退出，让管道读就绪
void sigFunc(int signum){
    printf("sigNum = %d\n",signum);
    write(exitPipe[1],"1",1);
}

//int main(int argc, char*argv[]){
    //ArgsCheck(argc,4);
int main(){
    char*argv[]={"0","172.16.0.3","58888","4"};
    pipe(exitPipe);
    // 初始化退出管道
    if(fork()!=0){
        close(exitPipe[0]);
        signal(SIGUSR1,sigFunc);
        wait(NULL);//子进程结束时父进程才会结束
        exit(0);
    }
    close(exitPipe[1]);

    //初始化MYSQL
    /*
    MYSQL *conn;
    SQL_Connect(&conn);
     */
    int workNum=atoi(argv[3]);

    //初始化线程池
    threadPool_t threadPool;
    threadPool_init(&threadPool,workNum);
    makeWorker(&threadPool);

    //初始化TCP与epoll
    int sockFd;
    tcpInit(&sockFd,argv[1],argv[2]);
    int epfd=epollCreator();
    epollAdd(sockFd,epfd);//将监听的sockFd加入epoll
    epollAdd(exitPipe[0],epfd);//将退出管道加入epoll
    
    //epoll主体
    struct epoll_event readyArr[SocketNum];
    while(1){
        int readyNum=epoll_wait(epfd,readyArr,SocketNum,-1);
        puts("epoll_wait returns");
        for(int i=0;i<readyNum;i++){
            //新连接请求
            if(readyArr[i].data.fd==sockFd){
                int netFd=accept(sockFd,NULL,NULL);
                epollAdd(netFd,epfd);
                printf("New connection estabilished with Fd:%d\n",netFd);
            }
            //退出管道就绪
            else if(readyArr[i].data.fd==exitPipe[0]){
                puts("Killing threads!");
                threadPool.exitFlag=1;
                pthread_cond_broadcast(&threadPool.taskQueue.cond);

            }
            //有已链接的netFd读就绪
            else{
                int netFd=readyArr[i].data.fd;
                pthread_mutex_lock(&threadPool.taskQueue.mutex);
                taskEnqueue(&threadPool.taskQueue,netFd);
                puts("New task allocated!");
                pthread_cond_signal(&threadPool.taskQueue.cond);
                pthread_mutex_unlock(&threadPool.taskQueue.mutex);
            }
        }
    }
}