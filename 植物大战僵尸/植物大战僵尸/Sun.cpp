#include "Sun.h"

Sun* Sun::create(int x, int y, int targetY)
{
	Sun* s = new Sun();
	s->m_targetY = targetY;
	//【A：根据实际情况调整大小设置】
	if (s->init(x, y, 80, 80)) return s; // 假设阳光大小 80x80
	delete s;
    return nullptr;
}

bool Sun::init(int x, int y, int w, int h)
{
	m_x = x; m_y = y; m_width = w; m_height = h;
	m_fallSpeed = 100.0f; // 每秒掉落100像素
	m_stayTimer = 0;
	m_isCollected = false;
	// 【A：这里记得去加载阳光的图片】
	loadimage(&m_img, "assets/sun.png", w, h);	//路径根据实际路径调整
	return true;
}

void Sun::drawTick()
{
	putimage(m_x, m_y, &m_img);
}

void Sun::eventTick(float delta)
{
	float sec = delta / 1000.0f;
	if (m_y < m_targetY) {
		m_y += m_fallSpeed * sec;	// 没到位置就继续掉
	}
	else {
		m_stayTimer += sec;			// 到位置了开始计时
	}
}
