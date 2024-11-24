#pragma once
#include "model.h"

/*
	�ļ����ݣ�
	-���ݻ����������
	-������Renderer��դ����shader������Ҫ�õ�������
	-����UniformBuffer��CurrentModel��
	-���һ���޸����ڣ�2024.11.24
*/

#pragma region ��Ⱦ�������ݽṹ�Ķ���
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

#pragma endregion

class DataBuffer
{
public:
	Attributes* attributes_;
private:
	UniformBuffer* uniform_buffer_;
	Model* model_;

private:
	static DataBuffer* instance_;
public:
	DataBuffer();
	~DataBuffer();
	static DataBuffer* GetInstance();
	UniformBuffer* GetUniformBuffer() const;
	void SetUniformBuffer(UniformBuffer* uniform_buffer);
	Model* GetModel() const;
	void SetModel(Model* uniform_buffer) ;

};