#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

/**
 * \brief Gets the name of this application. 
 * \remarks This constant is mainly used for the caption of windows and/or message boxes.
 */
#define PROG_NAME		"Simple Finger Query"

/**
 * \brief Gets the address of the host to which to connect in order to obtain Finger information.
 */
#define	HOST_NAME			"cerfnet.com"		// This can be any valid host name

/**
 * \brief Gets a code that specifies we want Winsock 1.1.
 */
#define WINSOCK_VERSION		0x0101				// Program requires Winsock version 1.1.

/**
 * \brief Gets the Finger query to send to the server.
 */
#define FINGER_QUERY		"lonetech"			// This can be a login or real name

/**
 * \brief Gets a value that indicates which protocol to use as the default.
 */
#define	DEFAULT_PROTOCOL	0					// No protocol specified

/**
 * \brief Gets flags to pass to the Windows Sockets send function.
 */
#define	SEND_FLAGS			0					// No send() flags specified

/**
 * \brief Gets flags to pass to the Windows Sockets recv function.
 */
#define RECV_FLAGS			0					// No recv() flags specified

///////////////////////////////////////////////////////////////////////////////
// Error handling helper functions

/**
 * \brief Shows an informational message to the user.
 * \param pszMessage The content of the message box.
 * \param pszCaption The caption (title) of the message box.
 */
void ShowInformationWithCaption(LPCSTR pszMessage, LPCSTR pszCaption)
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

/**
 * \brief Shows an informational message to the user.
 * \param pszMessage The content of the message box.
 * \remarks The caption of the message box is automatically set to the name
 * of this application, as given by the PROG_NAME constant.
 */
void ShowInformation(LPCSTR pszMessage)
{
	if (pszMessage == NULL
		|| strlen(pszMessage) == 0)
	{
		return;
	}

	MessageBoxA(NULL, pszMessage, PROG_NAME, MB_OK | MB_ICONINFORMATION);
}

/**
 * \brief Shows an error (i.e., fatal) message to the user.
 * \param pszMessage The content of the message box.
 * \remarks The caption of the message box is automatically set to the name
 * of this application, as given by the PROG_NAME constant.
 */
void ShowStopError(LPCSTR pszMessage)
{
	if (pszMessage == NULL
		|| strlen(pszMessage) == 0)
	{
		return;
	}

	MessageBoxA(NULL, pszMessage, PROG_NAME, MB_OK | MB_ICONSTOP);
}

/**
 * \brief Gets the error information from the Windows Sockets subsystem for
 * the most-recently executed socket operation that has failed.
 * \remarks Translates the result of calling WSAGetLastError() into a human-
 * readable error message.  Displays the message in a Stop message box.
 */
void HandleError()
{
	char *s = NULL;

	// Translates the output of the WSAGetLastError() function into a
	// human-understandable message.
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&s, 0, NULL);

	ShowStopError(s);

	LocalFree(s);
}

/**
 * \brief Gets the error information from the Windows Sockets subsystem for
 * the most-recently executed socket operation that has failed.
 * \param pszMessage Additional message that should be displayed on the first
 * line of the message box, above the Windows Sockets error text.  This can 
 * be used to provide additional error information to the user.
 * \remarks Translates the result of calling WSAGetLastError() into a human-
 * readable error message.  Displays the message in a Stop message box.
 */
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
		(LPSTR)&s, 0, NULL);

	wsprintf(szBuffer, "%s\r\n\r\n%s", pszMessage, s);

	ShowStopError(szBuffer);

	LocalFree(s);
}

///////////////////////////////////////////////////////////////////////////////
// Program entry point

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, INT nCmdShow)
{
	WSADATA		wsaData;				// Winsock implementation details
	SOCKADDR_IN	sockAddr;				// Socket address structure
	short		iFingerPort;			// Well-known port assignment is 79
	char		szFingerInfo[5000];		// Buffer to hold Finger information
	char		szFingerQuery[1000];	// Buffer to hold Finger query
	int			nCharRecv;				// Number of characters received

	const int nResult = WSAStartup(WINSOCK_VERSION, &wsaData);

	if (nResult != 0)
	{
		ShowStopError("Could not load Windows Sockets DLL.");

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Resolve the host name
	const LPHOSTENT lpHostEnt = gethostbyname(HOST_NAME);

	if (!lpHostEnt)
	{
		ShowStopError("Could not get IP address.");

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Create the socket
	const SOCKET nSocket = socket(PF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
	if (nSocket == INVALID_SOCKET)
	{
		ShowStopError("Unable to open a connection endpoint for communicating with the remote host.");

		WSACleanup();	// Free all allocated program resources and exit
		return 1;
	}

	// Configure the socket
	// Get the Finger service information
	const LPSERVENT lpServEnt = getservbyname("Finger", NULL);

	if (lpServEnt == NULL)
		iFingerPort = IPPORT_FINGER;		// use the well-known port
	else
		iFingerPort = lpServEnt->s_port;

	// Define the socket address
	sockAddr.sin_family = AF_INET;			// Internet address family
	sockAddr.sin_port = iFingerPort;
	sockAddr.sin_addr = *((LPIN_ADDR)*lpHostEnt->h_addr_list);

	// Connect the socket
	int nConnectResult = connect(nSocket, (LPSOCKADDR)&sockAddr,
	                       sizeof(sockAddr));

	if (nConnectResult != 0)		/* failed to connect to the server. */
	{
		HandleErrorWithUserMessage(
			"Unable to connect to the remote message.");

		WSACleanup();
		return 1;
	}

	// Format and send the Finger query
	wsprintf(szFingerQuery, "%s\r\n", FINGER_QUERY);

	const int nCharactersSent = send(nSocket, szFingerQuery,
	                     lstrlen(szFingerQuery), SEND_FLAGS);

	if (nCharactersSent == SOCKET_ERROR)		/* failed to send */
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
			(LPSTR)&szFingerInfo[nConnectResult /* This will be zero when we start */],
			sizeof(szFingerInfo) - nConnectResult,
			RECV_FLAGS);
		nConnectResult += nCharRecv;
	} while (nCharRecv > 0);

	if (nCharRecv == SOCKET_ERROR)
	{
		HandleErrorWithUserMessage(
			"Problem was encountered trying to receive response.");

		WSACleanup();
		return 1;
	}

	// Report the results of the Finger query
	wsprintf(szFingerQuery, "%s@%s",
		FINGER_QUERY, HOST_NAME);

	ShowInformationWithCaption(szFingerInfo, szFingerQuery);

	return 0;
}