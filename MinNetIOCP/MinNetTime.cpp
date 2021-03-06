#include "MinNetTime.h"

float MinNetTime::deltaTime = 0.0f;
float MinNetTime::lastFrameTime = 0.0f;


float MinNetTime::time()
{
	return std::clock() * 0.001f;
}

void MinNetTime::FrameEnd()
{
	float nowTime = MinNetTime::time();

	deltaTime = (nowTime - lastFrameTime);

	lastFrameTime = nowTime;
}

MinNetTime::MinNetTime()
{
}


MinNetTime::~MinNetTime()
{
}
