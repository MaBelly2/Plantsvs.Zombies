#include <ctime>
#include <windows.h>
#include <easyx.h>
#include "Scene.h"

int main()
{
	srand((unsigned)time(NULL));

	// 场景搭建
	initgraph(1280, 720, EX_SHOWCONSOLE);

	// 【性能修复1】开启 EasyX 双缓冲绘图，彻底解决画面闪烁和渲染卡顿
	BeginBatchDraw();

	Scene* scene = Scene::create();

	// 【性能修复2】使用 Windows 高精度计时器替代低精度的 clock()
	LARGE_INTEGER cpuFreq;
	QueryPerformanceFrequency(&cpuFreq); // 获取 CPU 频率
	double freq = (double)cpuFreq.QuadPart;

	LARGE_INTEGER lastTime, currentTime;
	QueryPerformanceCounter(&lastTime);

	// 目标帧率 60帧，每帧大约 16.6667 毫秒
	double targetDelta = 1000.0 / 60.0;

	while (true)
	{
		QueryPerformanceCounter(&currentTime);
		// 计算距离上一帧经过了多少毫秒
		double delta_ms = (currentTime.QuadPart - lastTime.QuadPart) * 1000.0 / freq;

		if (delta_ms >= targetDelta)
		{
			lastTime = currentTime;

			// 1. 逻辑更新（传入精确的毫秒差值，保证物理位移均匀）
			scene->eventTick((float)delta_ms);

			// 2. 画面绘制（写入后台缓冲区）
			cleardevice(); // 清空上一帧
			scene->drawTick();

			// 3. 将后台缓冲区的内容一次性刷入屏幕
			FlushBatchDraw();
		}
		else
		{
			// 如果当前帧计算太快，休眠 1 毫秒交出 CPU 控制权，避免 CPU 100% 满载发热
			Sleep(1);
		}

		// 退出机制：如果按了 ESC 键则退出游戏
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
	}

	// 退出游戏时的清理
	EndBatchDraw();
	delete scene; // 【关联内存修复】调用析构函数清理资源
	closegraph();

	return 0;
}