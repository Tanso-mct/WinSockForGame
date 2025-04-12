#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>

#pragma comment(lib,"ws2_32.lib")

class WSAWrapper
{
public:
    WSAWrapper(WSADATA* wsaData)
    {
        // Initialize Winsock
        if 
        (
            WSAStartup
            (
                2, // The version of Winsock to use.
                wsaData // Pointer to a WSADATA structure that receives information about the Winsock implementation.
            ) != 0
        ){
            std::cerr << "WSAStartup failed.\n";
            throw std::runtime_error("WSAStartup failed");
        }
    }

    ~WSAWrapper()
    {
        WSACleanup();
    }
};

class SocketWrapper
{
private:
    SOCKET socket_;
public:
    SocketWrapper(int af, int type, int protocol)
    {
        // Create a socket
        socket_ = socket(af, type, protocol);
        if (socket_ == INVALID_SOCKET)
        {
            std::cerr << "Socket creation failed.\n";
            throw std::runtime_error("Socket creation failed");
        }
    }

    ~SocketWrapper()
    {
        closesocket(socket_);
    }

    SOCKET get() const { return socket_; }
};

int main(){
    WSADATA wsaData; // Stores information needed to initialize WinSock
    WSAWrapper wsa(&wsaData);

    SocketWrapper clientSocket
    (
        AF_INET, // Specifies which type of address to use. AF_INET specifies IPv4.
        SOCK_DGRAM, // Specifies the socket type. SOCK_DGRAM specifies Datagram communication（UDP communication).
        IPPROTO_UDP // Specifies which communication protocol is used. IPPROTO_UDP specifies UDP communication.
    );

    // Set up the server address.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // Address family. AF_INET specifies IPv4.

    // Server IP Address
    inet_pton
    (
        AF_INET, // Converts an IPv4 address in string format to a binary format.
        "127.0.0.1", // Localhost IP address.
        &serverAddr.sin_addr // Pointer to the address structure where the converted address is stored.
    );

    serverAddr.sin_port = htons(50000); // Server port number.

    // Send a message to the server.
    const char* message = "Hello from client!";
    if 
    (
        sendto
        (
            clientSocket.get(), // Socket descriptor for sending data
            message, // Pointer to the data to be sent
            strlen(message), // Length of the data to be sent
            0, // Flags (0 means no special flags)
            (struct sockaddr*)&serverAddr, // Pointer to the address of the server to send data to
            sizeof(serverAddr) // Length of the address structure of the server
        ) == SOCKET_ERROR
    ){
        std::cerr << "sendto failed.\n";
        closesocket(clientSocket.get());
        WSACleanup();
        return 1;
    }

    std::cout << "Message sent to server.\n";

    auto start = std::chrono::high_resolution_clock::now();

    // Receive response from server
    char buffer[512];
    int serverAddrLen = sizeof(serverAddr);
    int recvLen = recvfrom
    (
        clientSocket.get(), // Socket descriptor for receiving data
        buffer, // Pointer to the buffer where the received data is stored
        sizeof(buffer), // Length of the buffer
        0, // Flags (0 means no special flags)
        (struct sockaddr*)&serverAddr, // Pointer to the address structure where the server's address is stored
        &serverAddrLen // Length of the address structure of the server
    );
    if (recvLen > 0)
    {
        buffer[recvLen] = '\0'; // NULL-terminated to treat incoming data as a string
        std::cout << "Server response: " << buffer << "\n";

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        float durationInMilliseconds = duration / 1000000.0f; // Convert to milliseconds
        std::cout << "Response time: " << durationInMilliseconds << " milliseconds\n";
    }

	return 0;
}