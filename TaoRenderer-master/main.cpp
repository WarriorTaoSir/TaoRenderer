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
	
	for (int i = 0; i < 2; i++) {
		data_buffer->model_list_.push_back(scene->current_model_);
		scene->LoadNextModel();
	}

	window->SetLogMessage("model_message", model->PrintModelInfo());
	window->SetLogMessage("model_name", "model name: " + scene->current_model_->model_name_);
	// window->SetLogMessage("skybox_name", "skybox name: " + scene->current_iblmap_->skybox_name_);
#pragma endregion

#pragma region ����UniformBuffer
	const Vec3f camera_position = { 0, 0, 2 };	// ���λ��
	const Vec3f camera_target = { 0, 0, 0 };	// ��������λ��
	const Vec3f camera_up = { 0, 1, 0 };		// ������ϵ�λ��
	constexpr float fov = 70.0f;				// ��ӵĴ�ֱFOV
	const float aspect = static_cast<float>(width) / height;
	auto* camera = new Camera(camera_position, camera_target, camera_up, fov, aspect);

	const auto uniform_buffer = new UniformBuffer();
	uniform_buffer->model_matrix = model->model_matrix_;
	uniform_buffer->view_matrix = matrix_look_at(camera_position, camera_target, camera_up);
	uniform_buffer->project_matrix = camera->GetProjectionMatrix();

	uniform_buffer->light_direction = { 2, -2, -2 };
	uniform_buffer->light_color = Vec3f(1.0f);
	uniform_buffer->camera_position = camera->position_;

	// ������Ӱ��ؾ���
	uniform_buffer->shadow_view_matrix = matrix_look_at(camera_target - uniform_buffer->light_direction, camera_target, camera_up);
	// uniform_buffer->shadow_project_matrix = matrix_set_perspective(fov, camera->aspect_, camera->near_plane_, camera->far_plane_);
	float ortho_height = 2 * tan(0.5f * (fov / 180.0f * kPi)) * camera->far_plane_ * 1 / 100.0f;
	uniform_buffer->shadow_project_matrix = matrix_set_orthograhpic(aspect * ortho_height *0.5f, -aspect * ortho_height * 0.5f, ortho_height *0.5f, -ortho_height *0.5f, camera->near_plane_, camera->far_plane_);

	uniform_buffer->CalculateRestMatrix();
	data_buffer->SetUniformBuffer(uniform_buffer); // �����ݻ�������Uniform

#pragma endregion

#pragma region ��ʼ����Ⱦ��
	// ��ʼ��shader
	const auto blinn_phong_shader = new DefaultShader(uniform_buffer);
	const auto pbr_shader = new PBRShader(uniform_buffer);
	// const auto skybox_shader = new SkyBoxShader(uniform_buffer);
	// ��ʼ����Ӱshader
	const auto shadow_shader = new ShadowShader(uniform_buffer);
	// ��ʼ����Ⱦ��
	const auto renderer = new TaoRenderer(width, height);
	// ������Ⱦ״̬
	renderer->SetRenderState(false, true);	// ��Ⱦ�߿��Լ��������
	renderer->render_shadow_ = true;		// ��Ⱦ��Ӱ

	// ����Ĭ��ʹ�õ�shader
	renderer->current_shader_type_ = ShaderType::kBlinnPhongShader;

	// ���ô�����Ϣ
	if(camera->is_perspective_)
		window->SetLogMessage("Projection Mode", "Projection Mode : Perspective");
	else
		window->SetLogMessage("Projection Mode", "Projection Mode : Orthographic");

#pragma endregion

#pragma region ��Ⱦѭ��
	while (!window->is_close_) {
		// ��Ӧ���λ��
		camera->HandleInputEvents(); 
		// �Ƿ�������Ӱ
		if (window->keys_['S']) renderer->render_shadow_ = true;
		if (window->keys_['D']) renderer->render_shadow_ = false;

		// ��Blinn-Phong��ɫģ�ͻ���Cook-Torrance��ɫģ��
		if (window->keys_['B'])
		{
			window->SetLogMessage("Shading Model", "Shading Model: Blinn-Phong");
			renderer->current_shader_type_ = kBlinnPhongShader;
			blinn_phong_shader->material_inspector_ = DefaultShader::kMaterialInspectorShaded;
			window->RemoveLogMessage("Material Inspector");
		}
		else if (window->keys_['C'])
		{
			window->SetLogMessage("Shading Model", "Shading Model: Cook-Torrance");
			renderer->current_shader_type_ = kPbrShader;
			pbr_shader->material_inspector_ = PBRShader::kMaterialInspectorShaded;
			window->RemoveLogMessage("Material Inspector");
		}

		// �жϵ�ǰshader���ͣ�������Ⱦ����VS��PS
		switch (renderer->current_shader_type_)
		{
			case kBlinnPhongShader:
				renderer->SetVertexShader(blinn_phong_shader->vertex_shader_);
				renderer->SetPixelShader(blinn_phong_shader->pixel_shader_);

				blinn_phong_shader->HandleKeyEvents();
				break;
			case kPbrShader:
				renderer->SetVertexShader(pbr_shader->vertex_shader_);
				renderer->SetPixelShader(pbr_shader->pixel_shader_);

				pbr_shader->HandleKeyEvents();
				break;
			default:;
		}

		// ���֡����
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);

#pragma region ��ȾModel
		// ����shadow_shader
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