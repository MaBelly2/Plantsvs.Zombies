#pragma once
#include "GameObject.h"

// 子弹类型枚举
//B：根据想要添加的子弹名称，在这里丰富枚举类型
enum BulletType {
    NORMAL_BULLET,	//普通子弹
    ICE_BULLET		//冰弹
};

// 子弹数据配置表
struct BulletData {
    int attackDamage;   // 攻击力
    float moveSpeed;    // 飞行速度（像素/秒）
    const char* imgPath;// 图片路径
};

class Bullet: public GameObject
{
    Bullet() = default;
    Bullet(const Bullet&) = default;
    Bullet& operator=(const Bullet&) = default;

private:
    float m_speed;
    int m_attack;
    BulletType m_type;
    bool m_isActive = true; //默认存活

public:
    static Bullet* create(BulletType type, Vec2 pos, int w, int h);

    bool init(Vec2 pos, int w, int h) override;
    void drawTick() override;
    void eventTick(float delta) override;

    void setType(BulletType type) { m_type = type; } // 设置类型
    bool isOutOfScreen();
    int getAttack() { return m_attack; }

    bool isActive()const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

    BulletType getType() { return m_type; }
};

