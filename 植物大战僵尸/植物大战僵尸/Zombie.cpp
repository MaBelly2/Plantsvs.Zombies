#include "Zombie.h"
#include <map>

// 【僵尸属性总表】
// 【C：在这个表里添加相关数据
// 从网上找图片下载到assets文件夹中，重命名，并在这里填写路径，设置移动速度等】
static std::map<ZombieType, ZombieData> g_ZombieConfig = {
	// 类型            血量  移速   攻击力  攻击间隔 图片路径
	{ NORMAL_ZOMBIE, { 100,0.02f,10,2.0f,"assets/Zombies/Normal Zombies" } },	//根据具体路径改一下，这里仅是示例
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

Zombie::~Zombie()
{
	// 1. 清理行走动画帧
	for (auto img : m_walkFrames) {
		delete img;
	}
	m_walkFrames.clear();

	// 2. 清理特殊行走动画帧
	for (auto img : m_walkSpecialFrames) {
		delete img;
	}
	m_walkSpecialFrames.clear();

	// 3. 清理攻击（吃）动画帧
	for (auto img : m_eatFrames) {
		delete img;
	}
	m_eatFrames.clear();

	// 4. 清理特殊攻击动画帧
	for (auto img : m_eatSpecialFrames) {
		delete img;
	}
	m_eatSpecialFrames.clear();

	// 5. 清理死亡动画帧
	for (auto img : m_dieFrames) {
		delete img;
	}
	m_dieFrames.clear();

	// 6. 清理跳跃动画帧
	for (auto img : m_jumpFrames) {
		delete img;
	}
	m_jumpFrames.clear();
}

bool Zombie::init(Vec2 pos, int w, int h)
{
	m_pos = pos;
	m_width = w;
	m_height = h;
	m_moveTimer = 0;
	m_isEating = false;   // 默认在走路
	m_attackTimer = 0;    // 攻击计时器归零

	m_state = WALK;
	m_curFrame = 0;
	m_animTimer = 0.0f;
	m_isRemovable = false;

	// 根据僵尸类型初始化特殊状态
	m_hashelmet = (m_type == FOOTBALL_ZOMBIE);
	m_haspole = (m_type == POLE_VAULTING_ZOMBIE);//撑杆僵尸初始有杆
	m_plantAhead = false;
	m_hasjump = false;

	// 1. 从配置表中查出当前僵尸品种的数据
	ZombieData data = g_ZombieConfig[m_type];

	// 2. 抄写蓝图数据到自己的私有变量身上
	m_hp = data.maxHp;
	m_moveSpeed = data.moveSpeed;

	// 3. 抄写攻击属性
	m_attackDamage = data.attackDamage;
	m_attackInterval = data.attackInterval;
	m_originalSpeed = data.moveSpeed; // 【新增】保存原始速度

	// ====================== 【强制僵尸对齐格子垂直位置】 ======================
	int row, col;
	getGridPosition(row, col);

	// 标准格子参数
	const int GRID_START_Y = 100;
	const int CELL_HEIGHT = 118;
	m_pos.y = GRID_START_Y + row * CELL_HEIGHT + CELL_HEIGHT - m_height;
	//上传所有图片
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
		loadAnimationFrames(m_walkFrames, "assets/Zombies/Normal Zombies/Walking/%d.png", 10);
		loadAnimationFrames(m_eatFrames, "assets/Zombies/Normal Zombies/Attack/%d.png", 10);
		loadAnimationFrames(m_dieFrames, "assets/Zombies/Normal Zombies/Dead/%d.png", 10);
		break;
	case FOOTBALL_ZOMBIE:
		loadAnimationFrames(m_walkFrames, "assets/Zombies/Football Zombies/Walking/%d.png", 10);
		loadAnimationFrames(m_walkSpecialFrames, "assets/Zombies/Football Zombies/Helmetless Walking/%d.png", 10);
		loadAnimationFrames(m_eatFrames, "assets/Zombies/Football Zombies/Attack/%d.png", 10);
		loadAnimationFrames(m_eatSpecialFrames, "assets/Zombies/Football Zombies/Helmetless Attack/%d.png", 10);
		loadAnimationFrames(m_dieFrames, "assets/Zombies/Football Zombies/Dead/%d.png", 8);
		break;
	case POLE_VAULTING_ZOMBIE:
		loadAnimationFrames(m_walkFrames, "assets/Zombies/Pole Vaulting Zombies/Poleless Walking/%d.png", 10);
		loadAnimationFrames(m_walkSpecialFrames, "assets/Zombies/Pole Vaulting Zombies/Walking/%d.png", 10);
		loadAnimationFrames(m_eatFrames, "assets/Zombies/Pole Vaulting Zombies/Attack/%d.png", 10);
		loadAnimationFrames(m_dieFrames, "assets/Zombies/Pole Vaulting Zombies/Dead/%d.png", 10);
		loadAnimationFrames(m_jumpFrames, "assets/Zombies/Pole Vaulting Zombies/Jump/%d.png", 10);
		break;

	}
}

void Zombie::loadAnimationFrames(vector<IMAGE*>& frames, const char* pathFormat, int frameCount) {
	for (int i = 0; i < frameCount; ++i) {
		char path[256];
		sprintf_s(path, pathFormat, i);
		if (m_type == NORMAL_ZOMBIE) {
			IMAGE* img = new IMAGE();
			loadimage(img, path);
			frames.push_back(img); // 直接把图片塞进动画数组
		}
		else {
			IMAGE* img1 = new IMAGE();
			loadimage(img1, path,m_width,m_height);
			frames.push_back(img1);
		}
	}
}
void Zombie::drawTick()
{
	// 根据状态画不同的序列帧

	if (m_state == EAT && !m_eatFrames.empty()) {
		bool useSpecialEat = false;
		if (m_type == FOOTBALL_ZOMBIE && m_hashelmet == false) {
			useSpecialEat = true;
		}
		if (useSpecialEat == true && !m_eatSpecialFrames.empty()) {
			putimage_alpha(m_pos.x, m_pos.y, m_eatSpecialFrames[m_curFrame]);
		}
		else {
			putimage_alpha(m_pos.x, m_pos.y, m_eatFrames[m_curFrame]);
		}
	}
	else if (m_state == DIE && !m_dieFrames.empty()) putimage_alpha(m_pos.x, m_pos.y, m_dieFrames[m_curFrame]);
	else if (m_state == WALK) {
		bool useSpecialWalk = false;
		if (m_type == FOOTBALL_ZOMBIE && m_hashelmet == false) {
			useSpecialWalk = true;
		}
		else if (m_type == POLE_VAULTING_ZOMBIE && m_haspole == true) {
			useSpecialWalk = true;
		}

		if (useSpecialWalk == true && !m_walkSpecialFrames.empty()) {
			putimage_alpha(m_pos.x, m_pos.y, m_walkSpecialFrames[m_curFrame]);
		}
		else if (!m_walkFrames.empty()) {
			putimage_alpha(m_pos.x, m_pos.y, m_walkFrames[m_curFrame]);
		}
	}
	else if (m_state == JUMP) {
		if (!m_jumpFrames.empty()) {
			putimage_alpha(m_pos.x, m_pos.y, m_jumpFrames[m_curFrame]);

		}

	}
}


void Zombie::getGridPosition(int& row, int& col) {
	// ================= 【修复点1】统一采用精确测量的网格刻度 =================
	const int GRID_START_X = 105;
	const int GRID_START_Y = 100;
	const int CELL_WIDTH = 116;
	const int CELL_HEIGHT = 118;

	int centerX = m_pos.x + m_width / 2;
	int centerY = m_pos.y + m_height / 2;

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

	// 减速状态倒计时逻辑
	if (m_slowTimer > 0) {
		m_slowTimer -= sec;
		if (m_slowTimer <= 0) {
			m_moveSpeed = m_originalSpeed; // 时间到，恢复原本速度
		}
	}

	// 1. 状态判定（血量归零切入死亡状态）
	if (m_hp <= 0 && m_state != DIE) {
		setState(DIE);
	}
	else if (m_hp > 0) {
		switch (m_type) {
		case FOOTBALL_ZOMBIE: {
			if (m_hashelmet && m_hp <= 400) {
				m_hashelmet = false;
				m_moveSpeed *= 0.7f;		//m_moveSpeed 实际上是步频的冷却时间，数值越小越快
				m_attackInterval *= 0.8f;
				m_attackDamage *= 1.5f;
			}
			break;
		}
		case POLE_VAULTING_ZOMBIE: {
			// 跳跃条件判断
			if (m_haspole && m_plantAhead && !m_hasjump && m_state != JUMP) {
				setState(JUMP);
				m_plantAhead = false;
				m_hasjump = true;
				m_curFrame = 0;
				m_animTimer = 0.0f; 
			}

			if (m_state == JUMP) {
				m_animTimer += sec;
				if (m_animTimer >= 0.1f) {
					m_animTimer -= 0.1f;
					m_curFrame++;
				}


				if (!m_jumpFrames.empty()&&m_curFrame >= m_jumpFrames.size()) {
					m_pos.x -= 350.0f;
					m_haspole = false;
					m_hasjump = true;
					setState(WALK);
					m_moveSpeed *= 1.5f;
					m_curFrame = 0;
					m_animTimer = 0.0f;
				}
				break;
			}
			if (m_state == WALK) {
				m_moveTimer += sec;
				float speed = m_haspole ? (m_moveSpeed * 1.5f) : m_moveSpeed;
				while (m_moveTimer >= speed) {
					m_moveTimer -= speed;
					m_pos.x -= 1;
				}
			}
			break;
		}
		default:
			break;
		}

		// 吃植物状态切换
		if (m_isEating && m_state != JUMP) setState(EAT);
		else if (!m_isEating && m_state == EAT)
			setState(WALK);

		// 普通僵尸移动更新（死人不能动，所以留在这里面）
		if (m_state == WALK && m_type != POLE_VAULTING_ZOMBIE) {
			m_moveTimer += sec;
			while (m_moveTimer >= m_moveSpeed) {
				m_moveTimer -= m_moveSpeed;
				m_pos.x -= 1;
			}
		}
	}

	// ================= 【修复点1】将动画更新独立到外面 =================
	// 无论僵尸活着还是死了，都要播放对应的动画！
	if (m_state != JUMP) {
		m_animTimer += sec;
		if (m_animTimer >= 0.15f) { // 0.15秒切一帧
			m_animTimer = 0;
			m_curFrame++;

			if (m_state == DIE) {
				// 死亡动画播到最后一帧时，不循环，并标记为可清理
				if (m_dieFrames.size() > 0 && m_curFrame >= m_dieFrames.size()) {
					m_curFrame = m_dieFrames.size() - 1; // 停在倒地那一帧
					m_isRemovable = true;                // 通知 Scene 把他清理掉
				}
			}
			else if (m_state == WALK) {
				if (m_walkFrames.size() > 0 && m_curFrame >= m_walkFrames.size()) m_curFrame = 0;
			}
			else if (m_state == EAT) {
				if (m_eatFrames.size() > 0 && m_curFrame >= m_eatFrames.size()) m_curFrame = 0;
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

void Zombie::applySlow()
{
	m_slowTimer = 5.0f; // 减速持续 5 秒
	m_moveSpeed = m_originalSpeed * 2.0f; // 速度减半
	// 还可以加一句变蓝的代码，如果使用了刚才提过的AlphaBlend，可以在外部加个淡蓝色遮罩，这里暂时改速度
}