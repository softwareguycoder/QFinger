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
    // TODO: Add the application's code here
    return 0;
}