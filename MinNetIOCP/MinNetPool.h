#pragma once

#include "MinNetOptimizer.h"
#include "MinNetOverlapped.h"

class MinNetRoom;
class MinNetGameObject;
class MinNetComponent;

static class MinNetPool
{

public:

	MinNetPool();
	~MinNetPool();

	static void Init();

public:

	static MinNetObjectPool<MinNetAcceptOverlapped> * acceptOverlappedPool;
	static MinNetObjectPool<MinNetCloseOverlapped> * closeOverlappedPool;
	static MinNetObjectPool<MinNetSendOverlapped> * sendOverlappedPool;
	static MinNetObjectPool<MinNetRecvOverlapped> * recvOverlappedPool;
	static MinNetObjectPool<MinNetUser> * userPool;
	static MinNetObjectPool<MinNetPacket> * packetPool;
};