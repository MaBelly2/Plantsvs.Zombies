#pragma once
#include <easyx.h>
#include "Plant.h"

class Card
{
private:
    PlantType m_type;       // 植物类型
    int m_x, m_y;           // 卡片在屏幕左上角的坐标
    int m_width, m_height;  // 卡片的宽高
    int m_cost;             // 种植所需的阳光（仅用于逻辑判定）

    IMAGE m_img;            // 原色卡片图片

    float m_maxCD;          // 最大冷却时间
    float m_currentCD;      // 当前剩余冷却时间

public:
    Card() = default;
    ~Card() = default;

    // 初始化卡片
    bool init(PlantType type, int x, int y, int w, int h, const char* imgPath);

    // 更新CD计时
    void eventTick(float delta);

    // 渲染卡片（无需再传入阳光数值）
    void drawTick();

    // 检查鼠标是否点中了卡片
    bool isClicked(int mouseX, int mouseY) const;

    PlantType getType() const { return m_type; }

    // 检查卡片是否就绪（CD完毕 且 阳光足够）
    bool isReady(int currentSun) const;

    // 重置CD
    void resetCD();
};