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

	// ��Ӱ��ͼ���
	Mat4x4f shadow_view_matrix;		// ��Ӱ�۲�任���󣨹�Դ�ӽǣ�
	Mat4x4f shadow_project_matrix;	// ��ӰͶӰ�任���󣨷����Ϊ����ͶӰ���۹��Ϊ͸��ͶӰ��
	Mat4x4f shadow_VP_matrix;		// VP = shadow_project_matrix * shadow_view_matrix

	void CalculateRestMatrix() {
		mvp_matrix = project_matrix * view_matrix * model_matrix;
		// ���ڽ����ߴ�ģ�Ϳռ�任������ռ�,ʹ��ԭʼ�任�������任����
		normal_matrix = matrix_invert(model_matrix).Transpose();
		// ��ӰVP����
		shadow_VP_matrix = shadow_project_matrix * shadow_view_matrix;
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
	uint8_t* color_buffer_;		// ��Ⱦ����ɫ����
	float** shadow_buffer_;		// ��Ⱦ����Ӱ����
	float** depth_buffer_;		// ��Ⱦ����Ȼ���
	float height_, width_;		// ��Ⱦ֡���
	std::vector<Model*> model_list_;	// Ҫ��Ⱦ��ģ���б�
private:
	UniformBuffer* uniform_buffer_;
private:
	static DataBuffer* instance_;
	int current_model_index_;
public:
	DataBuffer();
	~DataBuffer();
	static DataBuffer* GetInstance();
	UniformBuffer* GetUniformBuffer() const;
	void SetUniformBuffer(UniformBuffer* uniform_buffer);
	void Init(int height, int width);
	void CopyShadowBuffer();
	void MoveToNextModel();
	Model* GetModelBeingRendered();
};