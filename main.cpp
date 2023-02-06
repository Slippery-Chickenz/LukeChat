#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#define DEFAULT_PORT "27015"

#pragma comment(lib, "Ws2_32.lib")




// Function to call to accept any incoming connectiong requests
int accept_connections(SOCKET &ClientSocket, SOCKET &ListenSocket) {

  ClientSocket = INVALID_SOCKET;

  // Wrote this myself! 
  // The select function I think will tell us if the Listen Socket has a connection request
  // This is needed because the accept function below 'blocks' if there is no incoming request
  // From what I understand Blocking just means it will stop and wait there until there is an incomming call which stops the program
  // Another solution I might need to look into is creating another thread where I can run the Accept function in a loop always looking for a request
  // There is a setting to turn off the blocking which I would need to use if I made the loop above
  fd_set ReadSet; // This is making a fd_set called ReadSet. This is the input that the select function uses
  FD_ZERO(&ReadSet); //This resets the ReadSet to get it ready
  FD_SET(ListenSocket, &ReadSet); // This adds the Listen Socket to the fd_set we made to test it in the select function
  struct timeval time_out; // Need to make a timeval variable for the amount of time to wait til timeout
  time_out.tv_sec = 2; //2 Seconds
  time_out.tv_usec = 000000; // 0 Microseconds
  if (select(0, &ReadSet, NULL, NULL, &time_out) == 0) {
    printf("No Incomming Connections\n");
  }
  else {
    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
      printf("Incoming Connection!...\n");
      printf("Successfully accepted a connection request!\n");
    }
  }

  return 0;
}

// Function to create and then connect a client socket to a server from and IP and Port
int connect_to_ip(const char* connect_addr, const char* connect_port,SOCKET& ConnectSocket) {  

  int iResult;

  // This makes 3 different addrinfo struct objects. Two pointers result and ptr and then an object hints
  struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;

  ZeroMemory( &hints, sizeof(hints) ); // Sets all the memeory values in this structure to 0
  hints.ai_family = AF_UNSPEC; // Says that we can use either IPv4 or IPv4
  hints.ai_socktype = SOCK_STREAM; // Two way connection with "Out of Band data transmission" and it uses the TCP protocol (Again idk if thats the correct usage)
  hints.ai_protocol = IPPROTO_TCP; // Again just says we are using the TCP transmission protool


  // Resolve the server address and port
  // The get addrinfo function gets the addrinfo of all devices with the server address and point (argv[1] and DEFAULT_PORT respectively) and 
  // puts them in a linked list where results item points to the first thing in the list.
  iResult = getaddrinfo(connect_addr, connect_port, &hints, &result);
  if (iResult != 0) {
      printf("getaddrinfo (Client) failed: %d\n", iResult);
      WSACleanup();
      return 1;
  }



  printf("\nPossible Connections:\n");

  int i = 1;

  struct sockaddr_in  *sockaddr_ipv4;

  INT iRetval;

  char ipstringbuffer[46];
  DWORD ipbufferlength = 46;

  LPSOCKADDR sockaddr_ip;

  // Retrieve each address and print out the hex bytes
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamily: ");
        switch (ptr->ai_family) {
            case AF_UNSPEC:
                printf("Unspecified\n");
                break;
            case AF_INET:
                printf("AF_INET (IPv4)\n");
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                printf("\tIPv4 address %s\n",
                    inet_ntoa(sockaddr_ipv4->sin_addr) );
                break;
            case AF_INET6:
                printf("AF_INET6 (IPv6)\n");
                // the InetNtop function is available on Windows Vista and later
                // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                // printf("\tIPv6 address %s\n",
                //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );
                
                // We use WSAAddressToString since it is supported on Windows XP and later
                sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
                    ipstringbuffer, &ipbufferlength );
                if (iRetval)
                    printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
                else    
                    printf("\tIPv6 address %s\n", ipstringbuffer);
                break;
            case AF_NETBIOS:
                printf("AF_NETBIOS (NetBIOS)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_family);
                break;
        }
        printf("\tSocket type: ");
        switch (ptr->ai_socktype) {
            case 0:
                printf("Unspecified\n");
                break;
            case SOCK_STREAM:
                printf("SOCK_STREAM (stream)\n");
                break;
            case SOCK_DGRAM:
                printf("SOCK_DGRAM (datagram) \n");
                break;
            case SOCK_RAW:
                printf("SOCK_RAW (raw) \n");
                break;
            case SOCK_RDM:
                printf("SOCK_RDM (reliable message datagram)\n");
                break;
            case SOCK_SEQPACKET:
                printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_socktype);
                break;
        }
        printf("\tProtocol: ");
        switch (ptr->ai_protocol) {
            case 0:
                printf("Unspecified\n");
                break;
            case IPPROTO_TCP:
                printf("IPPROTO_TCP (TCP)\n");
                break;
            case IPPROTO_UDP:
                printf("IPPROTO_UDP (UDP) \n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_protocol);
                break;
        }
        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
        printf("\tCanonical name: %s\n", ptr->ai_canonname);
    }

    
  printf("\n");

  ////////////////////////////////////////////////////////////////////////////////////////////////


  // Create a Socket (A socket is just a point where we can send and recieve data from the network.)
  // Kind of neat to think of an ethernet port as a "socket" because they are kind the same thing but one hardware and one software
  ConnectSocket = INVALID_SOCKET;

  // Attempt to connect to the first address returned by
  // the call to getaddrinfo
  ptr=result;

  // Create a SOCKET for connecting to server
  ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
      ptr->ai_protocol);

  if (ConnectSocket == INVALID_SOCKET) {
    printf("Error at socket(): %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }
  else {
    printf("Client Side Socket Successfully Created!\n");
  }

  /////////////////////////////////////////////////
  // Now are trying to connect to another server //
  /////////////////////////////////////////////////

  // Connect to server.
  iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
      iResult = WSAGetLastError();
      printf("Error code trying to connect to server: %d\n", iResult);
      closesocket(ConnectSocket);
      ConnectSocket = INVALID_SOCKET;
  }

  // Should really try the next address returned by getaddrinfo
  // if the connect call failed
  // But for this simple example we just free the resources
  // returned by getaddrinfo and print an error message

  freeaddrinfo(result);

  if (ConnectSocket == INVALID_SOCKET) {
      printf("Unable to connect to server.\n");
  }
  else {
    printf("Successfully Connected to the server!\n");
  }

  return 0;
}


/////////////////////////////////////////////////
/////////////////////////////////////////////////
////////////////  MAIN FUNCTION  ////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  WSADATA wsaData;

  int iResult;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
      printf("WSAStartup failed: %d\n", iResult);
      return 1;
  }
  else {
    printf("Initilized Winsock Successfully!\n");
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  // https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-server //
  // This was used as a guide for the code below. The purpose is to set up a server side     //
  // socket. Future me can reference this when I inevitably forget everything I have done    //
  /////////////////////////////////////////////////////////////////////////////////////////////


  struct addrinfo *result_s = NULL, *ptr_s = NULL, hints_s;

  ZeroMemory(&hints_s, sizeof (hints_s));
  hints_s.ai_family = AF_INET; // Says to use IPv4
  hints_s.ai_socktype = SOCK_STREAM; // Use a Stream Socket
  hints_s.ai_protocol = IPPROTO_TCP; // Use TCP protocol
  hints_s.ai_flags = AI_PASSIVE; // Use the Socket Address in the bind function (The bind function binds a local address with a socket)

  // Resolve the local address and port to be used by the server
  iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints_s, &result_s); // Because we set AI_PASSIVE and the first thing is NULL it will give us an IP suitable for this socket based on our other specifications
  if (iResult != 0) {
      printf("getaddrinfo (Server) failed: %d\n", iResult);
      WSACleanup();
      return 1;
  }

  // This is the server socket that will listen for client connections
  SOCKET ListenSocket = INVALID_SOCKET;


  // Create a SOCKET for the server to listen for client connections
  ListenSocket = socket(result_s->ai_family, result_s->ai_socktype, result_s->ai_protocol);
  // The result_s is the first thing in a linked list given from the getaddrinfo function above

  if (ListenSocket == INVALID_SOCKET) {
    printf("Error at socket(): %ld\n", WSAGetLastError());
    freeaddrinfo(result_s);
    WSACleanup();
    return 1;
  }
  else {
    printf("Server Socket Successfully created!\n");
  }


  // https://docs.microsoft.com/en-us/windows/win32/winsock/binding-a-socket

  // Setup the TCP listening socket
  iResult = bind( ListenSocket, result_s->ai_addr, (int)result_s->ai_addrlen);
  // The bind function will 'bind' the address from the getaddrinfo function to the listenSocket that was created.
  if (iResult == SOCKET_ERROR) {
      printf("bind failed with error: %d\n", WSAGetLastError());
      freeaddrinfo(result_s);
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
  }
  else {
    printf("Successfully bound the Listen Socket!\n");
  }

  // Now that the socket is bound we no longer need the addrinfo
  freeaddrinfo(result_s);

  // https://docs.microsoft.com/en-us/windows/win32/winsock/listening-on-a-socket

  // Now we call the listen function to listen for connection requests to our socket 
  // The SOMAXCONN says that we allow a queue of connection to a certain length
  if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
      printf( "Listen failed with error: %ld\n", WSAGetLastError() );
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
  }
  else {
     // This below is just me trying to find the port that the Listening socket is listening on
    int addrlen;
    struct sockaddr_in server_addr;

    addrlen = sizeof(server_addr);
    iResult = getsockname(ListenSocket,(struct sockaddr *)&server_addr, &addrlen);
    if (iResult == -1){     
            printf("GETSOCKNAME ERROR\n");// = %d\n", sock_errno());
    }
    else {
      printf("Socket is now Listening on: %s:%d\n",inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    }

  }

  // https://docs.microsoft.com/en-us/windows/win32/winsock/accepting-a-connection

  // Now we make another socket for accepting client connections
  SOCKET ClientSocket;
  ClientSocket = INVALID_SOCKET;

  accept_connections(ClientSocket, ListenSocket);

  // This is here to do it again. If the function above does not get a connection then this below will take the argv's and
  // try and connected to another socket with the given ip and port. Then we run accept connections again to connect back to the attemped connection.

  SOCKET ConnectSocket = INVALID_SOCKET;

  connect_to_ip(argv[1], argv[2], ConnectSocket);

  accept_connections(ClientSocket, ListenSocket);


  char * message;

  std::cin >> message;

  int recvbuflen = 10;

  // Send an initial buffer
  iResult = send(ConnectSocket, message, (int) strlen(message), 0);
  if (iResult == SOCKET_ERROR) {
      printf("send failed: %d\n", WSAGetLastError());
      closesocket(ConnectSocket);
      WSACleanup();
      return 1;
  }

  printf("Bytes Sent: %ld\n", iResult);



  /*
  std::string command = "";
  while (command != "exit") {

    if (command == "connect") {
      char * connect_ip;
      char * connect_port;
      printf("IP to connect to: ");
      //std::cin >> connect_ip;
      printf("Port to connect to: ");
      //std::cin >> connect_port;
      connect_to_ip(connect_ip, connect_port, ConnectSocket);
    }

    printf("Enter Command: ");
    std::cin >> command;
  }
  */


  system("\npause");
  WSACleanup();
  closesocket(ClientSocket);
  closesocket(ListenSocket);
  closesocket(ConnectSocket);
  return 0;
}