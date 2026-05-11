#pragma once
#include <easyx.h>
#include <cstdio>

class GameObject
{
protected:
	int m_x;		//实体x坐标
	int m_y;		//实体y坐标
	int m_width;	//实体宽度
	int m_height;	//实体高度
	int m_hp;		//血量
	IMAGE m_img;	//实体对应的图片

	//允许子类访问，但外部不能直接构造
	GameObject() = default;

public:
	virtual ~GameObject() = default;
	//统一初始化接口
	virtual bool init(int x, int y, int w, int h) = 0;
	
	//统一帧接口
	virtual void drawTick() = 0;
	virtual void eventTick(float delta) = 0;

	int getX()const { return m_x; }
	int getY()const { return m_y; }
	int getWidth()const { return m_width; }
	int getHeight()const { return m_height; }
	int getHp()const { return m_hp; }
	void setHp(int hp) { m_hp = hp; }
};