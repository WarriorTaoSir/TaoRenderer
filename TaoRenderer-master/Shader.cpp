#include "Shader.h"

/*
	文件内容：
	-Shader类的定义
	-最近一次修改日期：2024.12.16
*/
#pragma region Shadow Shader
Vec4f ShadowShader::VertexShaderFunction(int index, Varyings& output) const
{
	// 从DataBuffer中获取所需数据
	UniformBuffer* uniform_buffer_ = dataBuffer_->GetUniformBuffer();
	Attributes* attributes_ = dataBuffer_->attributes_;

	// 将模型空间变换至裁剪空间
	Vec4f position_cs = (uniform_buffer_->shadow_VP_matrix * uniform_buffer_->model_matrix) * attributes_[index].position_os.xyz1();
	const Vec3f position_ws = (uniform_buffer_->model_matrix * attributes_[index].position_os.xyz1()).xyz();

	return position_cs;
}

// 将深度信息渲染到shadowmap上
Vec4f ShadowShader::PixelShaderFunction(Varyings& input) const
{
	Vec3f display_color(0.f);

	return display_color.xyz1();
}
#pragma endregion

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
	// 从DataBuffer中获取所需数据
	UniformBuffer* uniform_buffer_ = dataBuffer_->GetUniformBuffer();
	Model* model = dataBuffer_->GetModel();
	
	Vec2f uv = input.varying_vec2f[VARYING_TEXCOORD];				// 获取UV
	Vec3f normal_ws = input.varying_vec3f[VARYING_NORMAL_WS];		// 获取法向量
	if (model->normal_map_->has_data_)								// 如果有法线贴图，则采样法线贴图
	{
		Vec4f tangent_ws = input.varying_vec4f[VARYING_TANGENT_WS];
		Vec3f perturb_normal = (model->normal_map_->Sample2D(uv)).xyz();
		perturb_normal = perturb_normal * 2.0f - Vec3f(1.0f);
		normal_ws = calculate_normal(normal_ws, tangent_ws, perturb_normal);
	}
	normal_ws = vector_normalize(normal_ws);						// 归一化法线

	Vec3f position_ws = input.varying_vec3f[VARYING_POSITION_WS];	// 着色点世界空间位置
	// 计算光线方向与视角方向
	Vec3f light_color = uniform_buffer_->light_color;
	Vec3f light_dir = vector_normalize(-uniform_buffer_->light_direction);
	Vec3f view_dir = vector_normalize(uniform_buffer_->camera_position - position_ws);

	Vec3f display_color(0.f);
	
	// 漫反射
	Vec3f base_color = model->base_color_map_->Sample2D(uv).xyz();
	Vec3f diffuse = light_color * base_color * Saturate(vector_dot(light_dir, normal_ws));
	// 高光
	Vec3f half_dir = vector_normalize(view_dir + light_dir);
	float specular_intensity = pow(Saturate(vector_dot(normal_ws, half_dir)), 128);
	Vec3f specular = light_color * specular_intensity;
	// 环境光
	Vec3f ambient_color = base_color * Vec3f(0.1f);

	// Diffuse + Specular + Ambient = Final Color
	Vec3f shaded_color = ambient_color + diffuse + specular;

	display_color = shaded_color;
	return display_color.xyz1();
}

#pragma endregion