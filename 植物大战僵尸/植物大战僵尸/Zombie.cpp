#include "Zombie.h"
#include <map>

// 【僵尸属性总表】
// 【C：在这个表里添加相关数据
// 从网上找图片下载到assets文件夹中，重命名，并在这里填写路径，设置移动速度等】
static std::map<ZombieType, ZombieData> g_ZombieConfig = {
	// 类型            血量  移速   攻击力  攻击间隔 图片路径
	{ NORMAL_ZOMBIE, { 100,0.02f,10,2.0f,"assets/Zombies/Normal Zombies" } },	//���ݾ���·����һ�£��������ʾ��
	{ FOOTBALL_ZOMBIE, { 800,0.04f,10,2.0f,"assets/Zombies/Football Zombies" } },
	{ POLE_VAULTING_ZOMBIE, { 100,0.08f,10,2.0f,"assets/Zombies/Pole Vaulting Zombies" } },
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

	// 1. 从配置表中查出当前僵尸品种的数据
	ZombieData data = g_ZombieConfig[m_type];

	// 2. 抄写蓝图数据到自己的私有变量身上
	m_hp = data.maxHp;
	m_moveSpeed = data.moveSpeed;

	// 3. 抄写攻击属性
	m_attackDamage = data.attackDamage;
	m_attackInterval = data.attackInterval;

	//// 4. 自动加载对应的图片
	//if (!loadimage(&m_img, data.imgPath, w, h))
	//{
	//	printf("图片加载失败：%s\n", data.imgPath);
	//	return false;
	//}
	loadAllAnimation();
	return true;
}

void Zombie::loadAllAnimation() {
	m_walkFrames.clear();
	m_walkSpecialFrames.clear();
	m_eatFrames.clear();
	m_eatSpecialFrames.clear();
	m_jumpFrames.clear();
	switch (m_type) {
	case NORMAL_ZOMBIE:
		loadAnimationFrames(m_walkFrames, "assets/Zombies/Normal Zombies/Walking/%d.gif", 10);
		loadAnimationFrames(m_eatFrames, "assets/Zombies/Normal Zombies/Attack/%d.gif", 10);
		loadAnimationFrames(m_dieFrames, "assets/Zombies/Normal Zombies/Dead/%d.gif", 10);
		break;
	case FOOTBALL_ZOMBIE:
		loadAnimationFrames(m_walkFrames, "assets/Zombies/Football Zombies/Walking/%d.gif", 10);
		loadAnimationFrames(m_walkSpecialFrames, "assets/Zombies/Football Zombies/Helmetless Walking/%d.gif", 10);
		loadAnimationFrames(m_eatFrames, "assets/Zombies/Football Zombies/Attack/%d.gif", 10);
		loadAnimationFrames(m_eatSpecialFrames, "assets/Zombies/Football Zombies/Helmetless Attack/%d.gif", 10);
		loadAnimationFrames(m_dieFrames, "assets/Zombies/Football Zombies/Dead/%d.gif", 8);
		break;
	case POLE_VAULTING_ZOMBIE:
		loadAnimationFrames(m_walkFrames, "assets/Zombies/Pole Vaulting Zombies/Poleless Walking/%d.gif", 10);
		loadAnimationFrames(m_walkSpecialFrames, "assets/Zombies/Pole Vaulting Zombies/Walking/%d.gif", 10);
		loadAnimationFrames(m_eatFrames, "assets/Zombies/Pole Vaulting Zombies/Attack/%d.gif", 10);
		loadAnimationFrames(m_dieFrames, "assets/Zombies/Pole Vaulting Zombies/Dead/%d.gif", 10);
		loadAnimationFrames(m_jumpFrames, "assets/Zombies/Pole Vaulting Zombies/Jump/%d.gif", 10);
		break;

	}
}
void Zombie::loadAnimationFrames(vector<IMAGE>& frames, const char* pathFormat, int frameCount) {
	for (int i = 0; i < frameCount; ++i) {
		char path[256];
		sprintf_s(path, pathFormat, i);
		IMAGE img;
		if (loadimage(&img, path, m_width, m_height)) {
			frames.push_back(img);
		}
		else {
			printf("图片加载失败：%s\n", path);
		}
	}
}

void Zombie::drawTick()
{
	// ����״̬����ͬ������֡

	if (m_state == EAT && !m_eatFrames.empty()) {
		bool useSpecialEat = false;
		if (m_type == FOOTBALL_ZOMBIE && m_hashelmet == false) {
			useSpecialEat = true;
		}
		else {
			useSpecialEat = false;
		}
		if (useSpecialEat == true && !m_eatSpecialFrames.empty()) {
			putimage(m_x, m_y, &m_eatSpecialFrames[m_curFrame]);
		}
		else if (!m_eatFrames.empty()) {
			putimage(m_x, m_y, &m_eatFrames[m_curFrame]);
		}

	}
	else if (m_state == DIE && !m_dieFrames.empty()) putimage(m_x, m_y, &m_dieFrames[m_curFrame]);
	else if (m_state == WALK) {
		bool useSpecialWalk = false;
		if (m_type == FOOTBALL_ZOMBIE && m_hashelmet == false) {
			useSpecialWalk = true;
		}
		else if (m_type == POLE_VAULTING_ZOMBIE && m_haspole == true) {
			useSpecialWalk = true;
		}
		else if (m_type == NORMAL_ZOMBIE) {
			useSpecialWalk = false;
		}
		if (useSpecialWalk == true && !m_walkSpecialFrames.empty()) {
			putimage(m_x, m_y, &m_walkSpecialFrames[m_curFrame]);
		}
		else if (!m_walkFrames.empty()) {
			putimage(m_x, m_y, &m_walkFrames[m_curFrame]);
		}
	}
	else if (m_state == JUMP) {
		if (!m_jumpFrames.empty()) {
			putimage(m_x, m_y, &m_jumpFrames[m_curFrame]);

		}

	}
}
void Zombie::getGridPosition(int& row, int& col) {
	const int GRID_START_X = 250;
	const int GRID_START_Y = 80;
	const int CELL_WIDTH = 80;
	const int CELL_HEIGHT = 100;

	int centerX = m_x + m_width / 2;
	int centerY = m_y + m_height / 2;

	col = (centerX - GRID_START_X) / CELL_WIDTH;
	row = (centerY - GRID_START_Y) / CELL_HEIGHT;

	if (row < 0) row = 0;
	else if (row >= 5) row = 4;

	if (col < 0) col = 0;
	else if (col >= 9) col = 8;
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
		switch (m_type) {
		case FOOTBALL_ZOMBIE: {
			if (m_hashelmet && m_hp <= 400) {
				m_hashelmet = false;
				m_moveSpeed *= 1.5f;
				m_attackInterval *= 0.8f;
				m_attackDamage *= 1.5f;
			}
			break;
		}
		case POLE_VAULTING_ZOMBIE: {

			if (m_haspole && m_plantAhead && !m_hasjump && m_state != JUMP) {
				setState(JUMP);
				m_plantAhead = false;
				m_hasjump = true;
				m_curFrame = 0;
				m_animTimer = 0;

			}
			if (m_state == JUMP) {
				if (m_animTimer >= 0.1f) {
					m_animTimer = 0;
					m_curFrame++;
				}
				if (m_curFrame >= m_jumpFrames.size()) {
					m_haspole = false;
					setState(WALK);
					m_moveSpeed *= 0.7f;
				}
				m_moveTimer += sec;
				if (m_moveTimer >= m_moveSpeed * 3) {

					m_moveTimer = 0;
					m_x -= 3;
				}
			}
			else if (m_state == WALK) {

				m_moveTimer += sec;
				float speed;
				if (m_haspole) {
					speed = m_moveSpeed * 1.5f;
				}
				else {
					speed = m_moveSpeed;
				}
				if (m_moveTimer >= speed) {
					m_moveTimer = 0;
					m_x -= 1;
				}
			}


			break;
		}

		default:
			break;

		}
		if (m_isEating && m_state != JUMP) setState(EAT);
		else if (!m_isEating && m_state == EAT)
			setState(WALK);


		// 2. ����֡����
		m_animTimer += sec;
		if (m_animTimer >= 0.15f) { // ����0.15����һ֡
			m_animTimer = 0;
			m_curFrame++;
			if (m_state == DIE && m_curFrame >= m_dieFrames.size()) {
				m_curFrame = m_dieFrames.size() - 1; // ͣ�����һ֡�����أ�
				m_isRemovable = true;                // ���� Scene ����������
			}
			// ��·�Ϳ�ʳ������ѭ��
			else if (m_state == WALK && m_curFrame >= m_walkFrames.size()) m_curFrame = 0;
			else if (m_state == EAT && m_curFrame >= m_eatFrames.size()) m_curFrame = 0;

			else if (m_state == JUMP && m_curFrame >= m_jumpFrames.size()) {
				m_curFrame = m_jumpFrames.size() - 1;
			}
		}
		if (m_state == WALK && m_type != POLE_VAULTING_ZOMBIE) {
			m_moveTimer += sec;
			if (m_moveTimer >= m_moveSpeed) {
				m_moveTimer = 0;
				m_x -= 1;
			}
		}
	}
}
void Zombie::setType(ZombieType type){
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