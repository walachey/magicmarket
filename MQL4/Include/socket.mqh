#include <winsock.mqh>
string socket_protocol="TCP";


//+----------------------------------------------------------------------------+
//| Lovely function that helps us to get ANSI strings from DLLs to our UNICODE |
//| format                                                                     |
//| http://forum.mql4.com/60708                                                |
//+----------------------------------------------------------------------------+

#import "kernel32.dll"
   int lstrlenA(int);
   void RtlMoveMemory(uchar & arr[], int, int);
   int LocalFree(int); // May need to be changed depending on how the DLL allocates memory
#import
string mql4_mysql_ansi2unicode(int ptrStringMemory)
{
  int szString = lstrlenA(ptrStringMemory);
  uchar ucValue[];
  ArrayResize(ucValue, szString + 1);
  RtlMoveMemory(ucValue, ptrStringMemory, szString + 1);
  string str = CharArrayToString(ucValue);
  LocalFree(ptrStringMemory);
  return str;
}
//+----------------------------------------------------------------------------+ 

string inet_ntoa2(int addr)
{
	return mql4_mysql_ansi2unicode(inet_ntoa(addr));
}

#define RECV_BUFFER_SIZE 1

int err = 0;
int err(int code) {
  err = code;
  return(code);
}
int errno() {
  int ret = err;
  err = 0;
  return(ret);
}
 
int socket_initialized = 0;
int sock_start()
{
	if (socket_initialized) return 0;
	socket_initialized = 1;
	
	 int wsaData[WSADATA];
    int retval = WSAStartup(0x202, wsaData);
    if (retval != 0) {
        Print("Server: WSAStartup() failed with error "+ retval);
        err(-1);
        return(-1);
    } else
       Print("Server: WSAStartup() is OK.");
   return 0;
}
 
int sock_close(int sock) {
  int ret = closesocket(sock);
  err(ret);
  return(ret);
}
 
int sock_receive(int msgsock, int buffer_size, string &outString) {
   outString = "";
   int Buffer[RECV_BUFFER_SIZE];
   int retval = recv(msgsock, Buffer, 1, 0);
   if (retval == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) return 0;
   
   if (retval == SOCKET_ERROR) {
       Print("Server: recv() failed: error "+ WSAGetLastError());
       err(-1);
       return -1;
   } else
       // Print("Server: recv() is OK.");
   if (retval == 0) {
      Print("Server: Client closed connection.\n");
      err(-1);
      return -1;
   }
   outString = struct2str(Buffer,ArraySize(Buffer)<<18); 
   outString = StringSubstr(outString,0,retval);
   
   return 1;
}
          
int sock_send(int msgsock, string response) {
   int SendBuffer[];
   StringAdd(response, CharToStr(0));
   //StringAdd(response, "#");
   int messageSize = StringLen(response);
   ArrayResize(SendBuffer,messageSize); 
   str2struct(SendBuffer,ArraySize(SendBuffer)<<18,response);
   //SendBuffer[messageSize] = 0; // terminate with NULL
   int ret = send(msgsock,SendBuffer, messageSize,0);
   if (ret <= 0) {
      Print("Server: send() failed: error "+WSAGetLastError());
      err(ret);
   } else {
      //Print("Server: send() is OK."); 
      err(0); 
   }
   return(ret);
}

int sock_connect(int port, string ip_address, bool listen)
{
	int listen_socket;
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
 
	if (listen_socket == INVALID_SOCKET){
		Print("Server: socket() failed with error "+WSAGetLastError());
		err(-1);
		return(-1);
	} else
		Print("Server: socket() is OK.");
   
       
 	socket_sockaddr_in addrinfo;
	addrinfo.sin_family = AF_INET;
	addrinfo.sin_port = htons(port);
	uchar ip_address_buf[];
	StringToCharArray(ip_address, ip_address_buf);
	addrinfo.sin_addr.S_addr = inet_addr(127, 0, 0, 1);//inet_addr(ip_address_buf);
	Print (inet_ntoa2(addrinfo.sin_addr.S_addr));
	if (connect(listen_socket, memcpy(addrinfo, addrinfo, 0), sizeof(addrinfo)) == SOCKET_ERROR) {
        Print("Server: connect() failed with error "+WSAGetLastError());
        err(-1);
        return(-1);
    } else
        Print("Server: connect() is OK");
	
	// set nonblocking
	int mode[1];
	mode[0] = 1;
	ioctlsocket(listen_socket, FIONBIO, mode);
	
	// no nagel
	mode[0] = 0;
	setsockopt(listen_socket, SOL_SOCKET, SO_SNDBUF, mode, 4);
	setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, mode, 1);
	return(listen_socket);
}
/*
int sock_open(int port,string ip_address, bool listen) {
    int listen_socket; 
    int Buffer[2048];

    int fromlen[1];
    int loopcount=0;
    int socket_type = SOCK_STREAM;
    int  local[sockaddr_in], from[sockaddr_in];
   
    if (port==0) {
      err(-1);
      return(-1);
    }    

 
    int2struct(local,sin_family,AF_INET);
    Print(AF_INET+" family:"+local[0]+" "+local[1]+" "+local[2]+" "+local[3]+" f:"+sin_family);
    if (ip_address=="") 
      int2struct(local,sin_addr,INADDR_ANY); 
    else  
      int2struct(local,sin_addr,inet_addr(ip_address)); 
    Print(inet_addr(ip_address)+" addr:"+local[0]+" "+local[1]+" "+local[2]+" "+local[3]+" f:"+sin_addr);
    int2struct(local,sin_port,htons(port));
    Print(htons(port)+" port:"+local[0]+" "+local[1]+" "+local[2]+" "+local[3]+" f:"+sin_port);
    listen_socket = socket(AF_INET, socket_type,0);
 
    if (listen_socket == INVALID_SOCKET){
        Print("Server: socket() failed with error "+WSAGetLastError());
        err(-1);
        WSACleanup();
        return(-1);
    } else
       Print("Server: socket() is OK.");
    Print("sin_family:"+struct2int(local,sin_family)+" sin_port:"+struct2int(local,sin_port)+" sin_addr:"+struct2int(local,sin_addr));
    
    if (listen)
    {
	    if (bind(listen_socket, local, ArraySize(local)<<2) == SOCKET_ERROR) {
	        Print("Server: bind() failed with error "+WSAGetLastError());
	        WSACleanup();
	        err(-1);
	        return(-1);
	    } else
	        Print("Server: bind() is OK");
	 
	    if (socket_type != SOCK_DGRAM) {
	        if (listen(listen_socket,5) == SOCKET_ERROR) {
	            Print("Server: listen() failed with error "+ WSAGetLastError());
	            err(-1);
	            return(-1);
	        } else
	            Print("Server: listen() is OK.");
	    }
	    Print("Server: listening and waiting connection port:"+port+", protocol:"+socket_protocol);
	}
	else
	{
		socket_sockaddr_in addrinfo;
		addrinfo.sin_family = AF_INET;
		addrinfo.sin_port = port;
		addrinfo.sin_addr = inet_addr(ip_address);
		if (connect(listen_socket, local, ArraySize(local)<<2) == SOCKET_ERROR) {
	        Print("Server: connect() failed with error "+WSAGetLastError());
	        err(-1);
	        return(-1);
	    } else
	        Print("Server: connect() is OK");
	}
	return(listen_socket);
 }
 */
int sock_accept(int listen_socket) {
    /*int msgsock;
    int fromlen[1];
    int  local[sockaddr_in], from[sockaddr_in];
    //after setup code   
    int closed = True;
    fromlen[0] =ArraySize(from)<<2;
    msgsock = accept(listen_socket, from, fromlen);
    if (msgsock == INVALID_SOCKET) {
        Print("Server: accept() error "+ WSAGetLastError());
        WSACleanup();
        err(-1);
        return(-1);
     } else {
        Print("Server: accept() is OK.\n");
     }
     Print("Server: accepted connection from "+inet_ntoa(struct2int(from,sin_addr))+", port "+ htons(struct2int(from,sin_port))) ;
   return(msgsock);*/
   return -1;
}
 
void sock_cleanup() {
  WSACleanup();
}
