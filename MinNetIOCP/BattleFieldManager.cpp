#include "BattleFieldManager.h"
#include "PlayerMove.h"
#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNet.h"

void BattleFieldManager::AddPlayer(std::weak_ptr<MinNetComponent> player)
{
	playerList.push_back(player);
}

void BattleFieldManager::DelPlayer(std::weak_ptr<MinNetComponent> player)
{
	playerList.remove_if([player](std::weak_ptr<MinNetComponent> p) {
		auto srcPlayer = player.lock();
		auto dstPlayer = p.lock();

		if (srcPlayer && dstPlayer)
			return srcPlayer == dstPlayer;

		return false;
	});
}

void BattleFieldManager::ChangeState(BattleFieldState state, clock_t time)
{

}

void BattleFieldManager::InitRPC()
{
	DefRPC("ChangeState", [this](MinNetPacket * packet) {

		auto state = static_cast<BattleFieldManager::BattleFieldState>(packet->pop_int());
		auto time = static_cast<clock_t>(packet->pop_int());
		ChangeState(state, time);
	});
}

void BattleFieldManager::Awake()
{
	setRespawnPoints();

	SendChangeState(BattleFieldState::GameReady);
}

void BattleFieldManager::Update()
{	
	for (auto component : playerList)
	{
		if (!component.expired())
		{
			auto player = static_cast<PlayerMove *>(component.lock().get());

			PlayerRespawnUpdate(player);
			PlayerDieUpdate(player);
		}
		else
		{// 제거 해주어야 함

		}
	}

	switch (state)
	{
	case BattleFieldManager::BattleFieldState::GameReady:
		if (Time::curTime() - readyStartTime > readyTime)
		{// 게임 시작 해줌
			SendChangeState(BattleFieldState::GameStart);


			//for (auto component : playerList)
			//{
			//	if (!component.expired())
			//	{
			//		auto player = static_cast<PlayerMove *>(component.lock().get());
			//		auto team = getPeacefulTeam();
			//		player->RPC("Respawn", MinNetRpcTarget::All, getRespawnPoint(team), player->maxHP, static_cast<int>(team));
			//	}
			//}
		}
		break;
	case BattleFieldManager::BattleFieldState::GameStart:
		if (Time::curTime() - gameStartTime > gameTime)
		{// 게임 끝내고 결과 정산함
			SendChangeState(BattleFieldState::GameEnd);
		}

		break;
	case BattleFieldManager::BattleFieldState::GameEnd:
		if (Time::curTime() - gameEndTime > endTime)
		{// 잠시 대기했다가 점수 보여주고 넘김
			SendChangeState(BattleFieldState::GameReady);
		}

		break;
	case BattleFieldManager::BattleFieldState::MAX:
		break;
	}
}

void BattleFieldManager::OnInstantiate(MinNetUser * user)
{
	RPC("SyncState", user, static_cast<int>(state), getNowStateLeftTime());
}

clock_t BattleFieldManager::getNowStateLeftTime()
{
	clock_t sendTime = 0;
	clock_t delayedTime = 0;
	clock_t curTime = Time::curTime();

	switch (state)
	{
	case BattleFieldManager::BattleFieldState::GameReady:
		delayedTime = curTime - readyStartTime;
		sendTime = readyTime;
		break;

	case BattleFieldManager::BattleFieldState::GameStart:
		delayedTime = curTime - gameStartTime;
		sendTime = gameTime;
		break;

	case BattleFieldManager::BattleFieldState::GameEnd:
		delayedTime = curTime - gameEndTime;
		sendTime = endTime;
		break;

	case BattleFieldManager::BattleFieldState::MAX:
		break;
	}

	std::cout << "현재 상태에서 남은 시간 : " << (sendTime - delayedTime)  << " ms"<< std::endl;

	return sendTime - delayedTime;
}

PlayerMove::Team BattleFieldManager::getPeacefulTeam()
{
	int blueTeamCount = 0, redTeamCount = 0;

	for (auto component : playerList)
	{
		if (component.expired())
			continue;

		auto player = static_cast<PlayerMove *> (component.lock().get());
		if (player->team == PlayerMove::Team::Blue)
		{
			blueTeamCount++;
		}
		else if (player->team == PlayerMove::Team::Red)
		{
			redTeamCount++;
		}
	}

	if (blueTeamCount == redTeamCount)
	{// 사람수 같으면 랜덤으로 함
		if (rand() % 2 == 0)
			return PlayerMove::Team::Red;
		else
			return PlayerMove::Team::Blue;
	}
	else
	{
		if (blueTeamCount > redTeamCount)// 레드팀에 사람이 더 적음
			return PlayerMove::Team::Red;
		else// 블루팀에 사람이 더 적음
			return PlayerMove::Team::Blue;
	}
}

bool BattleFieldManager::isPeacefulTeam(PlayerMove::Team team)
{
	int blueTeamCount = 0, redTeamCount = 0;

	for (auto component : playerList)
	{
		if (component.expired())
			continue;

		auto player = static_cast<PlayerMove *> (component.lock().get());
		if (player->team == PlayerMove::Team::Blue)
		{
			blueTeamCount++;
		}
		else if (player->team == PlayerMove::Team::Red)
		{
			redTeamCount++;
		}
	}

	if (blueTeamCount == redTeamCount)
	{// 사람수 같으면 랜덤으로 함
		return true;
	}
	else
	{
		PlayerMove::Team peacefulTeam = PlayerMove::Team::None;

		if (blueTeamCount > redTeamCount)// 레드팀에 사람이 더 적음
			peacefulTeam = PlayerMove::Team::Red;
		else// 블루팀에 사람이 더 적음
			peacefulTeam = PlayerMove::Team::Blue;
		
		return peacefulTeam == team;
	}
}

void BattleFieldManager::setRespawnPoints()
{
	respawnPoints.resize(2);
	respawnPointCount.resize(2, 0);

	auto& redTeamPoints = respawnPoints[static_cast<int>(PlayerMove::Team::Red)];
	redTeamPoints.resize(9);
	redTeamPoints[0] = Vector3(69.52f, -10.361f, 34.03f);
	redTeamPoints[1] = Vector3(65.02f, -10.361f, 4.43f);
	redTeamPoints[2] = Vector3(90.42f, -10.361f, 2.43f);
	redTeamPoints[3] = Vector3(89.92f, -10.361f, -36.07f);
	redTeamPoints[4] = Vector3(65.62f, -10.361f, -36.47f);
	redTeamPoints[5] = Vector3(91.42f, -10.361f, -5.9699f);
	redTeamPoints[6] = Vector3(74.62f, -10.361f, -15.17f);
	redTeamPoints[7] = Vector3(77.82f, -10.361f, 20.43f);
	redTeamPoints[8] = Vector3(46.92f, -10.361f, 35.63f);

	auto& blueTeamPoints = respawnPoints[static_cast<int>(PlayerMove::Team::Blue)];
	blueTeamPoints.resize(9);
	blueTeamPoints[0] = Vector3(-4.5f, -12.6f, 14.28f);
	blueTeamPoints[1] = Vector3(-4.8f, -12.683f, 8.88f);
	blueTeamPoints[2] = Vector3(-8.2f, -10.94f, -7.52f);
	blueTeamPoints[3] = Vector3(-3.5f, -10.94f, -2.72f);
	blueTeamPoints[4] = Vector3(9.7f, -10.94f, -27.92f);
	blueTeamPoints[5] = Vector3(18.8f, -12.347f, 13.78f);
	blueTeamPoints[6] = Vector3(23.4f, -10.94f, 5.48f);
	blueTeamPoints[7] = Vector3(27.996, -10.94f, 0.597f);
	blueTeamPoints[8] = Vector3(23.0f, -12.695, -6.42f);
}

Vector3 BattleFieldManager::getRespawnPoint(PlayerMove::Team team)
{
	int teamIndex = static_cast<int>(team);
	int& count = respawnPointCount[teamIndex];

	auto respawnPoint = respawnPoints[teamIndex][count++];

	if (count >= respawnPoints[teamIndex].size())
	{
		count = 0;
	}

	return respawnPoint;
}


void BattleFieldManager::PlayerRespawnUpdate(PlayerMove * player)
{
	if (player->state == PlayerMove::State::Alive)
		return;

	if (player->team == PlayerMove::Team::Spectator)
	{// 관전자 중에
		if (player->nextSpawnTeam != PlayerMove::Team::None)
		{// 게임에 참여하려하는 사람이 있다면 팀에 넣어줌
			auto spawn = player->nextSpawnTeam;

			if (!isPeacefulTeam(spawn))
				spawn = getPeacefulTeam();

			auto spawnPosition = getRespawnPoint(spawn);

			player->RPC("Respawn", MinNetRpcTarget::All, spawnPosition, player->maxHP, static_cast<int>(spawn));
			player->nextSpawnTeam = PlayerMove::Team::None;
		}

		return;
	}

	if (player->IsCanRespawn(playerRespawnDelay))
	{// 리스폰 시켜줌
		auto& spawn = player->nextSpawnTeam;

		if (spawn != PlayerMove::Team::None)
		{
			player->RPC("Respawn", MinNetRpcTarget::All, getRespawnPoint(spawn), player->maxHP, static_cast<int>(spawn));
			spawn = PlayerMove::Team::None;
		}
		else
		{
			player->RPC("Respawn", MinNetRpcTarget::All, getRespawnPoint(player->team), player->maxHP, static_cast<int>(player->team));
		}
	}
}

void BattleFieldManager::PlayerDieUpdate(PlayerMove * player)
{
	if (player->state == PlayerMove::State::Die || player->team == PlayerMove::Team::Spectator)
		return;

	if (player->IsDie())
	{// 플레이어가 죽었음
		player->ChangeState(PlayerMove::State::Die);
		std::cout << "플레이어 사망" << std::endl;
	}
}

void BattleFieldManager::SendChangeState(BattleFieldState state)
{
	if (this->state == state)
		return;

	clock_t curTime = Time::curTime();

	switch (state)
	{
	case BattleFieldManager::BattleFieldState::GameReady:
		readyStartTime = curTime;
		break;

	case BattleFieldManager::BattleFieldState::GameStart:
		gameStartTime = curTime;
		break;

	case BattleFieldManager::BattleFieldState::GameEnd:
		gameEndTime = curTime;
		break;

	case BattleFieldManager::BattleFieldState::MAX:
		break;
	}

	this->state = state;

	RPC("SyncState", MinNetRpcTarget::All, static_cast<int>(state), getNowStateLeftTime());
}

BattleFieldManager::BattleFieldManager()
{
}

BattleFieldManager::~BattleFieldManager()
{
}