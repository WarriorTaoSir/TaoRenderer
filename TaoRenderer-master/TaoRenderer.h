#pragma once

#include "Math.h"
#include "Shader.h"

/*
	文件内容：
	-模型类的声明
	-最近一次修改日期：2024.11.19
*/

class TaoRenderer
{
public:
	TaoRenderer() = default;

	TaoRenderer(const int width, const int height); // 利用frame buffer的长宽来初始化渲染器

	~TaoRenderer() { CleanUp(); }

};