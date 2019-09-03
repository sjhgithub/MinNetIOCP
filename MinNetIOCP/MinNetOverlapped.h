#pragma once

#include <WinSock2.h>

class MinNetUser;
class MinNetPacket;

struct MinNetOverlapped : OVERLAPPED
{
	enum TYPE
	{
		ACCEPT, CLOSE, RECV, SEND
	};

	TYPE type;
};

struct MinNetAcceptOverlapped : MinNetOverlapped
{
	SOCKET socket;
	char buf[1024] = { '\0' };
	DWORD dwBytes;
};

struct MinNetCloseOverlapped : MinNetOverlapped
{
	MinNetUser * user;
};
struct MinNetSendOverlapped : MinNetOverlapped
{
	MinNetUser * user;
	WSABUF wsabuf;
};

struct MinNetRecvOverlapped : MinNetOverlapped
{
	MinNetUser * user;
	WSABUF wsabuf;
};
