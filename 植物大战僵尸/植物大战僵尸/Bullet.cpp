#include "Bullet.h"
#include <map>

// 【子弹属性总表】
// 【B：在这个表里添加相关数据 
// 从网上找图片下载到assets文件夹中，重命名，并在这里填写路径，设置移动速度等】
static std::map<BulletType, BulletData> g_BulletConfig = {
	// 类型            攻击力  飞行速度(像素/秒)  图片路径
	{ NORMAL_BULLET, { 20,     800.0f,          "assets/normal_bullet.jpg" } },
	{ ICE_BULLET,    { 20,     600.0f,          "assets/ice_bullet.jpg"    } }
};

Bullet* Bullet::create(BulletType type, Vec2 pos, int w, int h)
{
    Bullet* b = new Bullet();
    b->setType(type);
    if (b->init(pos, w, h)) return b;
    delete b;
    return nullptr;
}

bool Bullet::init(Vec2 pos, int w, int h)
{
    m_pos = pos;    // 统一赋值
    m_width = w;
    m_height = h;
    m_speed = 0.1f;
    m_attack = 20;

    // 从配置表中查出当前子弹品种的数据
    BulletData data = g_BulletConfig[m_type];

    m_attack = data.attackDamage;
    m_speed = data.moveSpeed;

	//加载图片
	if (!loadimage(&m_img, data.imgPath, w, h))
	{
		printf("图片加载失败：%s\n", data.imgPath);
		return false;
	}

    return true;
}

void Bullet::drawTick()
{
	putimage((int)m_pos.x, (int)m_pos.y, &m_img);
}

void Bullet::eventTick(float delta)
{
	float sec = delta / 1000.0f;
    m_pos.x += m_speed * sec; // 向右飞
}

bool Bullet::isOutOfScreen()
{
    return m_pos.x > 1280;
}