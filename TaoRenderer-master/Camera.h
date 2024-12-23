#pragma once

#include "Math.h"
#include "Window.h"
#include "Shader.h"

/*
	文件内容：
	-相机类的声明
	-最近一次修改日期：2024.11.19
*/

class Camera
{
public:
	Window* window_;					// 窗口
	DataBuffer* data_buffer_;			// 数据缓冲
	Vec3f position_, origin_position_;	// 相机的世界空间位置
	Vec3f target_, origin_target_;		// 相机看向的世界空间位置
	Vec3f up_;

	bool is_perspective_;				// 是否为透视投影
	float scale_factor_;				// 缩放系数
	// 相机的up向量

	/*
		相机坐标系的轴
		axis_r：正方向指向屏幕右侧
		axis_u：正方向指向屏幕上侧
		axis_v：正方向指向屏幕
	*/

	Vec3f axis_r_, axis_u_, axis_v_;

	float fov_;						// 相机FOV
	float aspect_;					// 长宽比

	float near_plane_;				// 近裁剪平面
	float far_plane_;				// 远裁剪平面

public:
	Camera(const Vec3f& position, const Vec3f& target, const Vec3f& up, float fov, float aspect); // 相机初始化，世界空间位置，up，朝向，FOV，aspect
	~Camera();
	
	void HandleInputEvents(); // 响应键盘输入，控制相机移动

	// 更新uniform buffer中的矩阵
	void UpdateUniformBuffer(UniformBuffer* uniform_buffer, const Mat4x4f& model_matrix) const;
	void UpdateSkyBoxUniformBuffer(UniformBuffer* uniform_buffer) const;
	void UpdateSkyboxMesh(SkyBoxShader* sky_box_shader) const;

	// 获取当前投影矩阵
	Mat4x4f GetProjectionMatrix() const;

private:
	// 更新相机姿态
	void UpdateCameraPose();		// 更新相机位置
	void HandleMouseEvents();		// 响应鼠标输入
	void HandleKeyEvents();			// 响应键盘输入
};