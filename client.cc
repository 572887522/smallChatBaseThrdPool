#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include "threadPool.hh"
#include "charHandle.hh"

using namespace std;


const char* LOCALIP = "127.0.0.1";

bool recvThrdStop = false;
string current_input;

unsigned int socketInit()
{
    // 1. 创建通信套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        return -1;
    }

    // 2. 链接服务器 IP port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    inet_pton(AF_INET, LOCALIP, &addr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("connect");
        return -1;
    }
    else
    {
        printf("server is connected successfully!\n");
    }
    return fd;
}

void recvThrd(unsigned int fd)
{
    recvThrdStop = false;
    for(;;)
    {
        char buff[1024];
        memset(buff, 0, sizeof(buff));
        int len = recv(fd, buff, sizeof(buff), 0);
        if(len > 0)
        {

            cout << "\r\033[2K";  // 将光标移回行首并清空行内容
            cout.flush();  // 确保立即输出
            cout << buff << endl;
            cout.flush();

            cout << current_input;
            cout.flush();
        }
        else if(len == 0)
        {
            printf("server is disconnect...\n");
            recvThrdStop = true;
            break;
        }
        else
        {
            perror("recv");
            recvThrdStop = true;
            break;
        }
    }
    close(fd); //通信文件描述符
}

void writeThrd(unsigned int fd)
{
    char ch;
    for(;;)
    {
        if(recvThrdStop)
        {
            cout << "writeThrd : recv thread Stop" << endl;
            break;
        }

        while ((ch = getch()) != '\n') 
        {
            cout << "\r\033[2K";  // 将光标移回行首并清空行内容
            cout.flush();
            if(ch == 127)    // 退格键
            {
                if(current_input.length() > 0)
                    current_input.pop_back();
            }
            else
            {
                current_input.push_back(ch);
            }
            std::cout << current_input;
            cout.flush();
            
        }
        cout << endl;
        cout.flush();
        // getline(cin, current_input);
        // if(cin.eof())
        //     break;

        // 检查用户是否输入了 "exit"
        if (current_input == "exit") {
            break;
        }

        std::cout << "\033[1A\033[2Kyou:>" << current_input << std::endl;
        send(fd, current_input.c_str(), current_input.length(), 0);
        current_input.clear();
    }
}


int main()
{
    int fd = socketInit();
    if(fd == -1)
    {
        perror("socketInit");
        return -1;
    }
    
    ThreadPool pool(2);
    
    pool.enequeue(recvThrd, fd);
    pool.enequeue(writeThrd, fd);

    return 0;
}