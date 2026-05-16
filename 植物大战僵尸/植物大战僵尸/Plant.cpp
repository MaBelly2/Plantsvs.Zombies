#include "Plant.h"
#include <map>

// 【植物属性总表】
// 【B：在这个表里添加相关数据 
// 从网上找图片下载到assets文件夹中，重命名，并在这里填写路径，设置移动速度等】
static std::map<PlantType, PlantData> g_PlantConfig = {
	{ PEASHOOTER, { 100, 100, 1.5f, 7.5f,"assets/PlantAnimation/Peashooter/Idle/%d.png",10 } },	//根据具体路径改一下，这里仅是示例
	{ SUNFLOWER,  { 50,  100, 5.0f, 7.5f,"assets/PlantAnimation/Sunflower/Idle/%d.png", 10 } }
};

Plant* Plant::create(PlantType type, Vec2 pos, int w, int h)
{
	Plant* plant = new Plant();
	plant->setType(type);

	if (plant->init(pos, w, h)) return plant;

	delete plant;
	return nullptr;
}

bool Plant::init(Vec2 pos, int w, int h)
{
	m_pos = pos;
	m_width = w;
	m_height = h;
	m_hp = 100;
	m_cdTimer = 0;

	// 1. 从配置表中查出当前植物的数据
	PlantData data = g_PlantConfig[m_type];

	m_hp = data.maxHp;         // 自动读取血量
	m_cdTimer = data.cooldown; // 自动读取冷却

	// 2. 循环加载序列帧图片
	for (int i = 0; i < data.frameCount; ++i) {
		char path[128];
		sprintf_s(path, data.imgPathFormat, i); // 把 %d 替换成数字 0, 1, 2...

		IMAGE img;
		if (!loadimage(&img, path, w, h)) {
			printf("图片加载失败：%s\n", path);
		}
		m_idleFrames.push_back(img);			//末尾追加一个元素
	}

	return true;
}

void Plant::drawTick()
{
	// 画出当前帧的图片
	if (!m_idleFrames.empty()) {
		putimage(m_pos.x, m_pos.y, &m_idleFrames[m_curFrame]);
	}
}

void Plant::eventTick(float delta)
{
	float sec = delta / 1000.0f;

	// 3. 动画播放逻辑
	m_animTimer += sec;
	if (m_animTimer >= m_animInterval) {
		m_animTimer = 0;
		m_curFrame++;
		if (m_curFrame >= m_idleFrames.size()) {
			m_curFrame = 0; // 播到最后一张，回到第一张循环
		}
	}

	if (m_cdTimer > 0) {
		m_cdTimer -= sec;
		return;
	}

	// 从配置表中查出当前植物的数据
	PlantData data = g_PlantConfig[m_type];

	switch (m_type){
	case PEASHOOTER:
		//发射豌豆
		m_shouldFire = true;			// 举起“我想开火”的旗子	
		m_cdTimer = data.cooldown;		//根据品种。重置冷却计时器
		break;
	case SUNFLOWER:
		m_cdTimer = data.cooldown;
		m_shouldSpawnSun = true;		// 举旗子
		break;
	default:break;
	}
}

void Plant::setType(PlantType type)
{
	m_type = type;
}

int Plant::getCost(PlantType type)
{
	return g_PlantConfig[type].cost;
}

float Plant::getCardCooldown(PlantType type)
{
	return g_PlantConfig[type].cardCooldown;
}
