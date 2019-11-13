#pragma once

#include <map>
#include <list>
#include <functional>
#include <iostream>

#include <memory>

class MinNetGameObject;
class MinNetRoom;

using FileCache = std::map<std::string, std::list<std::string>>;
using ComponentCache = std::map<std::string, std::function<void(MinNetGameObject *)>>;
using RoomCache = std::map<std::string, std::function<void(MinNetRoom *)>>;

static class MinNetCache
{
public:

	MinNetCache();
	~MinNetCache();
	
	static ComponentCache componentCache;
	static RoomCache roomCache;


	static void SetComponentCache(std::string prefabName, std::function<void(MinNetGameObject *)> f);
	static void AddComponent(MinNetGameObject * object);

	static void SetRoomCache(std::string prefabName, std::function<void(MinNetRoom *)> f);
	static void AddRoom(MinNetRoom * room);
};