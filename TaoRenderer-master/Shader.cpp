#include "Shader.h"

/*
	文件内容：
	-Shader类的定义
	-最近一次修改日期：2024.12.24
*/

#pragma region ToneMapping

static float ACESToneMapping(float value)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	value = (value * (a * value + b)) / (value * (c * value + d) + e);
	return Between(0.0f, 1.0f, value);
}

static float GammaCorrection(float value)
{
	return  pow(value, 1.0f / 2.2f);
}

static Vec3f& PostProcessing(Vec3f& color)
{
	for (int i = 0; i < 3; i++)
	{
		color[i] = ACESToneMapping(color[i]);
	}
	return color;
}
#pragma endregion

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

void ShadowShader::HandleKeyEvents()
{

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
	Model* model = dataBuffer_->GetModelBeingRendered();
	
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

void DefaultShader::HandleKeyEvents()
{	
	Window* window_ = Window::GetInstance();

	for (MaterialInspector i = kMaterialInspectorShaded;
		i <= kMaterialInspectorSpecular;
		i = static_cast<MaterialInspector>(i + 1))
	{
		if (window_->keys_[i])
		{
			material_inspector_ = i;
			if (i == kMaterialInspectorShaded)
			{
				window_->RemoveLogMessage("Material Inspector");
			}
			else
			{
				window_->SetLogMessage("Material Inspector", "Material Inspector: " + material_inspector_name_[i - 49]);
			}
			return;
		}
	}
}

#pragma endregion

#pragma region PBR Shader

// 菲涅尔反射率的Schlick近似值，详见RTR4 章节9.5
// 在F0和F90（白色）之间进行插值，使用pow函数进行拟合，在接近掠射角度下快速增长
// 由于镜面微表面的假设，因此half_dir就是微表面的法线，cos(light_dir, half_dir)=cos(view_dir, half_dir)
Vec3f FresnelSchlickApproximation(const Vec3f& m, const Vec3f& light_dir, const Vec3f& f0)
{
	const float m_dot_l = Saturate(vector_dot(m, light_dir));
	return f0 + (Vec3f(1.0f) - f0) * pow(1.0f - m_dot_l, 5.0f);
}

// GGX法线分布函数，详见RTR4章节9.8中的方程9.41
float D_GGX_Original(const Vec3f& m, const Vec3f& n, const float roughness)
{
	const float n_dot_m = Saturate(vector_dot(n, m));
	const float n_dot_m_2 = n_dot_m * n_dot_m;

	const float roughness2 = roughness * roughness;

	const float factor = 1.0f + n_dot_m_2 * (roughness2 - 1.0f);
	const float D = n_dot_m * roughness2 / (kPi * factor * factor);

	return D;
}

// Smith G1函数，详见RTR4章节9.7中的方程9.24
// 使用GGX分布的lambda函数，详见章节9.8中的方程9.37和方程9.42
// 参数中的s可能是view_dir或者light_dir
float Smith_G1_GGX(const Vec3f& m, const Vec3f& n, const Vec3f& s, const float roughness)
{
	const float m_dot_s = Saturate(vector_dot(m, s));
	const float n_dot_s = Saturate(vector_dot(n, s));
	const float n_dot_s_2 = n_dot_s * n_dot_s;

	const float roughness2 = roughness * roughness;

	const float a2_reciprocal = roughness2 * (1 - n_dot_s_2) / (n_dot_s_2 + kEpsilon);
	const float lambda = (sqrtf(1.0f + a2_reciprocal) - 1.0f) * 0.5f;

	const float Smith_G1 = m_dot_s / (1.0f + lambda);
	return Smith_G1;
}

// 最简单形式的G2函数，详见RTR4章节9.7中的方程9.27
float Smith_G2_GGX(const Vec3f& m, const Vec3f& n,
	const Vec3f& light_dir, const Vec3f& view_dir,
	const float roughness)
{
	const float g1_shadowing = Smith_G1_GGX(m, n, light_dir, roughness);
	const float g1_masking = Smith_G1_GGX(m, n, view_dir, roughness);

	return  g1_shadowing * g1_masking;
}

// https://google.github.io/filament/Filament.html#materialsystem/specularbrdf Listing 4
float G_SmithGGXCorrelatedFast(const Vec3f& n,
	const Vec3f& light_dir, const Vec3f& view_dir,
	const float roughness)
{
	const float n_dot_l = Saturate(vector_dot(n, light_dir));
	const float n_dot_v = Saturate(vector_dot(n, view_dir));

	const float a = roughness;
	const float GGXV = n_dot_v * (n_dot_v * (1.0f - a) + a);
	const float GGXL = n_dot_l * (n_dot_l * (1.0f - a) + a);
	return 0.5f / (GGXV + GGXL);
}



Vec4f PBRShader::VertexShaderFunction(int index, Varyings& output) const
{
	// 从DataBuffer中获取所需数据
	UniformBuffer* uniform_buffer_ = dataBuffer_->GetUniformBuffer();
	Attributes* attributes_ = dataBuffer_->attributes_;
	Model* model_ = dataBuffer_->GetModelBeingRendered();

	Vec4f position_cs = uniform_buffer_->mvp_matrix * attributes_[index].position_os.xyz1();
	const Vec3f position_ws = (uniform_buffer_->model_matrix * attributes_[index].position_os.xyz1()).xyz();
	const Vec3f normal_ws = (uniform_buffer_->normal_matrix * attributes_[index].normal_os.xyz1()).xyz();
	if (model_->has_tangent_)
	{
		const Vec4f tangent_ws = uniform_buffer_->model_matrix * attributes_[index].tangent_os;
		output.varying_vec4f[VARYING_TANGENT_WS] = tangent_ws;
	}
	output.varying_vec2f[VARYING_TEXCOORD] = attributes_[index].texcoord;
	output.varying_vec3f[VARYING_POSITION_WS] = position_ws;
	output.varying_vec3f[VARYING_NORMAL_WS] = normal_ws;
	return position_cs;
}

Vec4f PBRShader::PixelShaderFunction(Varyings& input) const 
{	
	// 从DataBuffer中获取所需数据
	UniformBuffer* uniform_buffer_ = dataBuffer_->GetUniformBuffer();
	Model* model_ = dataBuffer_->GetModelBeingRendered();

	Vec2f uv = input.varying_vec2f[VARYING_TEXCOORD];					// 纹理坐标
	Vec3f position_ws = input.varying_vec3f[VARYING_POSITION_WS];		// 世界空间坐标

	Vec3f normal_ws = input.varying_vec3f[VARYING_NORMAL_WS];			// 法线
	if (model_->normal_map_->has_data_ && model_->has_tangent_)			
	{
		Vec4f tangent_ws = input.varying_vec4f[VARYING_TANGENT_WS];
		Vec3f perturb_normal = (model_->normal_map_->Sample2D(uv)).xyz();
		perturb_normal = perturb_normal * 2.0f - Vec3f(1.0f);
		normal_ws = calculate_normal(normal_ws, tangent_ws, perturb_normal);
	}
	normal_ws = vector_normalize(normal_ws);


	float metallic = model_->metallic_map_->Sample2D(uv).b;					// 金属度
	float perceptual_roughness = model_->roughness_map_->Sample2D(uv).b;	// 粗糙度
	float roughness = perceptual_roughness * perceptual_roughness;		

	Vec3f occlusion(1.0f);
	if (model_->occlusion_map_->has_data_)
		occlusion = model_->occlusion_map_->Sample2D(uv).b;			// 环境光遮蔽（没有则默认为1.0，即全亮）
	Vec3f emission(0.0f);
	if (model_->emission_map_->has_data_)
		emission = model_->emission_map_->Sample2D(uv).xyz();		// 自发光（没有则默认为0.0，全黑）
	Vec3f base_color = model_->base_color_map_->Sample2D(uv).xyz();	// 非金属部分为albedo，金属部分为F0

	Vec3f light_color = uniform_buffer_->light_color;						// 光源颜色 
	Vec3f light_dir = vector_normalize(-uniform_buffer_->light_direction);	// 光源方向 L

	Vec3f view_dir = vector_normalize(uniform_buffer_->camera_position - position_ws);		// 观察方向 V
	Vec3f half_dir = vector_normalize(view_dir + light_dir);								// 半程向量	H = (L+V)/2

	float n_dot_l = vector_dot(normal_ws, light_dir);
	float n_dot_l_abs = Abs(n_dot_l);
	n_dot_l = Saturate(n_dot_l);

	float n_dot_v = vector_dot(normal_ws, view_dir);
	float n_dot_v_abs = Abs(n_dot_v);
	n_dot_v = Saturate(n_dot_v);

	//——————————————————计算直接光照——————————————————//

	// 镜面高光部分
	Vec3f f0 = vector_lerp(dielectric_f0_, base_color, metallic);					// 获取材质的F0值
	Vec3f F = FresnelSchlickApproximation(half_dir, light_dir, f0);					// 菲涅尔项， F项

	float D = D_GGX_Original(half_dir, normal_ws, roughness);			            // 法线分布项，D项

	float G = Smith_G2_GGX(half_dir, normal_ws, light_dir, view_dir, roughness);	// shadowing-masking项，G项

	Vec3f cook_torrance_brdf = (D * G) * F / (4.0f * n_dot_l_abs * n_dot_v_abs + kEpsilon);

	// 漫反射部分
	Vec3f kd = (Vec3f(1.0f) - F) * (1 - metallic);
	Vec3f lambertian_brdf = base_color;

	Vec3f radiance_direct = (kd * lambertian_brdf + cook_torrance_brdf) * light_color * n_dot_l;


	//——————————————————计算环境光照——————————————————//

	auto specular_cubemap_ = dataBuffer_->specular_cubemap_;
	auto brdf_lut_ = dataBuffer_->brdf_lut_;
	auto irradiance_cubemap_ = dataBuffer_->irradiance_cubemap_;

	// 获取最大mipmap等级
	int max_mipmap_level = SpecularCubeMap::max_mipmap_level_ - 1;
	// 根据粗糙度获取高光mipmap等级
	int specular_mipmap_level = roughness * max_mipmap_level + 0.5f;
	// 计算反射向量
	Vec3f reflected_view_dir = vector_reflect(view_dir, normal_ws);

	Vec3f prefilter_specular_color = specular_cubemap_->prefilter_maps_[specular_mipmap_level]->Sample(reflected_view_dir);
	
	Vec2f lut_uv = { n_dot_v ,roughness };
	Vec2f lut_sample = brdf_lut_->Sample2D(lut_uv).xy();

	// 计算镜面高光的IBL
	float specular_scale = lut_sample.x;
	float specular_bias = lut_sample.y;
	Vec3f specular = f0 * specular_scale + Vec3f(specular_bias);
	Vec3f radiance_specular_ibl = prefilter_specular_color * specular;
	
	// 计算漫反射光的IBL
	Vec3f irradiance = irradiance_cubemap_->Sample(normal_ws);
	Vec3f radiance_diffuse_ibl = kd * irradiance * base_color;

	Vec3f radiance_ibl = (radiance_diffuse_ibl + radiance_specular_ibl) * occlusion;


	//——————————————————确定最终颜色——————————————————//
	// 最终颜色
	Vec3f shaded_color = (radiance_direct + radiance_ibl) + emission;

	Vec3f display_color;
	switch (material_inspector_)
	{
		case kMaterialInspectorShaded:			display_color = PostProcessing(shaded_color);	break;
		case kMaterialInspectorBaseColor:		display_color = base_color;		break;
		case kMaterialInspectorNormal:			display_color = normal_ws;		break;
		case kMaterialInspectorWorldPosition:	display_color = position_ws;	break;
		case kMaterialInspectorRoughness:		display_color = roughness;		break;
		case kMaterialInspectorMetallic:		display_color = metallic;		break;
		case kMaterialInspectorOcclusion:		display_color = occlusion;		break;
		case kMaterialInspectorEmission:		display_color = emission;		break;

		default:								display_color = shaded_color;
	}

	return display_color.xyz1();
}

void PBRShader::HandleKeyEvents()
{	
	Window* window_ = Window::GetInstance();
	for (MaterialInspector i = kMaterialInspectorShaded;
		i <= kMaterialInspectorEmission;
		i = static_cast<MaterialInspector>(i + 1))
	{
		if (window_->keys_[i])
		{
			material_inspector_ = i;
			if (i == kMaterialInspectorShaded)
			{
				window_->RemoveLogMessage("Material Inspector");
			}
			else
			{
				window_->SetLogMessage("Material Inspector", "Material Inspector: " + material_inspector_name_[i - 49]);
			}
			return;
		}
	}
}
#pragma endregion

#pragma region SkyBox

Vec4f SkyBoxShader::VertexShaderFunction(int index, Varyings& output) const
{
	auto uniform_buffer_ = dataBuffer_->GetUniformBuffer();
	auto attributes_ = dataBuffer_->attributes_;
	Vec4f position_cs = uniform_buffer_->mvp_matrix * attributes_[index].position_os.xyz1();
	const Vec3f position_ws = (uniform_buffer_->model_matrix * attributes_[index].position_os.xyz1()).xyz();

	output.varying_vec3f[VARYING_POSITION_WS] = position_ws;
	return position_cs;
}

Vec4f SkyBoxShader::PixelShaderFunction(Varyings& input) const
{
	Vec3f position_ws = input.varying_vec3f[VARYING_POSITION_WS];		// 世界空间坐标
	return  dataBuffer_->skybox_cubemap_->Sample(position_ws).xyz1();
}

#pragma endregion 