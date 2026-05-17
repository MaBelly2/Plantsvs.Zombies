#include "Lawnmower.h"

Lawnmower* Lawnmower::create(Vec2 pos, int w, int h)
{
    Lawnmower* lm = new Lawnmower();
    if (lm->init(pos, w, h)) return lm;
    delete lm;
    return nullptr;
}

bool Lawnmower::init(Vec2 pos, int w, int h)
{
    m_pos = pos;
    m_width = w;
    m_height = h;

    m_isMoving = false;   // 默认静止
    m_isRemovable = false;
    m_speed = 600.0f;     // 推车冲刺速度（像素/秒），可以调快一点

    // 【A：需要去网上找一个推车的素材，放到 assets 文件夹下】
    loadimage(&m_img, "assets/Lawnmower.png", w, h);

    return true;
}

void Lawnmower::drawTick()
{
    // 向下取整渲染
    putimage_alpha((int)m_pos.x, (int)m_pos.y, &m_img);
}

void Lawnmower::eventTick(float delta)
{
    // 只有被触发了才移动
    if (m_isMoving) {
        float sec = delta / 1000.0f;
        m_pos.x += m_speed * sec; // 向右飞驰

        // 如果跑出了屏幕右边缘 (1280)，标记为可清理
        if (m_pos.x > 1280) {
            m_isRemovable = true;
        }
    }
}

void Lawnmower::trigger()
{
    m_isMoving = true;
}