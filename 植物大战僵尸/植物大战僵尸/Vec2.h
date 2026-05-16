#pragma once

class Vec2
{
public:
	float x, y; // 设为 public，方便直接访问和运算

	Vec2() : x(0.0f), y(0.0f) {}
	Vec2(float x, float y) : x(x), y(y) {}

	// 重载加减乘法运算符，方便后续做移动逻辑
	Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
	Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
	Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); } // 向量乘标量
	void operator+=(const Vec2& v) { x += v.x; y += v.y; }
};

