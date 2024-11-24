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
	const auto scene = new Scene();
	DataBuffer* dataBuffer = DataBuffer::GetInstance();

	auto model = scene->current_model_; // ��ȡ��ǰģ��
	dataBuffer->SetModel(scene->current_model_); // �����ݻ������õ�ǰģ��

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
	uniform_buffer->CalculateRestMatrix();

	uniform_buffer->light_direction = { 0, -5, -2 };
	uniform_buffer->light_color = Vec3f(1.0f);
	uniform_buffer->camera_position = camera->position_;

	dataBuffer->SetUniformBuffer(uniform_buffer); // �����ݻ�������Uniform

#pragma endregion

#pragma region ��ʼ����Ⱦ��
	// ��ʼ��shader
	const auto shader = new DefaultShader(uniform_buffer);

	// ��ʼ����Ⱦ��
	const auto renderer = new TaoRenderer(width, height);
	// ������Ⱦ״̬
	renderer->SetRenderState(true, false); // ��Ⱦ�߿򣬵����������

#pragma endregion



	while (!window->is_close_) {

#pragma region ��ȾModel
		// ������Ⱦ����ʹ�õ�shader
		renderer->SetVertexShader(shader->vertex_shader_);

		// ʹ����Ⱦ����ģ�ͻ��Ƶ�֡������
		renderer->DrawMesh();

		// �ڴ���չʾ��Ⱦ����֡����
		window->WindowDisplay(renderer->color_buffer_);


#pragma endregion 
		
	}
	
	return 0;
}