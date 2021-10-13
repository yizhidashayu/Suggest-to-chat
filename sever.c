#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <unistd.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include <pthread.h>

#include <time.h>

#define ADDRS "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 4096
#define FILE_NAME_MAX_SIZE 512

int serverfd;
int clientfd[100];
int size = 50;
typedef struct sockaddr meng;
time_t nowtime;
char filename[10];
int fds[100];

void init()
{
    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverfd == -1)
    {
        perror("创建socket失败");
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ADDRS);

    if (bind(serverfd, (meng *)&addr, sizeof(addr)) == -1)
    {
        perror("bind error\n");
        exit(-1);
    }

    if (listen(serverfd, 100) == -1)
    { //监听最大连接数

        perror("listen error\n");

        exit(-1);
    }
}

void SendAll()
{
    FILE *fp = NULL;
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    int new_server_socket = accept(serverfd, (struct sockaddr *)&client_addr, &length);
    if (new_server_socket < 0)
    {
        printf("Server Accept Failed!\n");
        exit(1);
    }
    printf("listen连接客户端成功,new_server_socket = %d\n", new_server_socket);
    printf("客户端ip =  %s\n", inet_ntoa(client_addr.sin_addr));
    printf("客户端端口号port = %d\n", ntohs(client_addr.sin_port));
    // 服务器端一直运行用以持续为客户端提供服务
    char buffer[BUFFER_SIZE];
    while (1)
    {
        bzero(buffer, sizeof(buffer));
        printf("等待客户端发送过来的文件名：\n");
        length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
        if (length > 0)
        {
            printf("接收client消息成功:'%s'，共%d个字节的数据\n", buffer, length);
        }
        else
        {
            printf("客户端断开了连接，退出！！！\n");
            break;
        }
        char file_name[FILE_NAME_MAX_SIZE + 1];
        bzero(file_name, sizeof(file_name));
        strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));

        fp = fopen(file_name, "r");
        if (fp == NULL)
        {
            printf("File:\t%s Not Found!\n", file_name);
        }
        else
        {
            printf("File:\t%s open success!\n", file_name);
            bzero(buffer, BUFFER_SIZE);
            int file_block_length = 0;
            //循环将文件file_name(fp)中的内容读取到buffer中
            while ((file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
            {
                printf("读取到的文件长度file_block_length = %d\n", file_block_length);

                // 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
                if (send(new_server_socket, buffer, file_block_length, 0) < 0)
                {
                    printf("Send File:\t%s Failed!\n", file_name);
                    break;
                }
                //清空buffer缓存区
                bzero(buffer, sizeof(buffer));
            }
            fclose(fp); //关闭文件描述符fp
            printf("File:\t%s Transfer Finished!\n", file_name);
        }
    }
    close(new_server_socket); //关闭accept文件描述符
    close(serverfd);
}

void *server_thread(void *p)
{
    int fd = *(int *)p;
    printf("pthread = %d\n", fd);
    while (1)
    {
        char buf[100] = {0};
        if (recv(fd, buf, sizeof(buf), 0) <= 0)
        {
            int i;
            for (i = 0; i < size; i++)
            {
                if (fd == clientfd[i])
                {
                    clientfd[i] = 0;
                    break;
                }
            }
            printf("退出：fd = %d 退出了。\n", fd);

            char buf[1024];

            FILE *logs = fopen("log.txt", "a");

            if (logs == NULL)
            {
                printf("open file erroe: \n");
            }
            else
            {
                sprintf(buf, "退出时间：%s\t\n", ctime(&nowtime));
                fputs(buf, logs);
                fclose(logs);
            }
            pthread_exit(0);
        }
        SendAll(buf);
    }
}

void server()
{
    printf("服务器启动\n");
    while (1)
    {
        struct sockaddr_in fromaddr;
        socklen_t len = sizeof(fromaddr);
        int fd = accept(serverfd, (meng *)&fromaddr, &len);
        //调用accept进入堵塞状态，等待客户端的连接
        if (fd == -1)
        {
            printf("客户端连接出错...\n");
            continue;
        }
        int i = 0;
        for (i = 0; i < size; i++)
        {
            if (clientfd[i] == 0)
            {
                //记录客户端的socket
                clientfd[i] = fd;
                printf("线程号= %d\n", fd); 
                //有客户端连接之后，启动线程给此客户服务
                pthread_t tid, pid;
                pthread_create(&tid, 0, server_thread, &fd);
                break;
            }
            if (size == i)
            {
                char *str = "对不起，聊天室已经满了!";
                send(fd, str, strlen(str), 0);
                close(fd);
            }
        }
    }
}

int main()
{

    init();
    server();
    SendAll();
}