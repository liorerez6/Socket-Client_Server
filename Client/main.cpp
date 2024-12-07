#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <string.h>


#define TIME_PORT 27015
#define GET_TIME_MESSAGE "GetTime"
#define GET_TIME_WITHOUT_DATA "GetOnlyTime"
#define GET_TIME_SINCE_EPOCH "GetTimeSinceEpoch"
#define GET_CLIENT_TO_SERVER_DELAY "GetClientToServerDelayEstimation"


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void initializeWinsock();
SOCKET createSocket();
void closeSocket(SOCKET& socket);
int displayMenu();
void sendRequest(SOCKET& connSocket, sockaddr_in& server, const char* requestMessage);
void receiveResponse(SOCKET& connSocket);

int main()
{
    initializeWinsock();
    SOCKET connSocket = createSocket();

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(TIME_PORT);

    bool exitRequested = false;

    while (!exitRequested)
    {
        int choice = displayMenu();

        const int NUM_REQUESTS = 100;
        DWORD serverTimes[NUM_REQUESTS] = { 0 };
        double totalDelay = 0.0;
        double avgDelay = 0.0;


        switch (choice)
        {
        case 1:
            cout << "Case 1 selected: Get time\n";
            sendRequest(connSocket, server, GET_TIME_MESSAGE);
            receiveResponse(connSocket);
            break;
        case 2:
            cout << "Case 2 selected: Get time without data\n";
            sendRequest(connSocket, server, GET_TIME_WITHOUT_DATA);
            receiveResponse(connSocket);
            break;
        case 3:
            cout << "Case 3 selected: Get time since epoch\n";
            sendRequest(connSocket, server, GET_TIME_SINCE_EPOCH);
            receiveResponse(connSocket);
            break;
        case 4:
            cout << "Case 4 selected: Estimate delay\n";
            totalDelay = 0.0;
            avgDelay = 0.0;

            // Step 1: Send all requests
            for (int i = 0; i < NUM_REQUESTS; ++i) {
                sendRequest(connSocket, server, GET_CLIENT_TO_SERVER_DELAY);
            }

            // Step 2: Receive all responses
            for (int i = 0; i < NUM_REQUESTS; ++i) {
                char recvBuff[255];
                int bytesRecv = recv(connSocket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (SOCKET_ERROR == bytesRecv) {
                    cerr << "Error: Failed to receive response.\n";
                    closeSocket(connSocket);
                    exit(EXIT_FAILURE);
                }
                recvBuff[bytesRecv] = '\0';
                serverTimes[i] = strtoul(recvBuff, NULL, 10);
            }

            // Step 3: Calculate delay
            for (int i = 1; i < NUM_REQUESTS; ++i) {
                totalDelay += (double)(serverTimes[i] - serverTimes[i - 1]);
            }

            avgDelay = totalDelay / (NUM_REQUESTS - 1);

            cout << "Average client-to-server delay estimation: " << avgDelay << " ms\n";
            break;

        case 14:
            exitRequested = true;
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
        }
    }

    closeSocket(connSocket);
    return 0;
}

void initializeWinsock()
{
    WSAData wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        cerr << "Error: WSAStartup failed.\n";
        exit(EXIT_FAILURE);
    }
}

SOCKET createSocket()
{
    SOCKET connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == connSocket)
    {
        cerr << "Error: Socket creation failed.\n";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    return connSocket;
}

void closeSocket(SOCKET& socket)
{
    closesocket(socket);
    WSACleanup();
}

int displayMenu()
{
    cout << "Please enter your choice:\n";
    cout << "1 - Get current time\n";
    cout << "2 - Get time without date\n";
    cout << "3 - Get time since epoch\n";
    cout << "4 - Get client to server delay estimation\n";

    cout << "14 - Exit\n";
    int choice;
    cin >> choice;
    return choice;
}

void sendRequest(SOCKET& connSocket, sockaddr_in& server, const char* requestMessage)
{
    int bytesSent = sendto(connSocket, requestMessage, (int)strlen(requestMessage), 0,
        (const sockaddr*)&server, sizeof(server));
    if (SOCKET_ERROR == bytesSent)
    {
        cerr << "Error: Failed to send request.\n";
        closeSocket(connSocket);
        exit(EXIT_FAILURE);
    }
    cout << "Sent: \"" << requestMessage << "\"\n";
}

void receiveResponse(SOCKET& connSocket)
{
    char recvBuff[255];
    int bytesRecv = recv(connSocket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (SOCKET_ERROR == bytesRecv)
    {
        cerr << "Error: Failed to receive response.\n";
        closeSocket(connSocket);
        exit(EXIT_FAILURE);
    }
    recvBuff[bytesRecv] = '\0';
    cout << "Received: \"" << recvBuff << "\"\n";
}
