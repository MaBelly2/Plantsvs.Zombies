#pragma once
#include <easyx.h>
#include <cstdio>
#include <vector>
#include "Vec2.h"
#include "RenderTool.h"

using namespace std;

class GameObject
{
protected:
	Vec2 m_pos;
	int m_width;	//实体宽度
	int m_height;	//实体高度
	int m_hp;		//血量
	IMAGE m_img;	//实体对应的图片

	//允许子类访问，但外部不能直接构造
	GameObject() = default;

public:
	virtual ~GameObject() = default;

	//统一初始化接口
	virtual bool init(Vec2 pos, int w, int h) = 0;
	
	//统一帧接口
	virtual void drawTick() = 0;
	virtual void eventTick(float delta) = 0;

	Vec2 getPos() const { return m_pos; }
	int getX() const { return (int)m_pos.x; }
	int getY() const { return (int)m_pos.y; }

	int getWidth()const { return m_width; }
	int getHeight()const { return m_height; }
	int getHp()const { return m_hp; }

	void setHp(int hp) { m_hp = hp; }
	void setPos(Vec2 pos) { m_pos = pos; } // 设置坐标接口
};