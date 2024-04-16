#include "ThreadPool.h"
void* handleEvent(void*arg){
    threadPool_t*pThreadPool=(threadPool_t*)arg;
    int netFd;
    while(1){
        pthread_mutex_lock(&pThreadPool->taskQueue.mutex);
        while(pThreadPool->taskQueue.size==0&&pThreadPool->exitFlag==0){
            pthread_cond_wait(&pThreadPool->taskQueue.cond,&pThreadPool->taskQueue.mutex);
        }
        if(pThreadPool->exitFlag!=0){
            pthread_exit(NULL);
        }
        netFd=pThreadPool->taskQueue.pFront->netFd;
        taskDequeue(&pThreadPool->taskQueue);
        pthread_mutex_unlock(&pThreadPool->taskQueue.mutex);
        puts("I'm working!");
        transFile(netFd);//根据控制内容选择任务
        puts("transmission success");
        close(netFd);

    }
}
int makeWorker(threadPool_t*pThreadPool){
    for(int i=0; i<pThreadPool->threadNum;++i){
        pthread_create(&pThreadPool->tid[i],NULL,handleEvent,(void*)pThreadPool);

    }
}