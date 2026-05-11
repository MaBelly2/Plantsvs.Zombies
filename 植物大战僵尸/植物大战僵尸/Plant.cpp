#include "Plant.h"
#include <map>

// 【植物属性总表】
// 【B：在这个表里添加相关数据 
// 从网上找图片下载到assets文件夹中，重命名，并在这里填写路径，设置移动速度等】
static std::map<PlantType, PlantData> g_PlantConfig = {
	{ PEASHOOTER, { 100, 100, 1.5f, 7.5f,"assets/peashooter.jpg" } },	//根据具体路径改一下，这里仅是示例
	{ SUNFLOWER,  { 50,  100, 5.0f, 7.5f,"assets/sunflower.jpg"  } }
};

Plant* Plant::create(PlantType type, int x, int y, int w, int h)
{
	Plant* plant = new Plant();
	plant->setType(type);

	if (plant->init(x, y, w, h)) return plant;

	delete plant;
	return nullptr;
}

bool Plant::init(int x, int y, int w, int h)
{
	m_x = x;
	m_y = y;
	m_width = w;
	m_height = h;
	m_hp = 100;
	m_cdTimer = 0;

	// 从配置表中查出当前植物的数据
	PlantData data = g_PlantConfig[m_type];

	m_hp = data.maxHp;         // 自动读取血量
	m_cdTimer = data.cooldown; // 自动读取冷却

	//加载图片
	if (!loadimage(&m_img, data.imgPath, w, h))
	{
		printf("图片加载失败：%s\n", data.imgPath);
		return false;
	}

	return true;
}

void Plant::drawTick()
{
	putimage(m_x, m_y, &m_img);
}

void Plant::eventTick(float delta)
{
	float sec = delta / 1000.0f;
	if (m_cdTimer > 0) {
		m_cdTimer -= sec;
		return;
	}

	// 从配置表中查出当前植物的数据
	PlantData data = g_PlantConfig[m_type];

	switch (m_type){
	case PEASHOOTER:
		//发射豌豆
		m_shouldFire = true;// 举起“我想开火”的旗子	
		m_cdTimer = data.cooldown;	//根据品种。重置冷却计时器
		break;
	case SUNFLOWER:
		m_cdTimer = data.cooldown;
		m_shouldSpawnSun = true; // 举旗子
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
