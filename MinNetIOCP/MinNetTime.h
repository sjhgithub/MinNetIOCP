#pragma once

#include <ctime>

static class MinNetTime
{
public:

	static float deltaTime;
	static float time();

	static void FrameEnd();

private:

	static float lastFrameTime;

	MinNetTime();
	~MinNetTime();
};