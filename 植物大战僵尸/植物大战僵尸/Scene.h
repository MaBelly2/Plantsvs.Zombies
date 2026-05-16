#pragma once
#include <easyx.h>
#include <vector>
#include <map>
#include "Plant.h"
#include "Zombie.h"
#include "Bullet.h"
#include "Sun.h"
#include "Lawnmower.h"
#include "Card.h"
using namespace std;

// 游戏当前状态枚举
enum GameStatus {
	PLAYING,    // 游戏中
	WIN,        // 胜利
	GAME_OVER   // 失败
};

class Scene
{
private:
	//把构造函数和拷贝构造封掉,使创建对象只能通过给的接口进行创建
	Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

private:
	IMAGE m_BackgroundImg;		//背景图片

	IMAGE m_ShovelSlotImg;		// 放在屏幕顶部的铲子槽位图标
	IMAGE m_ShovelImg;			// 拿在手里、跟着鼠标移动的铲子图片

	Plant* m_PlantTable[5][9];	//植物表格 这里只能指针 因为构造函数被封了
	vector<Zombie*> m_zombies;	//添加容器存放活动的僵尸和子弹
	vector<Bullet*> m_bullets;

	// 胜负判定相关的变量
	GameStatus m_status;      // 当前游戏状态
	int m_zombiesGenerated;   // 已经生成的僵尸数量
	int m_maxZombies;         // 本关总共需要生成的僵尸数量

	vector<Card*> m_cards;

	int m_sun;

public:
	static Scene* create();
public:
	//场景初始化
	bool init();
	//绘制更新
	void drawTick();
	//处理事件
	void eventTick(float delta);		//当作计时器使用

private:
	void checkCollision(float delta);	//碰撞检测
	void cleanUp();						//内存清理（垃圾回收）

private:
	bool m_isHoldingPlant;				// 玩家手里是否正拿着植物卡片准备种
	PlantType m_holdingType;			// 玩家手里拿的是什么植物

	bool m_isHoldingShovel;				// 铲子状态变量

	// 辅助函数：将鼠标的像素坐标（x,y) 转换成网格的二维数组下标 [row][col]
	bool getGridIndex(int x, int y, int& row, int& col);

	float m_zombieTimer = 0;			// 距离下次刷怪的累计时间
	float m_zombieInterval = 5.0f;		// 刷怪间隔（比如5秒一只）

private:
	std::vector<Sun*> m_suns;
	float m_skySunTimer = 0;			// 天空掉落计时器
	float m_skySunInterval = 10.0f;		// 每10秒掉一个

private:
	Lawnmower* m_lawnmowers[5];			// 存储 5 行的推车
};

