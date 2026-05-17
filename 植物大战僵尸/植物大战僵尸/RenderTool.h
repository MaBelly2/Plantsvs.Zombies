#pragma once
#include <easyx.h>

// 带有 Alpha 透明通道混合的图片绘制函数
inline void putimage_alpha(int dstX, int dstY, IMAGE* img) 
{
    // 获取当前绘制窗口的内存指针
    DWORD* dst = GetImageBuffer(GetWorkingImage());
    // 获取源图片的内存指针
    DWORD* src = GetImageBuffer(img);

    // 获取窗口和图片的宽高
    int dstW = getwidth();
    int dstH = getheight();
    int srcW = img->getwidth();
    int srcH = img->getheight();

    // 遍历图片的每一个像素进行混合计算
    for (int y = 0; y < srcH; y++) {
        for (int x = 0; x < srcW; x++) {
            // 计算在窗口上的实际坐标
            int dx = dstX + x;
            int dy = dstY + y;

            // 边界裁剪：防止图片画出屏幕外导致数组越界崩溃
            if (dx >= 0 && dx < dstW && dy >= 0 && dy < dstH) {
                // 获取图片当前像素点的颜色 (A R G B)
                DWORD sColor = src[y * srcW + x];
                BYTE a = (sColor >> 24) & 0xFF; // 提取 Alpha 透明度

                if (a == 0) continue; // 如果完全透明，跳过不画

                if (a == 255) { // 如果完全不透明，直接覆盖
                    dst[dy * dstW + dx] = sColor;
                    continue;
                }

                // 处理半透明的情况（RGB通道按比例混合）
                DWORD dColor = dst[dy * dstW + dx];
                BYTE sR = (sColor >> 16) & 0xFF;
                BYTE sG = (sColor >> 8) & 0xFF;
                BYTE sB = sColor & 0xFF;

                BYTE dR = (dColor >> 16) & 0xFF;
                BYTE dG = (dColor >> 8) & 0xFF;
                BYTE dB = dColor & 0xFF;

                // 核心混合公式
                dR = dR + (sR - dR) * a / 255;
                dG = dG + (sG - dG) * a / 255;
                dB = dB + (sB - dB) * a / 255;

                // 将计算后的像素写回屏幕缓存
                dst[dy * dstW + dx] = (a << 24) | (dR << 16) | (dG << 8) | dB;
            }
        }
    }
}
