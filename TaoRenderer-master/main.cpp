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
	data_buffer->SetModel(scene->current_model_); // 给数据缓冲设置当前模型

	window->SetLogMessage("model_message", model->PrintModelInfo());
	window->SetLogMessage("model_name", "model name: " + scene->current_model_->model_name_);
	// window->SetLogMessage("skybox_name", "skybox name: " + scene->current_iblmap_->skybox_name_);
#pragma endregion

#pragma region 配置UniformBuffer
	const Vec3f camera_position = { 0, 0, 2 };	// 相机位置
	const Vec3f camera_target = { 0, 0, 0 };	// 相机看向的位置
	const Vec3f camera_up = { 0, 1, 0 };		// 相机向上的位置
	constexpr float fov = 70.0f;				// 相加的垂直FOV
	auto* camera = new Camera(camera_position, camera_target, camera_up, fov, static_cast<float>(width) / height);

	const auto uniform_buffer = new UniformBuffer();
	uniform_buffer->model_matrix = model->model_matrix_;
	uniform_buffer->view_matrix = matrix_look_at(camera_position, camera_target, camera_up);
	uniform_buffer->project_matrix = matrix_set_perspective(fov, camera->aspect_, camera->near_plane_, camera->far_plane_);
	

	uniform_buffer->light_direction = { 2, -2, -2 };
	uniform_buffer->light_color = Vec3f(1.0f);
	uniform_buffer->camera_position = camera->position_;

	// 配置阴影相关矩阵
	uniform_buffer->shadow_view_matrix = matrix_look_at(camera_target - uniform_buffer->light_direction, camera_target, camera_up);
	uniform_buffer->shadow_project_matrix = matrix_set_perspective(fov, camera->aspect_, camera->near_plane_, camera->far_plane_);
	//uniform_buffer->shadow_project_matrix = matrix_set_orthograhpic(width*0.5f, -width * 0.5f, height*0.5f, -height*0.5f, camera->near_plane_, camera->far_plane_);

	uniform_buffer->CalculateRestMatrix();
	data_buffer->SetUniformBuffer(uniform_buffer); // 给数据缓冲设置Uniform

#pragma endregion

#pragma region 初始化渲染器
	// 初始化shader
	const auto shader = new DefaultShader(uniform_buffer);
	// 初始化阴影shader
	const auto shadow_shader = new ShadowShader(uniform_buffer);
	// 初始化渲染器
	const auto renderer = new TaoRenderer(width, height);
	// 设置渲染状态
	renderer->SetRenderState(false, true);	// 渲染线框以及填充像素
	renderer->render_shadow_ = false;		// 渲染阴影

#pragma endregion

#pragma region 渲染循环

	while (!window->is_close_) {
		// 响应相机位置
		camera->HandleInputEvents(); 
		if (window->keys_['S']) renderer->render_shadow_ = true;
		if (window->keys_['D']) renderer->render_shadow_ = false;
		// 清除帧缓冲
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);

#pragma region 渲染Model
		// 设置渲染器所使用的shader
		renderer->SetVertexShader(shader->vertex_shader_);
		renderer->SetPixelShader(shader->pixel_shader_);
		renderer->SetShadowVertexShader(shadow_shader->vertex_shader_);

		// 使用渲染器将深度信息绘制到ShadowBuffer里
		renderer->DrawShadowMap();
		renderer->ClearFrameBuffer(renderer->render_frame_ || renderer->render_pixel_, true);
		// 使用渲染器将模型绘制到FrameBuffer里
		renderer->DrawMesh();
#pragma endregion 
		// 在窗口展示渲染器的帧缓冲
		window->WindowDisplay(data_buffer->color_buffer_);
	}
#pragma endregion 
	
	return 0;
}