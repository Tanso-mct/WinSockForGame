#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

int main(){
    // WSADATA type is a structure that stores information necessary for initializing communication.
	WSADATA wsaData;

    // WSAStartup function initializes the Winsock. 
	WSAStartup
    (
        2, // The version of Winsock to use.
        &wsaData // Pointer to a WSADATA structure that receives information about the Winsock implementation.
    );

    // Function to terminate WinSock
	WSACleanup();

	return 0;
}