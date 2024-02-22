#include<iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

 /*
    // initialize winsock library
	// get ip and port
	// bind the ip/port with the socket
	// listen on the socket
	//  create the socket
	// accept
	// recieve and send
	//close the socket
	// cleanup the winsock
 */
bool Initialize() {
	WSADATA data;  // boilerplate code
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;    // (2,2) because winsock 2nd version
}

void InteractWithClient(SOCKET clientSocket , vector<SOCKET>& clients) {
	// send / rev
	//receive message from client

	cout << "client connected" << endl;
	char buffer[4096];
	while (1) {
		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesrecvd <= 0) {
			cout << "client disconnected" << endl;
			break;
		}
		string message(buffer, bytesrecvd);
		cout << "message from client" << message << endl;
		
		for (auto client : clients) {
			// sending messages to other clients also who are connected lately to server
			if (client != clientSocket) {
				// client sending message must not return to same client
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}
	// removing clients from vector
	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it);
	}
	closesocket(clientSocket);
}

int main() {
	// initialize
	if (!Initialize()) {
		cout << "winsock initialization failed" << endl;
		return 1;
	}
	cout << "server program" << endl;

	// create socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET) {
		// checking socket is created or not
		cout << "socket creation failed" << endl;
		return 1;
	}

	// get ip and port , craete address structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);  //htons -> host to network

	// covert the Ip address to binary format
	// convert the ipaddress (0,0,0,0) put it inside the sin_family in binary format
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "setting address structure failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// bind the ip/port with the socket
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "bind failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// listen on the socket
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "server has started listening on port : " << port << endl;
	vector<SOCKET> clients;
	while (1) {
		//accept
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			cout << "invalid client socket" << endl;
		}

		clients.push_back(clientSocket);
		thread t1(InteractWithClient, clientSocket , std::ref(clients));  // for next clients
		t1.detach(); //because in while it runs   until manually closed 
	}
	
	closesocket(listenSocket);

	// send message to client
	WSACleanup();   // finalize
	return 0;
}