#define _CRT_SECURE_NO_WARNINGS

#include <ctime>
#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <time.h>

using namespace std;

//END POINTS:

#define TIME_PORT 27015
#define GET_TIME_MESSAGE "GetTime"
#define GET_TIME_WITHOUT_DATA "GetOnlyTime"
#define GET_TIME_SINCE_EPOCH "GetTimeSinceEpoch"
#define GET_CLIENT_TO_SERVER_DELAY "GetClientToServerDelayEstimation"


#pragma comment(lib, "Ws2_32.lib")


void initializeWinsock();
SOCKET createSocket();
void bindSocket(SOCKET& serverSocket);
void handleRequests(SOCKET& serverSocket);
void processRequest(const char* request, char* response);
void sendResponse(SOCKET& serverSocket, const sockaddr& clientAddr, int clientAddrLen, const char* response);
void extractTimeWithoutDate(char* buffer);

int main()
{
    initializeWinsock();
    SOCKET serverSocket = createSocket();
    bindSocket(serverSocket);

    cout << "Server is waiting for client requests...\n";
    handleRequests(serverSocket);

    closesocket(serverSocket);
    WSACleanup();
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
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == serverSocket)
    {
        cerr << "Error: Socket creation failed.\n";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    return serverSocket;
}

void bindSocket(SOCKET& serverSocket)
{
    sockaddr_in serverService = {};
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(TIME_PORT);

    if (SOCKET_ERROR == bind(serverSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
    {
        cerr << "Error: Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

void handleRequests(SOCKET& serverSocket)
{
    sockaddr clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char recvBuff[255];
    char sendBuff[255];

    while (true)
    {
        int bytesRecv = recvfrom(serverSocket, recvBuff, sizeof(recvBuff) - 1, 0, &clientAddr, &clientAddrLen);
        if (SOCKET_ERROR == bytesRecv)
        {
            cerr << "Error: Failed to receive data.\n";
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        recvBuff[bytesRecv] = '\0';
        cout << "Received: \"" << recvBuff << "\"\n";

        processRequest(recvBuff, sendBuff);
        sendResponse(serverSocket, clientAddr, clientAddrLen, sendBuff);
    }
}

void processRequest(const char* request, char* response)
{
    if (strcmp(request, GET_TIME_MESSAGE) == 0)
    {
        time_t timer = time(nullptr);
        strcpy(response, ctime(&timer));
        response[strlen(response) - 1] = '\0'; // Remove newline
    }
    else if (strcmp(request, GET_TIME_WITHOUT_DATA) == 0)
    {
        extractTimeWithoutDate(response);
    }
    else if (strcmp(request, GET_TIME_SINCE_EPOCH) == 0) {

        // Get the current time since the epoch (seconds since 1.1.1970).
        time_t timer;
        time(&timer);

        // Convert time to string format.
        sprintf(response, "%ld", timer); // Long integer format to store epoch time
        response[strlen(response) - 1] = '\0'; // Remove newline
    }
    else if (strcmp(request, GET_CLIENT_TO_SERVER_DELAY) == 0) {
        DWORD tickCount = GetTickCount();
        sprintf(response, "%lu", tickCount);
    }
    else
    {
        strcpy(response, "Invalid request");
    }
}

void sendResponse(SOCKET& serverSocket, const sockaddr& clientAddr, int clientAddrLen, const char* response)
{
    int bytesSent = sendto(serverSocket, response, (int)strlen(response), 0, &clientAddr, clientAddrLen);
    if (SOCKET_ERROR == bytesSent)
    {
        cerr << "Error: Failed to send response.\n";
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << "Sent: \"" << response << "\"\n";
}

void extractTimeWithoutDate(char* buffer)
{
    time_t timer = time(nullptr);
    tm* timeInfo = localtime(&timer);
    strftime(buffer, 255, "%H:%M:%S", timeInfo);
}
