 typedef struct train_s{
    int dataLength;
    int type;
    char buf[1024];
 }train_t;

 int recvFile(int netFd);
 int recvn(int netFd,void*pstart,int len);

 void do_ls(char dirname[]);
 int transFile(int netFd,char filename[]);