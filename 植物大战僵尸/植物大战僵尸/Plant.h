#pragma once
#include "GameObject.h"

//植物类型枚举
//B：根据想要添加的植物名称，在这里丰富枚举类型
enum PlantType {
	PEASHOOTER,	// 豌豆射手
	SUNFLOWER,	// 向日葵
	SNOWPEA,
	WALLNUT
};

// 定义一个植物数据的结构体
struct PlantData {
	int cost;					// 阳光造价
	int maxHp;					// 最大血量
	float cooldown;				// 射击/生产冷却时间
	float cardCooldown;			// 卡片重新可用的冷却时间（秒）
	const char* imgPathFormat;	// 图片路径
	int frameCount;				//动画帧数
};

class Plant :public GameObject
{
	Plant() = default;
	Plant(const Plant&) = default;
	Plant& operator=(const Plant&) = default;

private:
	PlantType m_type;
	float m_cdTimer;				//冷却计时器

	bool m_shouldFire = false;		// 开火信号灯
	bool m_shouldSpawnSun = false;	// 阳光信号灯
	
public:
	static Plant* create(PlantType type, Vec2 pos, int w, int h);

	bool init(Vec2 pos, int w, int h)override;	//override 检查是否在重写一个虚函数
	void drawTick()override;
	void eventTick(float delta)override;

	//植物自己的功能
	void setType(PlantType type);

	static int getCost(PlantType type);
	static float getCardCooldown(PlantType type);	// 获取卡片最大CD

	// 1. 射击子弹相关
	bool shouldFire() { return m_shouldFire; }
	void resetFireFlag() { m_shouldFire = false; }

	// 2. 生产阳光相关
	PlantType getType()const { return m_type; }
	bool shouldSpawnSun() { return m_shouldSpawnSun; }
	void resetSunFlag() { m_shouldSpawnSun = false; }

private:
	vector<IMAGE> m_idleFrames;				//存放晃头动画的所有帧图片
	int m_curFrame = 0;						//当前播放到第几帧
	float m_animTimer = 0.0f;				//动画计时器
	float m_animInterval = 0.15f;			//帧间隔（0.15秒换一张图）
};

