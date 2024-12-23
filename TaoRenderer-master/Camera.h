#pragma once

#include "Math.h"
#include "Window.h"
#include "Shader.h"

/*
	�ļ����ݣ�
	-����������
	-���һ���޸����ڣ�2024.11.19
*/

class Camera
{
public:
	Window* window_;					// ����
	DataBuffer* data_buffer_;			// ���ݻ���
	Vec3f position_, origin_position_;	// ���������ռ�λ��
	Vec3f target_, origin_target_;		// ������������ռ�λ��
	Vec3f up_;

	bool is_perspective_;				// �Ƿ�Ϊ͸��ͶӰ
	float scale_factor_;				// ����ϵ��
	// �����up����

	/*
		�������ϵ����
		axis_r��������ָ����Ļ�Ҳ�
		axis_u��������ָ����Ļ�ϲ�
		axis_v��������ָ����Ļ
	*/

	Vec3f axis_r_, axis_u_, axis_v_;

	float fov_;						// ���FOV
	float aspect_;					// �����

	float near_plane_;				// ���ü�ƽ��
	float far_plane_;				// Զ�ü�ƽ��

public:
	Camera(const Vec3f& position, const Vec3f& target, const Vec3f& up, float fov, float aspect); // �����ʼ��������ռ�λ�ã�up������FOV��aspect
	~Camera();
	
	void HandleInputEvents(); // ��Ӧ�������룬��������ƶ�

	// ����uniform buffer�еľ���
	void UpdateUniformBuffer(UniformBuffer* uniform_buffer, const Mat4x4f& model_matrix) const;
	void UpdateSkyBoxUniformBuffer(UniformBuffer* uniform_buffer) const;
	void UpdateSkyboxMesh(SkyBoxShader* sky_box_shader) const;

	// ��ȡ��ǰͶӰ����
	Mat4x4f GetProjectionMatrix() const;

private:
	// ���������̬
	void UpdateCameraPose();		// �������λ��
	void HandleMouseEvents();		// ��Ӧ�������
	void HandleKeyEvents();			// ��Ӧ��������
};