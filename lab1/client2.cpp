#include <iostream>
#include <winsock2.h>
#include <string>
#include <thread>
#include <cstddef>
#include <codecvt>
#include <locale>

#define typeMessage "Message"

using namespace std;

const int PORT = 8080;
const int BUFFER_SIZE = 2048;
const string typeCommand = "Command";
// const string typeMessage = "Message";

int expected_message_id = 0;

int client_id = 0;
string client_name = "Client" + to_string(client_id);

std::string ConvertToUTF8(const std::string &str)
{
    // cout << "Input string: " << str << endl; // test point
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

    // cout << "Output string length: " << result.size() << endl; // test point
    // cout << "Output string: " << result << endl; // test point

    return result;
}

std::string ConvertFromUTF8(const std::string &utf8str)
{
    // cout << "Input string: " << utf8str << endl; // test point
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

    // cout << "Output string length: " << result.size() << endl; //test point
    // cout << "Output string: " << result << endl; // test point

    return result;
}

// 接收来自服务器的消息的函数
void receive_messages(SOCKET client_socket)
{
    char buffer[BUFFER_SIZE]; // 用于接收消息的缓冲区
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);                                   // 清空缓冲区
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); // 从服务器接收消息

        if (bytes_received <= 0)
        { // 如果接收到的字节少于或等于0，说明与服务器的连接已断开
            std::cerr << "已断开与服务器的链接" << std::endl;
            closesocket(client_socket);
            exit(1);
            // return; // 退出线程，而不是退出整个程序
        }
        if (bytes_received > 0 && strcmp(buffer, "CONNECTION_REJECTED") == 0)
        {
            std::cerr << "连接被服务器关闭,聊天室已满。" << std::endl;
            closesocket(client_socket);
            WSACleanup();
            exit(1);
        }

        std::string message1(buffer, bytes_received); // 将接收到的字符数组转换为string字符串
        string message = ConvertFromUTF8(message1);

        // cout << "message!!!:" << message << endl; // test point
        // cout << message.substr(7) << endl;
        if (message.find("YourID") != string::npos)
        {
            client_name = message.substr(7);

            // 它使用 substr 函数从字符串 message 的第 6 个字符开始提取子字符串，然后使用 stoi 函数将子字符串转换为整数类型
            // str.substr(7, 5)表示从字符串 str 的第 7 个字符开始提取长度为 5 的子字符串
            cout << "Your name is " << client_name << endl; // test point
            continue;
        }

        if (message.find("ACK") != string::npos)
        {
            continue;
        }
        cout << message << endl;
        // cout << "Client" << client_id << " :" << message << std::endl;
    }
}

int main()
{

    // system("chcp 65001");
    // system("[Console]::OutputEncoding = [System.Text.Encoding]::UTF8");
    WSADATA wsaData; // 用于初始化Winsock库
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }
    cout << "初始化socket成功" << endl;

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0); // 创建一个TCP套接字
    // 参数解释
    // AF_INET：使用IPv4地址
    // SOCK_STREAM：面向网路的流式套接字，即TCP套接字
    // 0：自动选择协议
    if (client_socket == INVALID_SOCKET)
    {
        cout << "创建客户端socket失败" << endl;
        return -1;
    }
    cout << "创建客户端socket成功" << endl;

    // 配置服务器地址
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;                          // 使用IPv4地址
    server_address.sin_port = htons(PORT);                        // 端口号
    server_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // 服务器IP地址
    // 服务器的 IP 地址为本地回环地址 127.0.0.1。

    // connect操作
    if (connect(client_socket, (sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        std::cerr << "Connect failed. Error: " << WSAGetLastError() << std::endl;
        if (WSAGetLastError() == WSAECONNREFUSED)
        { // 检查连接是否被服务器拒绝
            std::cerr << "服务器拒绝了连接请求。请稍后重试。" << std::endl;
        }
        closesocket(client_socket); // 关闭客户端套接字
        WSACleanup();               // 清理Winsock
        return -1;
    }
    cout << "成功连接到服务器" << endl;
    cout << "聊天界面初始化成功" << endl;

    std::thread t(receive_messages, client_socket); // 创建一个新的线程来接收消息
    // 这段代码创建了一个新的线程，并将 receive_messages 函数作为线程的入口点，同时将 client_socket 作为参数传递给 receive_messages 函数。然后，它调用 detach 函数，将新线程与当前线程分离，使得新线程可以独立运行。这样，receive_messages 函数就可以在新线程中运行，而不会阻塞当前线程。

    if (t.joinable())
    {
        t.detach(); // 将线程与当前线程分离
        cout << "线程创建成功" << endl;
    }
    else
    {
        // 线程创建失败，进行错误处理
        std::cerr << "线程创建失败" << std::endl;
        return -1;
    }

    std::string message;
    while (true)
    {
        std::getline(std::cin, message); // 从控制台读取用户输入

        // string temp = "哈哈哈";
        // cout << temp << endl;
        string type = typeMessage;

        if (message == "QUIT")
        {
            type = typeCommand;
            string fullMessage = type + ":" + message;
            string utf8FullMessage = ConvertToUTF8(fullMessage);
            send(client_socket, utf8FullMessage.c_str(), utf8FullMessage.size(), 0);

            // string utf8Message = ConvertToUTF8(message);
            // string utf8Type = ConvertToUTF8(type);
            // cout << "utf8Message: " << utf8Message << endl; // test point
            // // cout << "utf8Message: " << utf8Message << endl; // test point
            // // cout << "sending" << endl; // test point
            // send(client_socket, utf8Message.c_str(), utf8Message.size(), 0); // 发送消息到服务器
            // send(client_socket, utf8Type.c_str(), utf8Type.size(), 0);       // 发送消息到服务器
            break;
        }
        else if (message.find("CLEAR") == 0)
        {
            type = typeCommand;
            string fullMessage = type + ":" + message;
            string utf8FullMessage = ConvertToUTF8(fullMessage);
            send(client_socket, utf8FullMessage.c_str(), utf8FullMessage.size(), 0);
            system("cls");
        }
        else if (message.find("CHANGE NAME ") == 0)
        {
            client_name = message.substr(12);
            type = typeCommand;
            string fullMessage = type + ":" + message;
            string utf8FullMessage = ConvertToUTF8(fullMessage);
            send(client_socket, utf8FullMessage.c_str(), utf8FullMessage.size(), 0);
        }
        else if (message.find("SHOW") == 0)
        {
            type = typeCommand;
            string fullMessage = type + ":" + message;
            string utf8FullMessage = ConvertToUTF8(fullMessage);
            send(client_socket, utf8FullMessage.c_str(), utf8FullMessage.size(), 0);
        }
        else if (message.find("HELP") == 0)
        {
            cout << "<<==========================>>" << endl;
            cout << "HELP:显示帮助信息" << endl;
            cout << "SHOW:显示当前聊天室数目" << endl;
            cout << "QUIT:关闭客户端" << endl;
            cout << "CLEAN:清屏" << endl;
            cout << "CHANGE NAME <new name>: 更改用户名" << endl;
            cout << "<<==========================>>" << endl;
        }
        else // 本客户端发送消息
        {
            type = typeMessage;
            string fullMessage = type + ":" + message;
            string utf8FullMessage = ConvertToUTF8(fullMessage);
            send(client_socket, utf8FullMessage.c_str(), utf8FullMessage.size(), 0);

            cout << client_name << "(ME):" << message << std::endl;
            // // 等待服务器的ACK
            // char ackBuffer[10];
            // int ackReceived = recv(client_socket, ackBuffer, sizeof(ackBuffer), 0);
            // if (ackReceived <= 0 || string(ackBuffer) != "ACK")
            // {
            //     cerr << "没有从服务器收到确认，可能需要重新发送消息" << endl;
            // }
            // else
            // {
            //     cout << client_name << "(ME):" << message << std::endl;
            // }
        }
    }

    closesocket(client_socket); // 关闭客户端套接字
    WSACleanup();               // 清理Winsock
    return 0;
}
