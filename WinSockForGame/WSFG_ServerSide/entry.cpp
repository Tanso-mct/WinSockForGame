#include <iostream>
#include <winsock2.h>

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

    SocketWrapper serverSocket
    (
        AF_INET, // Specifies which type of address to use. AF_INET specifies IPv4.
        SOCK_DGRAM, // Specifies the socket type. SOCK_DGRAM specifies Datagram communication（UDP communication).
        IPPROTO_UDP // Specifies which communication protocol is used. IPPROTO_UDP specifies UDP communication.
    );

    // Set up the server address.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // Address family. AF_INET specifies IPv4.

    /**
     * The server is configured to accept connections from any local IP address.
     * INADDR_ANY will allow binding to any interface, even if the server machine has multiple network interfaces 
     * (e.g. LAN, Wi-Fi, virtual network, etc.)
     */
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    /**
     * Specifies the port number to wait for connections.
     * 0 ~ 1023 is the range of port numbers that can be used for communication.
     * 1024 ~ 49151 is the range of port numbers that can be used for registered services.
     * 49152 ~ 65535 is the range of port numbers that can be used for dynamic or private services.
     */
    serverAddr.sin_port = htons(50000);

    // Bind the socket to the address and port.
    if (bind(serverSocket.get(), (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed.\n";
        return 1;
    }

    // Message send and receive
    char buffer[512];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    while (true)
    {
        // Receive a message from the client.
        int recvLen = recvfrom
        (
            serverSocket.get(), // Socket descriptor for receiving data
            buffer, // Pointer to the buffer where the received data is stored
            sizeof(buffer), // Length of the buffer
            0, // Flags (0 means no special flags)
            (struct sockaddr*)&clientAddr, // Pointer to the address structure where the client's address is stored
            &clientAddrLen // Length of the address structure of the client
        );
        if (recvLen == SOCKET_ERROR)
        {
            std::cerr << "recvfrom failed.\n";
            break;
        }

        buffer[recvLen] = '\0'; // NULL-terminated to treat incoming data as a string
        std::cout << "Received message: " << buffer << "\n";

        // Send a response to the client.
        const char* response = "Message received!";
        sendto
        (
            serverSocket.get(), // Socket descriptor for sending data
            response, // Pointer to the data to be sent
            strlen(response), // Length of the data to be sent
            0, // Flags (0 means no special flags)
            (struct sockaddr*)&clientAddr, // Pointer to the address of the client to send data to
            clientAddrLen // Length of the address structure of the client
        );
    }

	return 0;
}