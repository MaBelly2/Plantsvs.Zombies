#pragma once
#include "GameObject.h"

class Lawnmower : public GameObject
{
private:
    bool m_isMoving;     // 是否被触发正在移动
    float m_speed;       // 冲刺速度
    bool m_isRemovable;  // 是否已经冲出屏幕外，等待被清理

public:
    static Lawnmower* create(Vec2 pos, int w, int h);

    bool init(Vec2 pos, int w, int h) override;
    void drawTick() override;
    void eventTick(float delta) override;

    // 推车特有功能
    void trigger();                         // 触发推车
    bool isMoving() const { return m_isMoving; }
    bool isRemovable() const { return m_isRemovable; }
};

