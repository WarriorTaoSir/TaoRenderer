#pragma once

#include "Math.h"
#include "Shader.h"

/*
	�ļ����ݣ�
	-ģ���������
	-���һ���޸����ڣ�2024.11.19
*/

class TaoRenderer
{
public:
	TaoRenderer() = default;

	TaoRenderer(const int width, const int height); // ����frame buffer�ĳ�������ʼ����Ⱦ��

	~TaoRenderer() { CleanUp(); }

};