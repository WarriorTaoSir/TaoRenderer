#include <iostream>
#include <cstdio>
#include "Vector.h"
#include "Matrix.h"
#include "Math.h"
#include "Camera.h"
#include "Model.h"
#include "Scene.h"
#include "Shader.h"
#include "DataBuffer.h"
#include "TaoRenderer.h"

/*
	��Ⱦ�������
*/

int main() {
#pragma region ���ڳ�ʼ��
	constexpr int width = 550;
	constexpr int height = 500;

	Window* window = Window::GetInstance();
	window->WindowInit(width, height, "TaoRenderer");
#pragma endregion

#pragma region �ⲿ��Դ����
	const auto scene = new Scene();						// չʾ�ĳ���
	DataBuffer* data_buffer = DataBuffer::GetInstance(); // ��ɫ����������

	auto model = scene->current_model_; // ��ȡ��ǰģ��
	data_buffer->SetModel(scene->current_model_); // �����ݻ������õ�ǰģ��

	window->SetLogMessage("model_message", model->PrintModelInfo());
	window->SetLogMessage("model_name", "model name: " + scene->current_model_->model_name_);
	// window->SetLogMessage("skybox_name", "skybox name: " + scene->current_iblmap_->skybox_name_);
#pragma endregion

#pragma region ����UniformBuffer
	const Vec3f camera_position = { 0, 0, 2 };	// ���λ��
	const Vec3f camera_target = { 0, 0, 0 };	// ��������λ��
	const Vec3f camera_up = { 0, 1, 0 };		// ������ϵ�λ��
	constexpr float fov = 70.0f;				// ��ӵĴ�ֱFOV
	auto* camera = new Camera(camera_position, camera_target, camera_up, fov, static_cast<float>(width) / height);

	const auto uniform_buffer = new UniformBuffer();
	uniform_buffer->model_matrix = model->model_matrix_;
	uniform_buffer->view_matrix = matrix_look_at(camera_position, camera_target, camera_up);
	uniform_buffer->project_matrix = matrix_set_perspective(fov, camera->aspect_, camera->near_plane_, camera->far_plane_);
	

	uniform_buffer->light_direction = { 2, -2, -2 };
	uniform_buffer->light_color = Vec3f(1.0f);
	uniform_buffer->camera_position = camera->position_;

	// ������Ӱ��ؾ���
	uniform_buffer->shadow_view_matrix = matrix_look_at(camera_target - uniform_buffer->light_direction, camera_target, camera_up);
	uniform_buffer->shadow_project_matrix = matrix_set_perspective(fov, camera->aspect_, camera->near_plane_, camera->far_plane_);
	//uniform_buffer->shadow_project_matrix = matrix_set_orthograhpic(width*0.5f, -width * 0.5f, height*0.5f, -height*0.5f, camera->near_plane_, camera->far_plane_);

	uniform_buffer->CalculateRestMatrix();
	data_buffer->SetUniformBuffer(uniform_buffer); // �����ݻ�������Uniform

#pragma endregion

#pragma region ��ʼ����Ⱦ��
	// ��ʼ��shader
	const auto shader = new DefaultShader(uniform_buffer);
	// ��ʼ����Ӱshader
	const auto shadow_shader = new ShadowShader(uniform_buffer);
	// ��ʼ����Ⱦ��
	const auto renderer = new TaoRenderer(width, height);
	// ������Ⱦ״̬
	renderer->SetRenderState(false, true);	// ��Ⱦ�߿��Լ��������
	renderer->render_shadow_ = false;		// ��Ⱦ��Ӱ

#pragma endregion

#pragma region ��Ⱦѭ��

	while (!window->is_close_) {
		// ��Ӧ���λ��
		camera->HandleInputEvents(); 
		if (window->keys_['S']) renderer->render_shadow_ = true;
		if (window->keys_['D']) renderer->render_shadow_ = false;
		// ���֡����
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);

#pragma region ��ȾModel
		// ������Ⱦ����ʹ�õ�shader
		renderer->SetVertexShader(shader->vertex_shader_);
		renderer->SetPixelShader(shader->pixel_shader_);
		renderer->SetShadowVertexShader(shadow_shader->vertex_shader_);

		// ʹ����Ⱦ���������Ϣ���Ƶ�ShadowBuffer��
		renderer->DrawShadowMap();
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);
		// ʹ����Ⱦ����ģ�ͻ��Ƶ�FrameBuffer��
		renderer->DrawMesh();
#pragma endregion 
		// �ڴ���չʾ��Ⱦ����֡����
		window->WindowDisplay(data_buffer->color_buffer_);
	}
#pragma endregion 
	
	return 0;
}