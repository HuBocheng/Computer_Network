#include <iostream>
#include <winsock2.h>
#include <string>
#include <vector>
#include <thread>
#include <cstddef>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <unordered_map>

using namespace std;

const int PORT = 8080;        // 服务器端口
const int BUFFER_SIZE = 2048; // 缓冲区大小
const int MAX_CLIENTS = 3;    // 最大客户端数量
const string typeCommand = "Command";
const string typeMessage = "Message";

int cur_client = 0; // 当前客户端数量

// int global_message_id = 0;

vector<SOCKET> clients;                                // 客户端套接字向量
std::unordered_map<SOCKET, std::string> clientNameMap; // 客户端套接字与客户端名字的映射

std::string ConvertToUTF8(const std::string &str)
{
    int wlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    if (wlen == 0)
        return ""; // Check for errors

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], wlen);

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0)
        return ""; // Check for errors

    std::string result(len - 1, 0); // Subtract 1 to exclude the null terminator
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

std::string ConvertFromUTF8(const std::string &utf8str)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, NULL, 0);
    if (wlen == 0)
        return ""; // Check for errors

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &wstr[0], wlen);

    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0)
        return ""; // Check for errors

    std::string result(len - 1, 0); // Subtract 1 to exclude the null terminator
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

void process_command(string commandStr, SOCKET client_socket)
{
    if (commandStr == "QUIT")
    {
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end()); // 删除客户端套接字
        clientNameMap.erase(client_socket);

        cout << "客户端" << clientNameMap[client_socket] << "退出聊天" << endl;
        cur_client--;
        cout << "清理客户端socket-----complete" << endl;
        cout << "当前客户端数量：" << cur_client << "/" << MAX_CLIENTS << endl;
        int temp = client_socket;
        closesocket(client_socket);
        // cout << "message:" << message << endl; // test point
    }
    else if (commandStr == "SHOW")
    {
        cout << "客户" << clientNameMap[client_socket] << "将获得以下信息" << endl;
        cout << "当前客户端数量：" << cur_client << "/" << MAX_CLIENTS << endl;
        string temp = "Serve:" + to_string(cur_client) + "/" + to_string(MAX_CLIENTS);
        send(client_socket, temp.c_str(), 100, 0);
    }
    else if (commandStr == "HELP")
    {
        ;
    }
    else if (commandStr == "CLEAR")
    {
        ;
    }
    else if (commandStr.find("CHANGE NAME ") == 0)
    {
        string newName = commandStr.substr(12); // ?为什么是12
        string oldName = clientNameMap[client_socket];
        clientNameMap[client_socket] = newName;
        cout << "客户端" << client_socket << "将用户名从" << oldName << "修改为" << newName << endl;
    }
    else
    {
        cout << "未知指令" << endl;
    }
}

// 线程函数——处理每一个客户端的函数
void handle_client(SOCKET client_socket)
{
    char buffer[BUFFER_SIZE];
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);                                   // 清空缓冲区
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); // 从客户端接收消息
        // cout << "BUFFER:" << buffer << endl;

        if (bytes_received <= 0)
        { // 如果接收到的字节少于或等于0，说明客户端已断开连接
            closesocket(client_socket);
            cout << "Client disconnected." << endl;
            break;
        }

        string message1(buffer, bytes_received);
        string message = ConvertFromUTF8(message1);

        if (message.find("Command") == 0) // 传输来的是指令——处理系统指令
        {
            cout << "需处理系统指令，指令来自" << clientNameMap[client_socket] << endl;
            string rawCommand = message.substr(8);      // 清除类型头**和冒号**！
            cout << "处理指令：" << rawCommand << endl; // test point
            process_command(rawCommand, client_socket);
            continue; // 不执行消息广播
        }
        if (message.find("Message") == 0) // 传输来的是消息
        {
            cout << "将接收客户端消息" << endl;
            message = message.substr(8); // 清除类型头**和冒号**！

            // 发送ACK给客户端
            // string ackMessage = "ACK";
            // send(client_socket, ackMessage.c_str(), ackMessage.size(), 0);
        }
        // 打印来自哪个客户端的消息
        std::cout << "Message from  " << clientNameMap[client_socket] << ':' << message << std::endl;

        // 给消息附上来源
        message = clientNameMap[client_socket] + ":" + message;

        // 广播消息给其他所有客户端
        // cout << clients.size() << endl; // test point
        for (SOCKET client : clients)
        {
            if (client != client_socket)
            // if (true)
            {
                // string message_with_id = "NO." + to_string(global_message_id++) + ":" + message;
                string utf8Message = ConvertToUTF8(message);
                send(client, utf8Message.c_str(), utf8Message.size(), 0);
            }
        }
    }
}

int main()
{
    // system("chcp 65001");
    // system("[Console]::OutputEncoding = [System.Text.Encoding]::UTF8");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "初始化socket失败." << endl;
        return 1;
    }
    cout << "初始化socket成功" << endl;

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0); // 创建一个TCP套接字
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.S_un.S_addr = INADDR_ANY;

    if (server_socket == INVALID_SOCKET)
    {
        cout << "创建服务器socket失败" << endl;
        return -1;
    }
    cout << "创建服务器socket成功" << endl;

    // 绑定套接字到指定的地址和端口
    if (bind(server_socket, (sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        cerr << "Bind failed. Error: " << WSAGetLastError() << endl;
        return -1;
    }
    else
    {
        cout << "绑定服务器socket成功,使用端口" << PORT << endl;
    }

    // 监听部分
    listen(server_socket, MAX_CLIENTS); // 开始监听来自客户端的连接
    cout << "客户端正在监听端口 " << PORT << endl;

    while (true)
    {
        if (cur_client < MAX_CLIENTS)
        {
            SOCKET client_socket = accept(server_socket, NULL, NULL); // 接受新的客户端连接
            clients.push_back(client_socket);                         // 将新的客户端套接字添加到clients向量中
            cur_client++;
            clientNameMap[client_socket] = "Client" + std::to_string(cur_client);

            cout << "客户端 " << client_socket << "加入聊天" << endl;
            cout << "当前客户端数量：" << cur_client << "/" << MAX_CLIENTS << endl;

            thread(handle_client, client_socket).detach(); // 为每一个客户端创建一个新的线程来处理其消息

            string idMessage = "YourID:client" + to_string(cur_client);
            send(client_socket, idMessage.c_str(), idMessage.size(), 0);
        }
        else
        {
            SOCKET client_socket2 = accept(server_socket, NULL, NULL); // ***必须消费掉这个请求，不然会死循环
            const char *rejectMessage = "CONNECTION_REJECTED";
            send(client_socket2, rejectMessage, strlen(rejectMessage), 0);
            closesocket(client_socket2);
            cout << "拒绝一个连接请求" << endl;
        }
    }

    closesocket(server_socket); // 关闭服务器套接字
    WSACleanup();               // 清理Winsock
    return 0;
}
