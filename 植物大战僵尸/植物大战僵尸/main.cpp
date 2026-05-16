#include <ctime>
#include <easyx.h>
#include "Scene.h"
#include"GameObject.h"

int main()
{
	srand((unsigned)time(NULL));

	//场景搭建
	initgraph(1280, 720, EX_SHOWCONSOLE);

	Scene* scene = Scene::create();  //游戏初始化

	//设置游戏帧率
	float fps = 1000.0f / 60;  // 60帧  1 帧 约等于 16.6667毫秒
	clock_t begin_time = clock(), end_time = 0; //clock() 获取当前系统时间戳（毫秒）

	while (true)
	{
		end_time = clock();
		if (float(end_time - begin_time) >= fps) //保证一秒调用60次
		{
			scene->drawTick();
			scene->eventTick(end_time - begin_time);

			begin_time = end_time;
		}
	}

	return 0;
}