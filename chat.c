#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define ADDRS "127.0.0.1" //服务器的IP
#define PORT 8888         //服务器服务端
int clientfd2;            //客户端socket
typedef struct sockaddr meng;
char name[30]; //设置支持的用户名长度
time_t nowtime;
char filename[10];
int a;
char send_buf[4096];
char recv_buf[4096];

void recv_file()
{
    FILE *fp = NULL;
    int ret = -1;
    while (1)
    {
        //3.使用send函数发生数据
        printf("请输入要发送给服务器的内容: \n");
        scanf("%s", send_buf);
        if (!strncmp(send_buf, "qu", 3))
            break;
        fp = fopen(send_buf, "w");
        if (fp == NULL)
        {
            printf("File:\t%s Can Not Open To Write!\n", send_buf);
            _exit(-1);
        }
        printf("File:\t%s Open Success,Waitting To Write...\n", send_buf);

        ret = send(clientfd2, send_buf, strlen(send_buf), 0);
        printf("send buffer:%s,sned len:%d\n", send_buf, ret);

        //4.使用recv函数接收来自服务端的消息
        ret = recv(clientfd2, recv_buf, sizeof(recv_buf), 0);
        if (ret < 1)
        {
            printf("服务器断开了连接\n");
            break;
        }
        printf("收到服务器发送的数据:recv buffer:\n%s\nrecv len:%d\n", recv_buf, ret);
        printf("将接收到的数据写入文件中:\n");
        //调用fwrite函数将recv_buf缓存中的数据写入文件中
        int write_length = fwrite(recv_buf, sizeof(char), ret, fp);
        if (write_length < ret)
        {
            printf("文件写入失败!\n");
            break;
        }
        printf("Recieve File:\t %s From Server[%s] 接收成功!\n", send_buf, ADDRS);
        memset(send_buf, 0, sizeof(send_buf)); //清空接收缓存区
        memset(recv_buf, 0, sizeof(recv_buf)); //清空接收缓存区
        fclose(fp);
    }
}

void *recv_thread(void *p)
{
    while (1)
    {
        char buf[100] = {0};
        if (recv(clientfd2, buf, sizeof(buf), 0) <= 0)
        {
            break;
        }
        printf("%s\n", buf);
    }
}

void start()
{
    pthread_t id;
    void *recv_thread(void *);
    //创建一个线程用于数据的接收，一个用于数据的发送
    pthread_create(&id, 0, recv_thread, 0);
    char buf2[100] = {0};
    sprintf(buf2, "%s进入了群聊", name);
    time(&nowtime);
    printf("进入的时间是: %s\n", ctime(&nowtime));
    send(clientfd2, buf2, strlen(buf2), 0);
    while (1)
    {
        char buf[100] = {0};
        scanf("%s", buf);
        char msg[100] = {0};
        sprintf(msg, "%s发送的信息是:%s", name, buf);
        send(clientfd2, msg, strlen(msg), 0);

        if (strcmp(buf, "q") == 0)
        {
            memset(buf2, 0, sizeof(buf2)); //初始化
            sprintf(buf2, "%s退出了群聊", name);
            send(clientfd2, buf2, strlen(buf2), 0);
            break;
        }
    }
    recv_file();
    close(clientfd2);
}

int main(int argc,char *argv[])
{
    clientfd2 = socket(AF_INET, SOCK_STREAM, 0); //创建套接字
    struct sockaddr_in addr;                     //将套接字存在sockaddr_in结构体中
    addr.sin_family = AF_INET;                   //地址族
    addr.sin_port = htons(PORT);                 //端口号 可随意设置，不过不可超过规定的范围
    addr.sin_addr.s_addr = inet_addr(argv[1]);     //inet_addr()函数将点分十进制的字符串转换为32位的网络字节顺序的ip信息
                                                 //发起连接
    if (connect(clientfd2, (meng *)&addr, sizeof(addr)) == -1)
    {
        perror("无法连接到服务器");
        exit(-1);
    }
    printf("客户端启动成功\n");

    printf("请输入用户名：");
    scanf("%s", name);
    printf("\n\n*****************************\n");
    printf("欢迎%s 进入群聊\n", name);
    printf("  输入q退出\n");
    printf("\n*****************************\n\n");
    start();

    return 0;
}