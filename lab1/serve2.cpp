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

const int PORT = 8080;        // �������˿�
const int BUFFER_SIZE = 2048; // ��������С
const int MAX_CLIENTS = 3;    // ���ͻ�������
const string typeCommand = "Command";
const string typeMessage = "Message";

int cur_client = 0; // ��ǰ�ͻ�������

// int global_message_id = 0;

vector<SOCKET> clients;                                // �ͻ����׽�������
std::unordered_map<SOCKET, std::string> clientNameMap; // �ͻ����׽�����ͻ������ֵ�ӳ��

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
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end()); // ɾ���ͻ����׽���
        clientNameMap.erase(client_socket);

        cout << "�ͻ���" << clientNameMap[client_socket] << "�˳�����" << endl;
        cur_client--;
        cout << "����ͻ���socket-----complete" << endl;
        cout << "��ǰ�ͻ���������" << cur_client << "/" << MAX_CLIENTS << endl;
        int temp = client_socket;
        closesocket(client_socket);
        // cout << "message:" << message << endl; // test point
    }
    else if (commandStr == "SHOW")
    {
        cout << "�ͻ�" << clientNameMap[client_socket] << "�����������Ϣ" << endl;
        cout << "��ǰ�ͻ���������" << cur_client << "/" << MAX_CLIENTS << endl;
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
        string newName = commandStr.substr(12); // ?Ϊʲô��12
        string oldName = clientNameMap[client_socket];
        clientNameMap[client_socket] = newName;
        cout << "�ͻ���" << client_socket << "���û�����" << oldName << "�޸�Ϊ" << newName << endl;
    }
    else
    {
        cout << "δָ֪��" << endl;
    }
}

// �̺߳�����������ÿһ���ͻ��˵ĺ���
void handle_client(SOCKET client_socket)
{
    char buffer[BUFFER_SIZE];
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);                                   // ��ջ�����
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); // �ӿͻ��˽�����Ϣ
        // cout << "BUFFER:" << buffer << endl;

        if (bytes_received <= 0)
        { // ������յ����ֽ����ڻ����0��˵���ͻ����ѶϿ�����
            closesocket(client_socket);
            cout << "Client disconnected." << endl;
            break;
        }

        string message1(buffer, bytes_received);
        string message = ConvertFromUTF8(message1);

        if (message.find("Command") == 0) // ����������ָ�������ϵͳָ��
        {
            cout << "�账��ϵͳָ�ָ������" << clientNameMap[client_socket] << endl;
            string rawCommand = message.substr(8);      // �������ͷ**��ð��**��
            cout << "����ָ�" << rawCommand << endl; // test point
            process_command(rawCommand, client_socket);
            continue; // ��ִ����Ϣ�㲥
        }
        if (message.find("Message") == 0) // ������������Ϣ
        {
            cout << "�����տͻ�����Ϣ" << endl;
            message = message.substr(8); // �������ͷ**��ð��**��

            // ����ACK���ͻ���
            // string ackMessage = "ACK";
            // send(client_socket, ackMessage.c_str(), ackMessage.size(), 0);
        }
        // ��ӡ�����ĸ��ͻ��˵���Ϣ
        std::cout << "Message from  " << clientNameMap[client_socket] << ':' << message << std::endl;

        // ����Ϣ������Դ
        message = clientNameMap[client_socket] + ":" + message;

        // �㲥��Ϣ���������пͻ���
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
        cerr << "��ʼ��socketʧ��." << endl;
        return 1;
    }
    cout << "��ʼ��socket�ɹ�" << endl;

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0); // ����һ��TCP�׽���
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.S_un.S_addr = INADDR_ANY;

    if (server_socket == INVALID_SOCKET)
    {
        cout << "����������socketʧ��" << endl;
        return -1;
    }
    cout << "����������socket�ɹ�" << endl;

    // ���׽��ֵ�ָ���ĵ�ַ�Ͷ˿�
    if (bind(server_socket, (sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        cerr << "Bind failed. Error: " << WSAGetLastError() << endl;
        return -1;
    }
    else
    {
        cout << "�󶨷�����socket�ɹ�,ʹ�ö˿�" << PORT << endl;
    }

    // ��������
    listen(server_socket, MAX_CLIENTS); // ��ʼ�������Կͻ��˵�����
    cout << "�ͻ������ڼ����˿� " << PORT << endl;

    while (true)
    {
        if (cur_client < MAX_CLIENTS)
        {
            SOCKET client_socket = accept(server_socket, NULL, NULL); // �����µĿͻ�������
            clients.push_back(client_socket);                         // ���µĿͻ����׽�����ӵ�clients������
            cur_client++;
            clientNameMap[client_socket] = "Client" + std::to_string(cur_client);

            cout << "�ͻ��� " << client_socket << "��������" << endl;
            cout << "��ǰ�ͻ���������" << cur_client << "/" << MAX_CLIENTS << endl;

            thread(handle_client, client_socket).detach(); // Ϊÿһ���ͻ��˴���һ���µ��߳�����������Ϣ

            string idMessage = "YourID:client" + to_string(cur_client);
            send(client_socket, idMessage.c_str(), idMessage.size(), 0);
        }
        else
        {
            SOCKET client_socket2 = accept(server_socket, NULL, NULL); // ***�������ѵ�������󣬲�Ȼ����ѭ��
            const char *rejectMessage = "CONNECTION_REJECTED";
            send(client_socket2, rejectMessage, strlen(rejectMessage), 0);
            closesocket(client_socket2);
            cout << "�ܾ�һ����������" << endl;
        }
    }

    closesocket(server_socket); // �رշ������׽���
    WSACleanup();               // ����Winsock
    return 0;
}
