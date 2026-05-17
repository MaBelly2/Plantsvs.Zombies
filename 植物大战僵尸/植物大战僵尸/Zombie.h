#pragma once
#include "GameObject.h"

//僵尸类型枚举
//C：根据想要添加的僵尸名称，在这里丰富枚举类型
enum ZombieType {
	NORMAL_ZOMBIE,		//普通僵尸
	FOOTBALL_ZOMBIE,    //橄榄球僵尸
	POLE_VAULTING_ZOMBIE //撑杆僵尸
};

//僵尸状态枚举
enum ZombieState {
	WALK,
	EAT,
	DIE,
	JUMP
};

// 定义一个僵尸数据的结构体
struct ZombieData {
	int maxHp;				// 最大血量
	float moveSpeed;		// 移动速度（或者叫移动间隔）
	int attackDamage;		// 攻击力（每次啃咬扣除植物多少血，提前加上备用）
	float attackInterval;	// 啃咬的间隔时间（比如 1.0f 代表 1秒咬一次）
	const char* imgPath;	// 图片路径
};

class Zombie:public GameObject
{
	Zombie() = default;
	Zombie(const Zombie&) = default;
	Zombie& operator=(const Zombie&) = default;

private:
	ZombieState m_state = WALK;		// 默认走路
	bool m_isRemovable = false;		// 死亡动画是否播完？播完才能从内存中清理
	bool m_hashelmet = true;
	bool m_haspole = true;
	bool m_plantAhead = false;
	bool m_hasjump = false;
	//动画相关
	vector<IMAGE> m_walkFrames;
	vector<IMAGE> m_eatFrames;
	vector<IMAGE> m_dieFrames;
	vector<IMAGE> m_walkSpecialFrames;
	vector<IMAGE> m_eatSpecialFrames;
	vector<IMAGE> m_jumpFrames;

	int m_curFrame = 0;
	float m_animTimer = 0.0f;

private:
	ZombieType m_type;
	float m_moveSpeed;	//移动速度
	float m_moveTimer;	//移动计时器

	bool m_isEating;		// 是否正在啃食植物
	int m_attackDamage;		// 攻击力
	float m_attackTimer;	// 攻击计时器
	float m_attackInterval; // 攻击间隔

	float m_slowTimer = 0.0f;	// 减速持续时间计时器
	float m_originalSpeed;		// 记录僵尸原本的移动速度

public:
	ZombieType getType() {
		return m_type;
	}
	void loadAllAnimation();
	void loadAnimationFrames(std::vector<IMAGE>& frames, const char* path, int frameCount);
	static Zombie* create(ZombieType type, Vec2 pos, int w, int h);

	bool init(Vec2 pos, int w, int h) override;	//override 检查是否在重写一个虚函数
	
	void setPlantAhead(bool ahead) {
		m_plantAhead = ahead;
	}
	void getGridPosition(int& row, int& col);
	void drawTick()override;
	void eventTick(float delta)override;

	//僵尸自己的功能
	void setType(ZombieType type);
	//是否死亡
	bool isDead() { return m_hp <= 0; }

	void setEating(bool eating) { m_isEating = eating; }
	int getDamage(float delta); // 计算并返回当前帧应该造成的伤害

	void setState(ZombieState state);
	bool isRemovable() { return m_isRemovable; }

	ZombieState getState() { return m_state; }

	void applySlow();			// 受到冰弹攻击时调用的减速方法
};

