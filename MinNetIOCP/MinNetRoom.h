#pragma once

#include <string>
#include <iostream>
#include <list>
#include <map>
#include "MinNet.h"
#include "MinNetOptimizer.h"
#include "EasyContainer.h"
#include "MinNetp2pGroup.h"

#include <DetourCommon.h>
#include <DetourNavMeshQuery.h>
#include <DetourCrowd.h>
#include <Recast.h>

#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <boost\thread.hpp>

class MinNetIOCP;
class MinNetUser;
class MinNetPacket;
class MinNetRoomManager;
class MinNetNavMeshAgent;

class MinNetGameObject;

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;
static const int MAX_POLYS = 256;
static const int MAX_SMOOTH = 2048;



class MinNetRoom
{
public:

	MinNetRoom(boost::asio::io_service& service);
	~MinNetRoom();

	void SetName(std::string name);
	std::string GetName();
	void SetNumber(int number);
	int GetNumber();
	void SetMaxUser(int max);
	int GetMaxUser();
	int UserCount();
	bool IsPeaceful();

	void SetLock(bool lock);

	MinNetp2pGroup * Createp2pGroup();

	std::list<MinNetUser *> * GetUserList();

	bool destroyWhenEmpty = false;
	bool changeRoom = false; // 룸이 실행되는 도중 룸 옵션을 바꿀때 true로
	std::string changeRoomName = "";

	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName);
	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName, Vector3 position, Vector3 euler);
	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName, Vector3 position, Vector3 euler, int id, bool casting = false, MinNetUser * except = nullptr, bool autoDelete = false);

	void Destroy(std::string prefabName, int id, bool casting = false, MinNetUser * except = nullptr);
	void Destroy();

	void ObjectInstantiate(MinNetUser * user, MinNetPacket * packet);
	void ObjectDestroy(MinNetUser * user, MinNetPacket * packet);

	void SetManager(MinNetRoomManager * manager);
	MinNetRoomManager * GetManager();

	void ObjectSyncing(MinNetUser * user);

	void AddUser(MinNetUser * user);
	void RemoveUser(MinNetUser * user);

	EasyContainer roomOption;

	void RemoveUsers();

	void AddObject(std::shared_ptr<MinNetGameObject> object);
	void RemoveObject(std::shared_ptr<MinNetGameObject> object);
	void RemoveObject(int id);
	void RemoveObjects();
	std::shared_ptr<MinNetGameObject> GetGameObject(int id);

	std::shared_ptr<MinNetGameObject> GetGameObject(std::string prefabName);
	std::vector<std::shared_ptr<MinNetGameObject>> & GetGameObjects(std::string prefabName);

	void ChangeRoom(std::string roomName);

	void Update(float tick);
	void LateUpdate();

	int GetNewID();

	MinNetUser * GetUser(int id);

	void ObjectRPC(MinNetUser * user, MinNetPacket * packet);
	void SendRPC(int objectId, std::string componentName, std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters, bool isTcp);
	void SendRPC(int objectId, std::string componentName, std::string methodName, MinNetUser * target, MinNetPacket * parameters, bool isTcp);

	void SetSceneName(std::string sceneName);

	boost::asio::io_service::strand strand;
	
	void ResetNavSystem();
	void SetNavSystem(dtNavMesh * navMesh);

	int GetMaxNavAgent();
	void SetMaxNavAgent(int max);

	float GetMaxNavAgentRadius();
	void SetMaxNavAgentRadius(float radius);

	void AddNavAgent(MinNetNavMeshAgent * agent);
	void DelNavAgent(int idx);
	void UpdateAgentParameters(int idx, const dtCrowdAgentParams * params);
	void RequestMoveTarget(const int idx, Vector3 endPosition, Vector3 halfExtents);

	void SetObstacleAvoidanceParams(const int idx, float velBias, float adaptiveDivs, float adaptiveRigns, float adaptiveDepth);
	bool GetPointOnPoly(float * position, float * halfExtents, float * pointOnPoly, dtPolyRef * polyRef);

	//void SetDefaultQueryFilter();

	enum MinNetPolyAreas
	{
		POLYAREA_GROUND,
		POLYAREA_WATER,
		POLYAREA_ROAD,
		POLYAREA_DOOR,
		POLYAREA_GRASS,
		POLYAREA_JUMP,
	};
	enum MinNetPolyFlags
	{
		POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		POLYFLAGS_ALL = 0xffff	// All abilities.
	};

private:

	void UpdateCrowd(float tick);

	dtCrowdAgentDebugInfo agentDebugInfo;

	dtNavMeshQuery * navMeshQuery = nullptr;// 네비게이션 메쉬에서 기본적인 길찾기를 수행하는 곳
	dtCrowd * crowd = nullptr;// 찾은 길에 따라 이동하는 오브젝트들의 모임
	dtQueryFilter filter;// A* 설정

	std::string nowSceneName = "";

	std::string name = "";

	int maxNavAgent = 100;
	float maxNavAgentRadius = 0.6f;

	int room_number = 0;
	int max_user = 10;

	int id_count = 0;

	bool lock = false;// 방 접속 차단

	std::map<int, MinNetUser *> user_map;
	std::list<MinNetUser *> user_list;

	std::map<int, std::shared_ptr<MinNetGameObject>> object_map;
	std::list<std::shared_ptr<MinNetGameObject>> object_list;

	std::list<MinNetp2pGroup *> p2pGroupList;

	MinNetRoomManager * manager = nullptr;
};

class MinNetRoomManager
{
public:
	MinNetRoomManager();
	~MinNetRoomManager();

	MinNetRoom * GetPeacefulRoom(std::string roomName);
	MinNetRoom * GetRoom(int roomId);

	void Send(MinNetRoom * room, MinNetPacket * packet, MinNetUser * except = nullptr);
	void Send(MinNetUser * user, MinNetPacket * packet);

	void PacketHandler(MinNetUser * user, MinNetPacket * packet);
	
	void Update(float tick);
	void LateUpdate();

	std::list<MinNetRoom *>& GetRoomList();

private:

	void UserEnterRoom(MinNetUser * user, MinNetPacket * packet);

	int roomNumberCount = -1;
	MinNetRoom * CreateRoom(std::string roomName, MinNetPacket * packet);
	void DestroyRoom(MinNetRoom * room);

	std::list<MinNetRoom *> room_list;

	boost::asio::io_service service;
	boost::thread_group io_threads;
	boost::asio::io_service::work work;
};