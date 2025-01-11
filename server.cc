#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <mutex>
#include <vector>
#include <arpa/inet.h>
#include "threadPool.hh"

using namespace std;

vector<unsigned int> fdSet;
mutex mtx;
char commonDataArea[1024];


int socketInit()
{
    // 1. 创建通信套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        return -1;
    }

    // 2. 绑定本地的 IP port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;// 0 = 0.0.0.0, 会去读网卡的实际IP地址
    int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("bind");
        return -1;
    }

    // 3. 设置监听
    ret = listen(fd, 128);
    if(ret == -1)
    {
        perror("listen");
        return -1;
    }

    // cout << "listen fd is " << fd << endl;
    return fd;
}

void communication(int cfd)
{
    // 5. 通信
    for(;;)
    {
        // 先收数据
        char buff[1000]; char formattedBuff[1024];
        memset(buff, 0, sizeof(buff));
        memset(formattedBuff, 0, sizeof(formattedBuff));
        int len = recv(cfd, buff, sizeof(buff), 0);
        if(len > 0)
        {
            // 格式化发来的消息
            sprintf(formattedBuff, "user(fd):%d>%s", cfd, buff);
            printf("%s\n", formattedBuff);
            // 将消息发送给其他连接
            lock_guard<mutex> lock(mtx);
            for(int fd_ : fdSet)
            {
                if(fd_ == cfd)
                    continue;
                send(fd_, formattedBuff, strlen(formattedBuff)+1, 0);
            }
        }
        else if(len == 0)
        {
            printf("fd = %d is disconnect...\n", cfd);
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }

    int tmpcfd = cfd;
    auto it = std::find(fdSet.begin(), fdSet.end(), tmpcfd);
    if (it != fdSet.end()) {
        fdSet.erase(it);
    }
    close(cfd);// 通信文件描述符
}


struct sockaddr_in caddr;
socklen_t caddrlen = sizeof(caddr);

int main()
{
    int fd = socketInit();
    if(fd == -1)
    {
        perror("listen fd get failed, socket init failed");
        return -1;
    }

    ThreadPool pool(10);

    for(;;)
    {
        // 4. 阻塞并等待客户端的链接
        int cfd = accept(fd, (struct sockaddr*)&caddr, &caddrlen);
        if(cfd == -1)
        {
            perror("accept");
            continue;
        }
        else
        {
            lock_guard<mutex> lock(mtx);
            fdSet.push_back(cfd);
            printf("fd = %d connected\n", cfd);
        }
        pool.enequeue(communication, cfd);
    }

    close(fd);
    return 0;
}







    // 连接成功打印信息
    // char ip[32];
    // printf("server ip : %s;  port is : %d\n", 
    // inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip)), ntohs(caddr.sin_port));