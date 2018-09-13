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

	ShowStopError(s);
	
	LocalFree(s);
}

void HandleErrorWithUserMessage(PCSTR pszMessage)
{
	if (pszMessage == NULL
		|| strlen(pszMessage) == 0)
		return;

	char szBuffer[512];
	char *s = NULL;

	// Translates the output of the WSAGetLastError() function into a
	// human-understandable message.
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);

	wsprintf(szBuffer, "%s\r\n\r\n%s", pszMessage, s);

	ShowStopError(szBuffer);
	
	LocalFree(s);
}

void ShowStopError(PCSTR pszMessage)
{
	if (pszMessage == NULL
		|| strlen(pszMessage) == 0)
	{
		return;
	}
	
	MessageBoxA(NULL, pszMessage, PROG_NAME, MB_OK | MB_ICONSTOP);
}

void ShowInformation(PCSTR pszMessage)
{
	if (pszMessage == NULL
		|| strlen(pszMessage) == 0)
	{
		return;
	}
	
	MessageBoxA(NULL, pszMessage, PROG_NAME, MB_OK | MB_ICONINFORMATION);
}

void ShowInformation(PCSTR pszMessage, PCSTR pszCaption)
{
	if (pszMessage == NULL
		|| strlen(pszMessage) == 0)
	{
		return;
	}

	if (pszCaption == NULL
		|| strlen(pszCaption) == 0)
	{
		return;
	}
	
	MessageBoxA(NULL, pszMessage, pszCaption, MB_OK | MB_ICONINFORMATION);
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

	int nResult = WSAStartup(WINSOCK_VERSION, &wsaData);
	
	if (nResult != 0)
	{
		ShowStopError("Could not load Windows Sockets DLL.");

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Resolve the host name
	lpHostEnt = gethostbyname(HOST_NAME);

	if (!lpHostEnt)
	{
		ShowStopError("Could not get IP address.");

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Create the socket
	nSocket = socket(PF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
	if (nSocket == INVALID_SOCKET)
	{
		ShowStopError("Unable to open a connection endpoint for communicating with the remote host.");

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

	// Connect the socket
	nConnect = connect(nSocket, (LPSOCKADDR)&sockAddr,
		sizeof(sockAddr));

	if (nConnect != 0)		/* failed to connect to the server. */
	{
		HandleErrorWithUserMessage(
			"Unable to connect to the remote message.");
		
		WSACleanup();
		return 1;
	}
	
	// Format and send the Finger query
	wsprintf(szFingerQuery, "%s\r\n", FINGER_QUERY);
	
	nCharSent = send(nSocket, szFingerQuery,
		lstrlen(szFingerQuery), SEND_FLAGS);
		
	if (nCharSent == SOCKET_ERROR)		/* failed to send */
	{
		HandleErrorWithUserMessage(
			"A problem was experienced trying to send the Finger query.");
		
		WSACleanup();
		return 1;
	}
	
	// Get the Finger information from the remote host
	do
	{
		nCharRecv = recv(nSocket,
			(LPSTR)&szFingerInfo[nConnect],
			sizeof(szFingerInfo) - nConnect,
			RECV_FLAGS);
		nConnect += nCharRecv;
	}
	while(nCharRecv > 0);
	
	if (nCharRecv == SOCKET_ERROR)
	{
		HandleErrorWithUserMessage(
			"Problem was encountered trying to receive response.");
		
		WSACleanup();
		return 1;
	}
	
	// Report the results
	wsprintf(szFingerQuery, "%s@%s", 
		FINGER_QUERY, HOST_NAME);
	
	ShowInformation(szFingerInfo, szFingerQuery);
	
	return 0;
}