#include <kana.h>
#include "kanatypec.h"

//int main(int argc,char*argv[]){
    //ArgsCheck(argc,3); //检查参数合法性

int main(){
    //初始化
    BEGIN:
    char* argv[]={"0","192.168.0.103","58888"};
    int sockFd = socket(AF_INET,SOCK_STREAM,0);//取得socket
    //初始化TCP链接
    struct sockaddr_in addr;
    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(argv[1]);
    addr.sin_port=htons(atoi(argv[2]));
    int ret=connect(sockFd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in));
    ErrorCheck(ret,-1,"connect");
    system("clear");
    puts("Connection success.");
    puts("\033[33mWelcome to ZeDisk! \033[0m\nPlease select one option.");
    puts("[1] Upload\n[2] Download\n[3] exit");
    char opt;
    scanf("%c",&opt);
    //char* dir=getcwd(NULL,0);
    switch (opt)
    {
    case ('1'):
        do_ls("Files");
        puts("Choose the file you want to upload!");
        char filename[20];
        scanf("%s",filename);
        transFile(sockFd,filename);
        break;
    case('2'):
        puts("Here are file list from Server!");
        recvFile(sockFd);
        break;
    case('3'):
        close(sockFd);
        return 0;
        break;
   }
   close(sockFd);
   //recvFile(sockFd);//取得文件
    goto BEGIN;
}

int recvFile(int netFd)
{
    //发送目录请求
    train_t t={1,0,"a"};
    int ret= send(netFd,&t,9,MSG_NOSIGNAL);
    bzero(&t,sizeof(t));
    ret = recvn(netFd,&t,8);
    ret=recvn(netFd,t.buf,t.dataLength);
    printf("%s\n",t.buf);
    //输入文件名并发送给服务器
    puts("Choose the file you want to download!");
    char filename[20];
    char storage[30]="Files/";
    scanf("%s",filename);
    strncat(storage,filename,strlen(filename));
    bzero(&t,sizeof(t));
    t.dataLength=strlen(filename);
    strncat(t.buf,filename,t.dataLength);
    t.type=0;
    ret = send (netFd,&t,8+t.dataLength,MSG_NOSIGNAL);

    bzero(&t, sizeof(t));
    //ret= recvn(netFd, &t, 8);
    //ErrorCheck(ret, -1, "recvn");
    //ret= recvn(netFd, t.buf, t.dataLength);
    //ErrorCheck(ret, -1, "recvn");
    //memcpy(,t.buf,t.dataLength);
    int fd = open(storage, O_RDWR | O_CREAT|O_TRUNC, 0666);
    //ErrorCheck(fd, -1, "open");
    //接收文件大小
    off_t fileSize;
    //int dataSize;
    ret=recvn(netFd,&t,8);
    ErrorCheck(ret, -1, "recvn");
    ret = recvn(netFd, &fileSize, sizeof(off_t));
    ErrorCheck(ret, -1, "recvn");
    //printf("File Size: %ld", fileSize);
    /*
    // 设置进度条
    off_t doneSize=0;
    off_t lastSize=0;
    off_t slice =fileSize/100;

    while (1){
        bzero(&t, sizeof(t));
        recvn(netFd, &t.dataLength, sizeof(int));
        if(t.dataLength==0){
            break;
        }
        //打印进度条
        doneSize+=t.dataLength;
        if(doneSize-lastSize>=slice){
            printf("%5.2lf%%\r",100.0*doneSize/fileSize);
            fflush(stdout);
            lastSize=doneSize;
        }
        recvn(netFd, t.buf, t.dataLength);
        write(fd, t.buf, t.dataLength);
    }
    //保存进度条
    printf("100.00%%\n");
    */
    ftruncate(fd,fileSize);
    char* p=(char*)mmap(NULL,fileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ErrorCheck(p,MAP_FAILED,"mmap");
    //接收文件
    recvn(netFd,p,fileSize);
    printf("File \"%s\" has been received!\n",filename);
    munmap(p,fileSize);
    close(fd);
    return 0;
}
//封装的recvn with SIG_NOWAIT
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

void do_ls(char dirname[])
{
    DIR* dir_ptr;
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
            printf("%s\n", direntp->d_name);
        }
        closedir(dir_ptr);
    }
}

int transFile(int netFd,char filename[])
{   
    train_t t;
    t.dataLength=strlen(filename);
    t.type=1;
    strcpy(t.buf,filename);
    int ret = send(netFd, &t, t.dataLength+2*sizeof(int), MSG_NOSIGNAL);
    ErrorCheck(ret, -1, "Send");
    char storage[30]="Files/";
    strcat(storage,t.buf);
    printf("%s\n",storage);
    int fd = open(storage, O_RDONLY);
    ErrorCheck(fd, -1, "Open");
    bzero(&t, sizeof(t));
    // 获取文件大小
    struct stat statbuf;
    ret =fstat(fd,&statbuf);
    ErrorCheck(ret,-1,"fstat");
    //puts("1");
    t.dataLength=sizeof(off_t);
    memcpy(t.buf,&statbuf.st_size,sizeof(off_t));
    //printf("%s\n",t.buf);
    send(netFd,&t,t.dataLength+2*sizeof(t.dataLength),MSG_NOSIGNAL);
    //sendfile方法
    puts("sendfile");
    sendfile(netFd,fd,NULL,statbuf.st_size);
    //t.dataLength = 0;
    //send(netFd, &t, 4, MSG_NOSIGNAL);
    //munmap(p, statbuf.st_size);
    return 0;
}