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

// �������Է���������Ϣ�ĺ���
void receive_messages(SOCKET client_socket)
{
    char buffer[BUFFER_SIZE]; // ���ڽ�����Ϣ�Ļ�����
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);                                   // ��ջ�����
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); // �ӷ�����������Ϣ

        if (bytes_received <= 0)
        { // ������յ����ֽ����ڻ����0��˵����������������ѶϿ�
            std::cerr << "�ѶϿ��������������" << std::endl;
            closesocket(client_socket);
            exit(1);
            // return; // �˳��̣߳��������˳���������
        }
        if (bytes_received > 0 && strcmp(buffer, "CONNECTION_REJECTED") == 0)
        {
            std::cerr << "���ӱ��������ر�,������������" << std::endl;
            closesocket(client_socket);
            WSACleanup();
            exit(1);
        }

        std::string message1(buffer, bytes_received); // �����յ����ַ�����ת��Ϊstring�ַ���
        string message = ConvertFromUTF8(message1);

        // cout << "message!!!:" << message << endl; // test point
        // cout << message.substr(7) << endl;
        if (message.find("YourID") != string::npos)
        {
            client_name = message.substr(7);

            // ��ʹ�� substr �������ַ��� message �ĵ� 6 ���ַ���ʼ��ȡ���ַ�����Ȼ��ʹ�� stoi ���������ַ���ת��Ϊ��������
            // str.substr(7, 5)��ʾ���ַ��� str �ĵ� 7 ���ַ���ʼ��ȡ����Ϊ 5 �����ַ���
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
    WSADATA wsaData; // ���ڳ�ʼ��Winsock��
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }
    cout << "��ʼ��socket�ɹ�" << endl;

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0); // ����һ��TCP�׽���
    // ��������
    // AF_INET��ʹ��IPv4��ַ
    // SOCK_STREAM��������·����ʽ�׽��֣���TCP�׽���
    // 0���Զ�ѡ��Э��
    if (client_socket == INVALID_SOCKET)
    {
        cout << "�����ͻ���socketʧ��" << endl;
        return -1;
    }
    cout << "�����ͻ���socket�ɹ�" << endl;

    // ���÷�������ַ
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;                          // ʹ��IPv4��ַ
    server_address.sin_port = htons(PORT);                        // �˿ں�
    server_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // ������IP��ַ
    // �������� IP ��ַΪ���ػػ���ַ 127.0.0.1��

    // connect����
    if (connect(client_socket, (sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        std::cerr << "Connect failed. Error: " << WSAGetLastError() << std::endl;
        if (WSAGetLastError() == WSAECONNREFUSED)
        { // ��������Ƿ񱻷������ܾ�
            std::cerr << "�������ܾ��������������Ժ����ԡ�" << std::endl;
        }
        closesocket(client_socket); // �رտͻ����׽���
        WSACleanup();               // ����Winsock
        return -1;
    }
    cout << "�ɹ����ӵ�������" << endl;
    cout << "��������ʼ���ɹ�" << endl;

    std::thread t(receive_messages, client_socket); // ����һ���µ��߳���������Ϣ
    // ��δ��봴����һ���µ��̣߳����� receive_messages ������Ϊ�̵߳���ڵ㣬ͬʱ�� client_socket ��Ϊ�������ݸ� receive_messages ������Ȼ�������� detach �����������߳��뵱ǰ�̷߳��룬ʹ�����߳̿��Զ������С�������receive_messages �����Ϳ��������߳������У�������������ǰ�̡߳�

    if (t.joinable())
    {
        t.detach(); // ���߳��뵱ǰ�̷߳���
        cout << "�̴߳����ɹ�" << endl;
    }
    else
    {
        // �̴߳���ʧ�ܣ����д�����
        std::cerr << "�̴߳���ʧ��" << std::endl;
        return -1;
    }

    std::string message;
    while (true)
    {
        std::getline(std::cin, message); // �ӿ���̨��ȡ�û�����

        // string temp = "������";
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
            // send(client_socket, utf8Message.c_str(), utf8Message.size(), 0); // ������Ϣ��������
            // send(client_socket, utf8Type.c_str(), utf8Type.size(), 0);       // ������Ϣ��������
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
            cout << "HELP:��ʾ������Ϣ" << endl;
            cout << "SHOW:��ʾ��ǰ��������Ŀ" << endl;
            cout << "QUIT:�رտͻ���" << endl;
            cout << "CLEAN:����" << endl;
            cout << "CHANGE NAME <new name>: �����û���" << endl;
            cout << "<<==========================>>" << endl;
        }
        else // ���ͻ��˷�����Ϣ
        {
            type = typeMessage;
            string fullMessage = type + ":" + message;
            string utf8FullMessage = ConvertToUTF8(fullMessage);
            send(client_socket, utf8FullMessage.c_str(), utf8FullMessage.size(), 0);

            cout << client_name << "(ME):" << message << std::endl;
            // // �ȴ���������ACK
            // char ackBuffer[10];
            // int ackReceived = recv(client_socket, ackBuffer, sizeof(ackBuffer), 0);
            // if (ackReceived <= 0 || string(ackBuffer) != "ACK")
            // {
            //     cerr << "û�дӷ������յ�ȷ�ϣ�������Ҫ���·�����Ϣ" << endl;
            // }
            // else
            // {
            //     cout << client_name << "(ME):" << message << std::endl;
            // }
        }
    }

    closesocket(client_socket); // �رտͻ����׽���
    WSACleanup();               // ����Winsock
    return 0;
}
