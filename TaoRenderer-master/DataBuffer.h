#pragma once
#include "model.h"

/*
	文件内容：
	-数据缓冲类的声明
	-包含了Renderer光栅化和shader计算所要用到的数据
	-例如UniformBuffer，CurrentModel，
	-最近一次修改日期：2024.11.24
*/

#pragma region 渲染所需数据结构的定义
struct UniformBuffer
{
	Mat4x4f model_matrix;		// 模型变换矩阵
	Mat4x4f view_matrix;		// 观察变换矩阵
	Mat4x4f project_matrix;		// 投影变换矩阵
	Mat4x4f mvp_matrix;			// MVP变换矩阵

	Mat4x4f normal_matrix;		// 法线变换矩阵

	void CalculateRestMatrix() {
		mvp_matrix = project_matrix * view_matrix * model_matrix;
		// 用于将法线从模型空间变换到世界空间,使用原始变换矩阵的逆变换矩阵
		normal_matrix = matrix_invert(model_matrix).Transpose();
	}

	// 光照数据
	Vec3f light_direction;		// 光照方向（由着色点指向光源）
	Vec3f light_color;			// 光照颜色
	Vec3f camera_position;		// 相机方向
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