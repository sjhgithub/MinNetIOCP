#pragma once

#include <map>
#include <list>
#include <functional>
#include <iostream>

#include <memory>

#include <DetourCommon.h>
#include <DetourNavMeshQuery.h>
#include <DetourCrowd.h>
#include <Recast.h>


class MinNetGameObject;
class MinNetRoom;
class MinNetPacket;

using FileCache = std::map<std::string, std::list<std::string>>;
using GameObjectPrefab = std::map<std::string, std::function<void(MinNetGameObject *)>>;
using RoomPrefab = std::map<std::string, std::function<void(MinNetRoom *, MinNetPacket *)>>;
using ScenePrefab = std::map<std::string, std::string>;
using NavMeshPrefab = std::map<std::string, dtNavMesh *>;

struct NavMeshSetHeader
{
	int magic;
	int version;
	int numTiles;
	dtNavMeshParams params;
};

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

static class MinNetPrefab
{
public:

	MinNetPrefab();
	~MinNetPrefab();
	
	static GameObjectPrefab gameObjectPrefab;// string의 이름을 갖는 게임오브젝트 에게 컴포넌트를 추가하는 용도로 쓸 람다의 집합
	static RoomPrefab roomPrefab;// string의 이름을 갖는 룸이 생성될 때 옵션을 주는 용도로 쓸 람다의 집합
	static ScenePrefab scenePrefab;// string의 이름을 갖는 룸이 사용할 유니티 씬 이름
	static NavMeshPrefab navMeshPrefab;


	static void SetGameObjectPrefab(std::string prefabName, std::function<void(MinNetGameObject *)> f);
	static void AddComponent(MinNetGameObject * object);

	static void SetRoomPrefab(std::string prefabName, std::function<void(MinNetRoom *, MinNetPacket *)> f);
	static void AddRoom(MinNetRoom * room, MinNetPacket * packet);

	static void SetScenePrefab(std::string roomName, std::string sceneName);
	static std::string GetScenePrefab(std::string roomName);

	static void SetNavMeshPrefab(MinNetRoom * room, std::string navMeshName);

private:

	static dtNavMesh * loadAll(const char * path);
};