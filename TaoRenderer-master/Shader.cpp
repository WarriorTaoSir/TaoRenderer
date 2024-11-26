#include "Shader.h"

/*
	文件内容：
	-Shader类的定义
	-最近一次修改日期：2024.11.21
*/

#pragma region Default Shader
Vec4f DefaultShader::VertexShaderFunction(int index, Varyings& output) const
{	
	// 从DataBuffer中获取所需数据
	UniformBuffer* uniform_buffer_ = dataBuffer_->GetUniformBuffer();
	Attributes* attributes_ = dataBuffer_->attributes_;

	// 将模型空间变换至裁剪空间
	Vec4f position_cs = uniform_buffer_->mvp_matrix * attributes_[index].position_os.xyz1();
	const Vec3f position_ws = (uniform_buffer_->model_matrix * attributes_[index].position_os.xyz1()).xyz();
	const Vec3f normal_ws = (uniform_buffer_->normal_matrix * attributes_[index].normal_os.xyz1()).xyz();
	const Vec4f tangent_ws = uniform_buffer_->model_matrix * attributes_[index].tangent_os;

	output.varying_vec2f[VARYING_TEXCOORD] = attributes_[index].texcoord;
	output.varying_vec3f[VARYING_POSITION_WS] = position_ws;
	output.varying_vec3f[VARYING_NORMAL_WS] = normal_ws;
	output.varying_vec4f[VARYING_TANGENT_WS] = tangent_ws;
	return position_cs;
}

Vec4f DefaultShader::PixelShaderFunction(Varyings& input) const {
	Vec3f display_color(0.f);
	return display_color.xyz1();
}