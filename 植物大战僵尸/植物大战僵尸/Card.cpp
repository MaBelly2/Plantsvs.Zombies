#include "Card.h"
#include <cstdio>

bool Card::init(PlantType type, int x, int y, int w, int h, const char* imgPath)
{
    m_type = type;
    m_x = x;
    m_y = y;
    m_width = w;
    m_height = h;

    m_cost = Plant::getCost(type);
    m_maxCD = Plant::getCardCooldown(type);
    m_currentCD = 0.0f; // 初始开局可以直接种

    // 注意：请确保你的路径后缀是 .png 或 .jpg
    loadimage(&m_img, imgPath, w, h);

    return true;
}

void Card::eventTick(float delta)
{
    // 更新冷却时间
    if (m_currentCD > 0) {
        m_currentCD -= (delta / 1000.0f);
        if (m_currentCD < 0) m_currentCD = 0;
    }
}

void Card::drawTick()
{
    // 1. 画出实体卡片（自带阳光数字）
    putimage_alpha(m_x, m_y, &m_img);

    // 2. 绘制冷却遮罩层 (从下往上收缩的效果)
    if (m_currentCD > 0) {
        float ratio = m_currentCD / m_maxCD;
        int coverHeight = (int)(m_height * ratio);

        setfillcolor(DARKGRAY);
        // 画一个无边框的灰色半透明矩形遮罩
        solidrectangle(m_x, m_y, m_x + m_width, m_y + coverHeight);
    }
}

bool Card::isClicked(int mouseX, int mouseY) const
{
    return (mouseX >= m_x && mouseX <= m_x + m_width &&
        mouseY >= m_y && mouseY <= m_y + m_height);
}

bool Card::isReady(int currentSun) const
{
    return (m_currentCD <= 0 && currentSun >= m_cost);
}

void Card::resetCD()
{
    m_currentCD = m_maxCD;
}