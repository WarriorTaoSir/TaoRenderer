#pragma once

#include "Math.h"

/*
	�ļ����ݣ�
	-Shader�������
	-���һ���޸����ڣ�2024.11.19
*/

struct UniformBuffer
{
	Mat4x4f model_matrix;		// ģ�ͱ任����
	Mat4x4f view_matrix;		// �۲�任����
	Mat4x4f project_matrix;		// ͶӰ�任����
	Mat4x4f mvp_matrix;			// MVP�任����

	Mat4x4f normal_matrix;		// ���߱任����

	void CalculateRestMatrix() {
		mvp_matrix = project_matrix * view_matrix * model_matrix;
		// ���ڽ����ߴ�ģ�Ϳռ�任������ռ�,ʹ��ԭʼ�任�������任����
		normal_matrix = matrix_invert(model_matrix).Transpose();
	}

	// ��������
	Vec3f light_direction;		// ���շ�������ɫ��ָ���Դ��
	Vec3f light_color;			// ������ɫ
	Vec3f camera_position;		// �������
};