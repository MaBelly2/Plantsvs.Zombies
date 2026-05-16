#pragma once
#include "GameObject.h"

class Sun : public GameObject
{
private:
	float m_fallSpeed;    // 掉落速度
	float m_targetY;      // 掉落的目标高度（掉到哪停下）
	float m_stayTimer;    // 在地上停留的时间计时器
	bool m_isCollected;   // 是否已被收集（用于删除标记）

public:
	static Sun* create(Vec2 pos, int targetY);

	bool init(Vec2 pos, int w, int h) override; // 这里参数w,h在create里固定
	void drawTick() override;
	void eventTick(float delta) override;

	bool isCollected() { return m_isCollected; }
	void collect() { m_isCollected = true; }
	bool isExpired() { return m_stayTimer >= 8.0f; } // 8秒不捡就消失
};

