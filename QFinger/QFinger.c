#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define PROG_NAME		"Simple Finger Query"

#define	HOST_NAME			"cerfnet.com"		// This can be any valid host name
#define WINSOCK_VERSION		0x0101				// Program requires Winsock version 1.1.

#define FINGER_QUERY		"lonetech"			// This can be a login or real name
#define	DEFAULT_PROTOCOL	0					// No protocol specified
#define	SEND_FLAGS			0					// No send() flags specified
#define RECV_FLAGS			0					// No recv() flags specified

///////////////////////////////////////////////////////////////////////////////
// Error handling helper function

void HandleError()
{
	char *s = NULL;

	// Translates the output of the WSAGetLastError() function into a
	// human-understandable message.
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);

	MessageBoxA(NULL, s, PROG_NAME, MB_OK | MB_ICONSTOP);
}

///////////////////////////////////////////////////////////////////////////////
// Program entry point

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, INT nCmdShow)
{
	WSADATA		wsaData;				// Winsock implementation details
	LPHOSTENT	lpHostEnt;				// Internet host information structure
	SOCKET		nSocket;				// Socket number used by this program
	SOCKADDR_IN	sockAddr;				// Socket address structure
	LPSERVENT	lpServEnt;				// Service information structure
	short		iFingerPort;			// Well-known port assignment is 79
	char		szFingerInfo[5000];		// Buffer to hold Finger information
	char		szFingerQuery[1000];	// Buffer to hold Finger query
	int			nCharSent;				// Number of characters transmitted
	int			nCharRecv;				// Number of characters received
	int			nConnect;				// Socket connection results

	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		MessageBox(NULL, "Could not load Windows Sockets DLL.",
			PROG_NAME, MB_OK | MB_ICONSTOP);

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Resolve the host name
	lpHostEnt = gethostbyname(HOST_NAME);

	if (!lpHostEnt)
	{
		MessageBox(NULL, "Could not get IP address.",
			PROG_NAME, MB_OK | MB_ICONSTOP);

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Create the socket
	nSocket = socket(PF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
	if (nSocket == INVALID_SOCKET)
	{
		MessageBox(NULL, "Unable to open a connection endpoint for communicating with the remote host.",
			PROG_NAME, MB_OK | MB_ICONSTOP);

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Configure the socket
	// Get the Finger service information
	lpServEnt = getservbyname("Finger", NULL);

	if (lpServEnt == NULL)
		iFingerPort = IPPORT_FINGER;		// use the well-known port
	else
		iFingerPort = lpServEnt->s_port;

	// Define the socket address
	sockAddr.sin_family = AF_INET;			// Internet address family
	sockAddr.sin_port = iFingerPort;
	sockAddr.sin_addr = *((LPIN_ADDR)*lpHostEnt->h_addr_list);

	return 0;
}