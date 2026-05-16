#include "Zombie.h"
#include <map>

// 【僵尸属性总表】
// 【C：在这个表里添加相关数据
// 从网上找图片下载到assets文件夹中，重命名，并在这里填写路径，设置移动速度等】
static std::map<ZombieType, ZombieData> g_ZombieConfig = {
	// 类型            血量  移速   攻击力  攻击间隔 图片路径
	{ NORMAL_ZOMBIE, { 100,  0.08f,  10,     1.0f,    "assets/normal_zombie.png" } },	//根据具体路径改一下，这里仅是示例
	{ NEWSPAPER_ZOMBIE, { 250,  0.08f,  10,     1.0f,    "assets/newspaper_zombie.png"   } },
	{ DANCING_ZOMBIE,{ 120,  0.07f,  10,     1.0f,    "assets/dancing_zombie.png"   } },
	{ BACKUP_DANCER,{ 80,  0.09f,  10,  1.0f,  "assets/backup_dancer.png"}}
};

Zombie* Zombie::create(ZombieType type, Vec2 pos, int w, int h)
{
    Zombie* z = new Zombie();
    z->setType(type);
    if (z->init(pos, w, h)) return z;

    delete z;
    return nullptr;
}

bool Zombie::init(Vec2 pos, int w, int h)
{
	m_pos = pos;
	m_width = w;
	m_height = h;
	m_moveTimer = 0;
	m_isEating = false;   // 默认在走路
	m_attackTimer = 0;    // 攻击计时器归零
	m_hasNewspaper = true;
	m_newspaperHp=100;      //报纸血量
	m_danceTimer=0;       //跳舞召唤计时器
	isdancing=false;         //是否正在召唤

	// 1. 从配置表中查出当前僵尸品种的数据
	ZombieData data = g_ZombieConfig[m_type];

	// 2. 抄写蓝图数据到自己的私有变量身上
	m_hp = data.maxHp;
	m_moveSpeed = data.moveSpeed;

	// 3. 抄写攻击属性
	m_attackDamage = data.attackDamage;
	m_attackInterval = data.attackInterval;

	// 4. 自动加载对应的图片
	if (!loadimage(&m_img, data.imgPath, w, h))
	{
		printf("图片加载失败：%s\n", data.imgPath);
		return false;
	}

	return true;
}

void Zombie::drawTick()
{
	// 根据状态画不同的序列帧
	if (m_state == WALK && !m_walkFrames.empty()) putimage(m_pos.x, m_pos.y, &m_walkFrames[m_curFrame]);
	else if (m_state == EAT && !m_eatFrames.empty()) putimage(m_pos.x, m_pos.y, &m_eatFrames[m_curFrame]);
	else if (m_state == DIE && !m_dieFrames.empty()) putimage(m_pos.x, m_pos.y, &m_dieFrames[m_curFrame]);
}

void Zombie::eventTick(float delta)
{
	// 如果标记为可移除，就不更新了
	if (m_isRemovable) return;

	float sec = delta / 1000.0f;  //将毫秒转换为秒

	// 1. 状态判定（血量归零切入死亡状态）
	if (m_hp <= 0 && m_state != DIE) {
		setState(DIE);
	}
	else if (m_hp > 0) {
		// 根据是否吃到植物切换状态
		if (m_isEating) setState(EAT);
		else setState(WALK);
	}

	// 2. 动画帧更新
	m_animTimer += sec;
	if (m_animTimer >= 0.15f) { // 假设0.15秒切一帧
		m_animTimer = 0;
		m_curFrame++;

		// 死亡动画播到最后一帧时，不循环，标记为可清理
		if (m_state == DIE && m_curFrame >= m_dieFrames.size()) {
			m_curFrame = m_dieFrames.size() - 1; // 停在最后一帧（倒地）
			m_isRemovable = true;                // 允许 Scene 把他清理掉
		}
		// 走路和啃食动画则循环
		else if (m_state == WALK && m_curFrame >= m_walkFrames.size()) m_curFrame = 0;
		else if (m_state == EAT && m_curFrame >= m_eatFrames.size()) m_curFrame = 0;
	}

	// 3. 只有走路状态才位移
	if (m_state == WALK) {
		m_moveTimer += sec;
		if (m_moveTimer >= m_moveSpeed) {
			m_moveTimer = 0;
			m_pos.x -= 1;
		}
	}
	
	switch (m_type) {
	case NEWSPAPER_ZOMBIE:
		if (m_hasNewspaper && m_newspaperHp > 0) {
			m_newspaperHp -= getDamage(delta);
		}
		else if (m_hasNewspaper && m_newspaperHp <= 0) {
			m_hasNewspaper = false;
			m_moveSpeed = 0.15f;
		}
			



	}



	if (m_moveTimer >= m_moveSpeed) {
		m_moveTimer = 0;
		m_pos.x -= 1;	// 僵尸向左走
	}
}

void Zombie::setType(ZombieType type)
{
	m_type = type;
}

int Zombie::getDamage(float delta)
{
	float sec = delta / 1000.0f;
	m_attackTimer += sec;

	// 如果冷却时间到了，就返回攻击力，并重置计时器
	if (m_attackTimer >= m_attackInterval) {
		m_attackTimer = 0;
		return m_attackDamage;
	}

	return 0;
}

void Zombie::setState(ZombieState state)
{
	if (m_state == state) return;	// 状态没变就不管
	m_state = state;
	m_curFrame = 0;					// 切换状态时，动画从第0帧开始播
	m_animTimer = 0;
}
