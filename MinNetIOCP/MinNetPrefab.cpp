#include "MinNetPrefab.h"
#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNet.h"

GameObjectPrefab MinNetPrefab::gameObjectPrefab = GameObjectPrefab();
RoomPrefab MinNetPrefab::roomPrefab = RoomPrefab();
ScenePrefab MinNetPrefab::scenePrefab = ScenePrefab();
NavMeshPrefab MinNetPrefab::navMeshPrefab = NavMeshPrefab();


MinNetPrefab::MinNetPrefab()
{
}


MinNetPrefab::~MinNetPrefab()
{
}

void MinNetPrefab::SetGameObjectPrefab(std::string prefabName, std::function<void(MinNetGameObject *)> f)
{
	if (gameObjectPrefab.find(prefabName) == gameObjectPrefab.end())
	{// 중복 키값이 없음
		gameObjectPrefab.insert(std::make_pair(prefabName, f));
	}
	else
	{// 키값이 중복됨
		std::cout << prefabName.c_str() << " 오브젝트는 이미 캐시에 있습니다." << std::endl;
	}
}

void MinNetPrefab::AddComponent(MinNetGameObject * object)
{
	auto prefabName = object->GetName();

	auto prefab = gameObjectPrefab.find(prefabName);

	if (prefab == gameObjectPrefab.end())
	{// 캐시가 없음
		std::cout << prefabName.c_str() << " 오브젝트는 캐시에 없습니다." << std::endl;
	}
	else
	{
		auto function = prefab->second;
		function(object);
	}
}

void MinNetPrefab::SetRoomPrefab(std::string prefabName, std::function<void(MinNetRoom *, MinNetPacket *)> f)
{
	if (roomPrefab.find(prefabName) == roomPrefab.end())
	{// 중복 키값이 없음
		roomPrefab.insert(std::make_pair(prefabName, f));
	}
	else
	{// 키값이 중복됨
		std::cout << prefabName.c_str() << " 오브젝트는 이미 캐시에 있습니다." << std::endl;
	}
}

void MinNetPrefab::AddRoom(MinNetRoom * room, MinNetPacket * packet)
{
	auto roomName = room->GetName();

	auto prefab = roomPrefab.find(roomName);

	if (prefab == roomPrefab.end())
	{// 캐시가 없음
		roomPrefab.insert(std::make_pair(roomName, nullptr));
	}
	else
	{
		auto function = prefab->second;
		if(function != nullptr)
			function(room, packet);
	}

	room->SetSceneName(GetScenePrefab(roomName));
}

void MinNetPrefab::SetScenePrefab(std::string roomName, std::string sceneName)
{
	scenePrefab.insert(std::make_pair(roomName, sceneName));
}

std::string MinNetPrefab::GetScenePrefab(std::string roomName)
{
	std::string retval = "";

	auto prefab = scenePrefab.find(roomName);
	if (prefab == scenePrefab.end())
	{// 저장된 캐시가 없음
		SetScenePrefab(roomName, retval);
	}
	else
	{// 저장된 캐시가 있음
		retval = prefab->second;
	}

	return retval;
}

void MinNetPrefab::SetNavMeshPrefab(MinNetRoom * room, std::string navMeshName)
{
	if (room == nullptr) return;

	auto prefab = navMeshPrefab.find(navMeshName);
	
	dtNavMesh * navMesh = nullptr;

	if (prefab == navMeshPrefab.end())
	{// 찾아놓은 네비게이션 메쉬가 없음
		navMesh = loadAll(navMeshName.c_str());
		navMeshPrefab.insert(std::make_pair(navMeshName, navMesh));
	}
	else
	{// 있음
		navMesh = prefab->second;
	}

	if (navMesh == nullptr) return;

	room->ResetNavSystem();
	room->SetNavSystem(navMesh);
}

dtNavMesh * MinNetPrefab::loadAll(const char * path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return nullptr;

	// Read header.
	NavMeshSetHeader header;
	size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return nullptr;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return nullptr;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return nullptr;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return nullptr;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return nullptr;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return nullptr;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return nullptr;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	return mesh;
}
