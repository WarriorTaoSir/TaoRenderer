#include "Camera.h"

/*
	�ļ����ݣ�
	-�����Ķ���
	-���һ���޸����ڣ�2024.11.19
*/

Camera::Camera(const Vec3f& position, const Vec3f& target, const Vec3f& up, float fov, float aspect) :
	position_(position), target_(target), up_(up), fov_(fov), aspect_(aspect)
{	
	// ���Զ��ƽ�����ã����Ա�¶Ϊ����
	near_plane_ = 0.4f; 
	far_plane_ = 1000.0f;

	// ����ԭλ���볯�������������λ��
	origin_position_ = position_;
	origin_target_ = target_;

	// ���ڳ�ʼ��
	window_ = Window::GetInstance();
}

void Camera::UpdateCameraPose()
{
	Vec3f view = position_ - target_;		// �۲������� �����λ��ָ��Ŀ��λ��
	float radius = vector_length(view);		// �۲쳤��/�뾶

	float phi = atan2(view[0], view[2]);	// azimuth angle ��λ��
	float theta = acos(view[1] / radius);	// zenith  angle �춥��
	
	const float mouse_delta_x = window_->mouse_info_.mouse_delta[0] / window_->width_;
	const float mouse_delta_y = window_->mouse_info_.mouse_delta[1] / window_->height_;

	// ��������������ӽ�
	if (window_->mouse_buttons_[0])
	{
		constexpr float factor = 1.5 * kPi;

		phi += mouse_delta_x * factor;
		theta += mouse_delta_y * factor;
		if (theta > kPi) theta = kPi - kEpsilon * 100;
		if (theta < 0)  theta = kEpsilon * 100;
	}

	// ����Ҽ��������ƽ��
	if (window_->mouse_buttons_[1])
	{
		const float factor = radius * static_cast<float>(tan(60.0 / 360 * kPi)) * 2.2f;
		const Vec3f right = mouse_delta_x * factor * axis_r_;
		const Vec3f up = mouse_delta_y * factor * axis_u_;

		position_ += (-right + up);
		target_ += (-right + up);
	}

	// �����ֿ������ţ�Ҳ���ǿ���radius�Ĵ�С
	if (window_->mouse_buttons_[2])
	{
		radius *= static_cast<float>(pow(0.95, window_->mouse_info_.mouse_wheel_delta));
		window_->mouse_buttons_[2] = 0;
	}
	
	// �Թ̶�ģ��Ϊ��׼���������λ��
	position_[0] = target_[0] + radius * sin(phi) * sin(theta);
	position_[1] = target_[1] + radius * cos(theta);
	position_[2] = target_[2] + radius * sin(theta) * cos(phi);
}

// �����������̵������¼�
void Camera::HandleInputEvents()
{
	/*
		�����������ϵ����
		axis_v���۲���������Ŀ��λ��ָ�����λ��
		axis_r��������ָ����Ļ�Ҳ�
		axis_u��������ָ����Ļ�ϲ�
	*/
	axis_v_ = vector_normalize(target_ - position_);
	axis_r_ = vector_normalize(vector_cross(axis_v_, up_));
	axis_u_ = vector_normalize(vector_cross(axis_r_, axis_v_));


	// ���������¼�
	HandleMouseEvents();
	HandleKeyEvents();
}

// ��������¼�
void Camera::HandleMouseEvents()
{	
	
	if (window_->mouse_buttons_[0] || window_->mouse_buttons_[1] || window_->mouse_buttons_[2])
	{
		const Vec2f mouse_position = window_->GetMousePosition();
		window_->mouse_info_.mouse_delta = window_->mouse_info_.mouse_position - mouse_position;
		window_->mouse_info_.mouse_position = mouse_position;

		UpdateCameraPose();
	}
}

// ������������¼�
void Camera::HandleKeyEvents()
{	
	
	const float distance = vector_length(target_ - position_);

	if (window_->keys_['Q'])
	{
		const float factor = distance / window_->width_ * 200.0f;
		position_ += -0.05f * axis_v_ * factor;
	}
	if (window_->keys_['E'])
	{
		position_ += 0.05f * axis_v_;
	}
	if (window_->keys_['W'])
	{
		position_ += 0.05f * axis_u_;
		target_ += 0.05f * axis_u_;
	}
	if (window_->keys_['S'])
	{
		position_ += -0.05f * axis_u_;
		target_ += -0.05f * axis_u_;
	}
	if (window_->keys_['A'])
	{
		position_ += -0.05f * axis_r_;
		target_ += -0.05f * axis_r_;
	}
	if (window_->keys_['D'])
	{
		position_ += 0.05f * axis_r_;
		target_ += 0.05f * axis_r_;
	}
	if (window_->keys_[VK_SPACE])
	{
		position_ = origin_position_;
		target_ = origin_target_;
	}
	if (window_->keys_[VK_ESCAPE])
	{
		window_->is_close_ = true;
	}
}

// ��������MVP��������еĲ������Լ����VP���󣬶�M�����ɲ�������
void Camera::UpdateUniformBuffer(UniformBuffer* uniform_buffer, const Mat4x4f& model_matrix) const
{
	uniform_buffer->model_matrix = model_matrix;
	uniform_buffer->view_matrix = matrix_look_at(position_, target_, up_);
	uniform_buffer->project_matrix = matrix_set_perspective(fov_, aspect_, near_plane_, far_plane_);;

	uniform_buffer->CalculateRestMatrix();

	uniform_buffer->camera_position = position_;
}

void Camera::UpdateSkyBoxUniformBuffer(UniformBuffer* uniform_buffer) const
{
	// TODO
}

//void Camera::UpdateSkyboxMesh(SkyBoxShader* sky_box_shader) const
//{
//	// TODO
//}
