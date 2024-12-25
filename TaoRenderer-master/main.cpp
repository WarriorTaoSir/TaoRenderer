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
	渲染器的入口
*/
void HandleModelSkyboxSwitchEvents(Window* window, Scene* scene, TaoRenderer* mo_renderer);

int main() {
#pragma region 窗口初始化
	constexpr int width = 550;
	constexpr int height = 500;

	Window* window = Window::GetInstance();
	window->WindowInit(width, height, "TaoRenderer");
#pragma endregion

#pragma region 外部资源加载
	const auto scene = new Scene();						// 展示的场景
	DataBuffer* data_buffer = DataBuffer::GetInstance(); // 着色器所需数据

	auto model = scene->current_model_; // 获取当前模型
	
	for (int i = 0; i < scene->total_model_count_; i++) {
		data_buffer->model_list_.push_back(scene->current_model_);
		scene->LoadNextModel();
	}

	window->SetLogMessage("model_message", model->PrintModelInfo());
	// window->SetLogMessage("model_name", "model name: " + scene->current_model_->model_name_);
	// window->SetLogMessage("skybox_name", "skybox name: " + scene->current_iblmap_->skybox_name_);
#pragma endregion

#pragma region 配置UniformBuffer
	const Vec3f camera_position = { 0, 0, 2 };	// 相机位置
	const Vec3f camera_target = { 0, 0, 0 };	// 相机看向的位置
	const Vec3f camera_up = { 0, 1, 0 };		// 相机向上的位置
	constexpr float fov = 70.0f;				// 相加的垂直FOV
	const float aspect = static_cast<float>(width) / height;
	auto* camera = new Camera(camera_position, camera_target, camera_up, fov, aspect);

	const auto uniform_buffer = new UniformBuffer();
	uniform_buffer->model_matrix = model->model_matrix_;
	uniform_buffer->view_matrix = matrix_look_at(camera_position, camera_target, camera_up);
	uniform_buffer->project_matrix = camera->GetProjectionMatrix();

	uniform_buffer->light_direction = { 2, -2, -2 };
	uniform_buffer->light_color = Vec3f(1.0f);
	uniform_buffer->camera_position = camera->position_;

	// 配置阴影相关矩阵
	uniform_buffer->shadow_view_matrix = matrix_look_at(camera_target - uniform_buffer->light_direction, camera_target, camera_up);
	// uniform_buffer->shadow_project_matrix = matrix_set_perspective(fov, camera->aspect_, camera->near_plane_, camera->far_plane_);
	float ortho_height = 2 * tan(0.5f * (fov / 180.0f * kPi)) * camera->far_plane_ * 1 / 100.0f;
	uniform_buffer->shadow_project_matrix = matrix_set_orthograhpic(aspect * ortho_height *0.5f, -aspect * ortho_height * 0.5f, ortho_height *0.5f, -ortho_height *0.5f, camera->near_plane_, camera->far_plane_);

	uniform_buffer->CalculateRestMatrix();
	data_buffer->SetUniformBuffer(uniform_buffer); // 给数据缓冲设置Uniform

#pragma endregion

#pragma region 初始化渲染器
	// 初始化shader
	const auto blinn_phong_shader = new DefaultShader(uniform_buffer);
	const auto pbr_shader = new PBRShader(uniform_buffer);
	const auto skybox_shader = new SkyBoxShader(uniform_buffer);
	const auto shadow_shader = new ShadowShader(uniform_buffer);
	// 初始化渲染器
	const auto renderer = new TaoRenderer(width, height);
	// 设置渲染状态
	renderer->SetRenderState(false, true);	// 渲染线框以及填充像素
	renderer->render_shadow_ = false;		// 渲染阴影

	// 设置默认使用的shader
	renderer->current_shader_type_ = ShaderType::kPbrShader;

	// 设置窗口信息
	if(camera->is_perspective_)
		window->SetLogMessage("Projection Mode", "Projection Mode : Perspective");
	else
		window->SetLogMessage("Projection Mode", "Projection Mode : Orthographic");

#pragma endregion

#pragma region 渲染循环
	while (!window->is_close_) {
		HandleModelSkyboxSwitchEvents(window, scene, renderer);		// 切换天空盒和模型，切换线框渲染
		// 清除上一帧的帧缓冲
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);
		data_buffer->UpdateInfoInScene(scene);
#pragma region 按键响应
		// 响应相机位置
		camera->HandleInputEvents(); 
		// 更新UniformBuffer
		camera->UpdateUniformBuffer(data_buffer->GetUniformBuffer(), data_buffer->GetModelBeingRendered()->model_matrix_);
		// 是否生成阴影
		if (window->keys_['S']) renderer->render_shadow_ = true;
		if (window->keys_['D']) renderer->render_shadow_ = false;

		// 是Blinn-Phong着色模型还是Cook-Torrance着色模型
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
#pragma endregion 
		// 动态切换Shader，判断当前shader类型，设置渲染器的VS与PS
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
		// 设置shadow_shader
		renderer->SetShadowShader(shadow_shader);
#pragma region 渲染Model
		// 使用渲染器将深度信息绘制到ShadowBuffer里
		renderer->DrawShadowMap();
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);
		// 使用渲染器将模型绘制到FrameBuffer里
		renderer->DrawMesh();
#pragma endregion 

#pragma region 渲染Skybox
		renderer->SetSkyboxShader(skybox_shader);

		camera->UpdateSkyBoxUniformBuffer(data_buffer->GetUniformBuffer());
		camera->HandleInputEvents();
		camera->UpdateSkyboxMesh(skybox_shader);

		renderer->DrawSkybox();
#pragma endregion
		// 在窗口展示渲染器的帧缓冲
		window->WindowDisplay(data_buffer->color_buffer_);
	}
#pragma endregion 
	
	return 0;
}


void HandleModelSkyboxSwitchEvents(Window* window, Scene* scene, TaoRenderer* mo_renderer)
{
	if (window->can_press_keyboard_)
	{
		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			scene->LoadPrevModel();
			window->SetLogMessage("model_message", scene->current_model_->PrintModelInfo());
			//window->SetLogMessage("model_name", "model name: " + scene->current_model_->model_name_);
			window->can_press_keyboard_ = false;

		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			scene->LoadNextModel();
			window->SetLogMessage("model_message", scene->current_model_->PrintModelInfo());
			//window->SetLogMessage("model_name", "model name: " + scene->current_model_->model_name_);
			window->can_press_keyboard_ = false;
		}
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			scene->LoadPrevIBLMap();
			window->SetLogMessage("skybox_name", "skybox name: " + scene->current_iblmap_->skybox_name_);
			window->can_press_keyboard_ = false;
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			scene->LoadNextIBLMap();
			window->SetLogMessage("skybox_name", "skybox name: " + scene->current_iblmap_->skybox_name_);
			window->can_press_keyboard_ = false;
		}
		else if (window->keys_['0'])					// 切换渲染模式：线框渲染-像素渲染
		{
			if (mo_renderer->render_frame_)
			{
				mo_renderer->SetRenderState(false, true);
			}
			else
			{
				mo_renderer->SetRenderState(true, false);
			}
			window->can_press_keyboard_ = false;
		}

	}
}
