#include "ThreadPool.h"

int taskEnqueue(taskQueue_t *pTaskQueue,int netFd){
    task_t *pTask= (task_t*)calloc(1,sizeof(task_t));
    pTask->netFd=netFd;
    if(pTaskQueue->size==0){
        pTaskQueue->pFront=pTask;
        pTaskQueue->pRear=pTask;
    }
    else{
        pTaskQueue->pRear->pNext=pTask;
        pTaskQueue->pRear=pTask;
    }
    ++pTaskQueue->size;
    return 0;
}

int taskDequeue(taskQueue_t *pTaskQueue){
    task_t *pCur = pTaskQueue->pFront;
    pTaskQueue->pFront=pCur->pNext;
    free(pCur);
    --pTaskQueue->size;
    return 0;
}

int threadPool_init(threadPool_t *pThreadPool, int workerNum){
    pThreadPool->threadNum=workerNum;
    pThreadPool->tid=(pthread_t*)calloc(workerNum,sizeof(pthread_t));
    pThreadPool->taskQueue.pFront=NULL;
    pThreadPool->taskQueue.pRear=NULL;
    pThreadPool->taskQueue.size=0;
    pthread_mutex_init(&pThreadPool->taskQueue.mutex,NULL);
    pthread_cond_init(&pThreadPool->taskQueue.cond,NULL);
    pThreadPool->exitFlag=0;

}

int tcpInit(int *pSockFd,char* ip,char*port){
    *pSockFd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(atoi(port));
    //以下用于设置在TIME_WAIT时间依然可以使用重复地址，提高效率
    int reuse =1;
    int ret;
    ret =setsockopt(*pSockFd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ErrorCheck(ret,-1,"setsockopt");
    ret=bind(*pSockFd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in));
    ErrorCheck(ret,-1,"bind");
    listen(*pSockFd,10);
    return 0;
}

int epollCreator(){
    int epfd = epoll_create(1);
    ErrorCheck(epfd,-1,"epoll_create");
    return epfd;
}

int epollAdd(int fd,int epfd){
    struct epoll_event event;
    event.events=EPOLLIN;
    event.data.fd=fd;
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    ErrorCheck(ret,-1,"epoll_add");
    return 0;
}

int epollDel(int fd,int epfd){
    int ret =epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
    ErrorCheck(ret,-1,"epoll del");
    return 0;
}

int transFile(int netFd)
{
    train_t t;
    int ret=recvn(netFd,&t,8);
    ErrorCheck(ret,-1,"recvn");
    ret=recvn(netFd,&t.buf,t.dataLength);
    ErrorCheck(ret,-1,"recvn");
    if(t.type==1){
        //接收文件名字
        char filename[20]="";
        char storage[30]="MainStorage/";
        off_t filesize;
        strncat(filename,t.buf,t.dataLength);
        bzero(&t,sizeof(t));
        recvn(netFd,&t,8);
        recvn(netFd,&filesize,t.dataLength);
        strcat(storage,filename);
        //printf("%ld\n",filesize);
        int fd=open(storage,O_RDWR|O_CREAT|O_TRUNC,0666);
        ErrorCheck(fd,-1,"open");
        ftruncate(fd,filesize);
        char* p=(char*)mmap(NULL,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        ErrorCheck(p,MAP_FAILED,"mmap");
        recvn(netFd,p,filesize);
        printf("File \"%s\" has been received!\n",filename);
        munmap(p,filesize);
        close(fd);
        return 0;


    }
    else{
        //发送目录下的文件
        bzero(&t,sizeof(t));
        //char* list;
        //list=Server_ls("MainStorage");
        strncat(t.buf,Server_ls("MainStorage"),strlen(Server_ls("MainStorage")));
        printf("%s\n",t.buf);
        t.dataLength=sizeof(t.buf);
        t.type=0;
        ret = send(netFd, &t, t.dataLength+2*sizeof(int), MSG_NOSIGNAL);
        ErrorCheck(ret, -1, "Send");
        //接收需要下载的文件名
        bzero(&t,sizeof(t));
        ret= recvn(netFd,&t,8);
        ret=recvn(netFd,t.buf,t.dataLength);
        char storage[30]="MainStorage/";
        strncat(storage,t.buf,t.dataLength);
        // 打开文件
        int fd = open(storage, O_RDONLY);
        ErrorCheck(fd, -1, "Open");
        bzero(&t, sizeof(t));
        // 获取文件大小
        struct stat statbuf;
        ret =fstat(fd,&statbuf);
        ErrorCheck(ret,-1,"fstat");
        t.dataLength=sizeof(off_t);
        memcpy(t.buf,&statbuf.st_size,sizeof(off_t));
        send(netFd,&t,t.dataLength+2*sizeof(t.dataLength),MSG_NOSIGNAL);
    /*传统方法
    bzero(&t, sizeof(t));
    while (1)
    {
        t.dataLength=read(fd,t.buf,sizeof(t.buf));
        ErrorCheck(t.dataLength,-1,"read");
        if(t.dataLength!=sizeof(t.buf)){
            printf("DataLength = %d\n",t.dataLength);
        }
        if(t.dataLength==0){
            bzero(&t,sizeof(t));
            send(netFd,&t,sizeof(int),MSG_NOSIGNAL);
            break;
        }
        int ret = send(netFd,&t,sizeof(int)+t.dataLength,MSG_NOSIGNAL);
        if(ret ==-1){
            perror("send");
            break;
        }
    }
    close(fd);
    return 0;
    */
   /*mmap方法
    char*p=(char*)mmap(NULL,statbuf.st_size,PROT_READ,MAP_SHARED,fd,0);
    ErrorCheck(p, MAP_FAILED, "mmap");
    off_t total = 0;
    while (total < statbuf.st_size)
    {
        bzero(&t,sizeof(t));
        if (statbuf.st_size - total > sizeof(t.buf))
        {
            t.dataLength = sizeof(t.buf);
        }
        else
        {
            t.dataLength = statbuf.st_size - total;
        }
        memcpy(t.buf, p + total, t.dataLength);
        total += t.dataLength;
        int ret = send(netFd, &t, sizeof(int) + t.dataLength, MSG_NOSIGNAL);
        if (ret == -1)
        {
            perror("send");
            break;
        }
    }*/
    //sendfile方法
    puts("sendfile");
    sendfile(netFd,fd,NULL,statbuf.st_size);
    //t.dataLength = 0;
    //send(netFd, &t, 4, MSG_NOSIGNAL);
    //munmap(p, statbuf.st_size);
    close(fd);
    }
    return 0;
}

int recvn(int netFd, void*pstart,int len){
    int total=0;
    int ret;
    char*p=(char*)pstart;
    while(total<len){
        ret =recv(netFd,p+total,len-total,0);
        total+=ret;
    }
    return 0;
}

//文件系统命令
char* Server_ls(char dirname[])
{
    DIR* dir_ptr;
    static char list[1024]="";
    struct dirent *direntp;

    if ((dir_ptr = opendir(dirname)) == NULL)
    {
        fprintf(stderr, "ls1: cannot open %s\n",dirname);
    }
    else
    {
        while((direntp = readdir(dir_ptr)) != NULL) 
        {
            //printf("direntp->d_name = %s\n",direntp->d_name);
            if(    strcmp(direntp->d_name, ".") != 0 &&
                strcmp(direntp->d_name, "..") != 0)
            //printf("%s\n", direntp->d_name);
            strncat(list,direntp->d_name,strlen(direntp->d_name));
            strcat(list," ");
        }
        closedir(dir_ptr);
    }
    return list;
}