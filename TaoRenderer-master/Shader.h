#pragma once

#include "Math.h"

#include "DataBuffer.h"
#include <map>
#include <functional>

/*
	文件内容：
	-Shader类的声明
	-最近一次修改日期：2024.12.16
*/

#pragma region 相关结构体/定义
// 着色器上下文，由 VS 设置，再由渲染器按像素逐点插值后，供 PS 读取
struct Varyings {
	std::map<int, float> varying_float;    // 浮点数 varying 列表
	std::map<int, Vec2f> varying_vec2f;    // 二维矢量 varying 列表
	std::map<int, Vec3f> varying_vec3f;    // 三维矢量 varying 列表
	std::map<int, Vec4f> varying_vec4f;    // 四维矢量 varying 列表
};

enum ShaderType
{
	kBlinnPhongShader,
	kPbrShader,
	kSkyBoxShader
};

// 顶点着色器：返回顶点的裁剪空间坐标
typedef std::function<Vec4f(int index, Varyings& output)> VertexShader;

// 像素着色器：返回像素的颜色
typedef std::function<Vec4f(Varyings& input)> PixelShader;

#pragma endregion

#pragma region shader类定义

class IShader
{
public:
	DataBuffer* dataBuffer_;
	VertexShader vertex_shader_;
	PixelShader pixel_shader_;

public:

	IShader(UniformBuffer* uniform_buffer)
	{
		//window_ = Window::GetInstance();
		dataBuffer_ = DataBuffer::GetInstance();

		vertex_shader_ = [&](const int index, Varyings& output)->Vec4f
			{
				return VertexShaderFunction(index, output);
			};
		pixel_shader_ = [&](Varyings& input)->Vec4f
			{
				return PixelShaderFunction(input);
			};
	}
	virtual ~IShader() = default;

	virtual Vec4f VertexShaderFunction(int index, Varyings& output) const = 0;
	virtual Vec4f PixelShaderFunction(Varyings& input) const = 0;
	virtual  void HandleKeyEvents() = 0;
	
};

// 用于渲染阴影贴图的shader
class ShadowShader final : public IShader
{
public:
	enum VaryingAttributes
	{
		VARYING_TEXCOORD = 0,			// 纹理坐标
		VARYING_POSITION_WS = 1,		// 世界空间坐标
		VARYING_NORMAL_WS = 2,			// 世界空间法线
		VARYING_TANGENT_WS = 3,			// 世界空间切线
	};
public:
	ShadowShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer) {}
	Vec4f VertexShaderFunction(int index, Varyings& output) const override;
	Vec4f PixelShaderFunction(Varyings& input) const override;
	void HandleKeyEvents()  override;
};

// 一个临时使用的默认shader 可以看做是Blinn-Phong着色模型
class DefaultShader final : public IShader
{
public:
	enum VaryingAttributes
	{
		VARYING_TEXCOORD = 0,			// 纹理坐标
		VARYING_POSITION_WS = 1,		// 世界空间坐标
		VARYING_NORMAL_WS = 2,			// 世界空间法线
		VARYING_TANGENT_WS = 3			// 世界空间切线
	};
	enum MaterialInspector
	{
		kMaterialInspectorShaded = '1',
		kMaterialInspectorBaseColor,
		kMaterialInspectorNormal,
		kMaterialInspectorWorldPosition,
		kMaterialInspectorAmbient,
		kMaterialInspectorDiffuse,
		kMaterialInspectorSpecular
	};

	const std::string material_inspector_name_[7] =
	{
		"Shaded",
		"BaseColor",
		"Normal",
		"WorldPosition",
		"Ambient",
		"Diffuse",
		"Specular"
	};

	MaterialInspector material_inspector_;

public:
	DefaultShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer) {}
	Vec4f VertexShaderFunction(int index, Varyings& output) const override;
	Vec4f PixelShaderFunction(Varyings& input) const override;
	void HandleKeyEvents()  override;
};


// PBR光照模型， 使用metallic工作流
class PBRShader final :public IShader
{
public:
	PBRShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer) {
		// 非金属的F0值默认为0.04
		dielectric_f0_ = Vec3f(0.04f);
		material_inspector_ = kMaterialInspectorShaded;

		// 是否使用LUT
		use_lut_ = false;
	}
	Vec4f VertexShaderFunction(int index, Varyings& output) const override;
	Vec4f PixelShaderFunction(Varyings& input) const override;
	void HandleKeyEvents() override;    // 材质查看器

	enum VaryingAttributes
	{
		VARYING_TEXCOORD = 0,			// 纹理坐标
		VARYING_POSITION_WS = 1,		// 世界空间坐标
		VARYING_NORMAL_WS = 2,			// 世界空间法线
		VARYING_TANGENT_WS = 3			// 世界空间切线
	};

	enum MaterialInspector
	{
		kMaterialInspectorShaded = '1',
		kMaterialInspectorBaseColor,
		kMaterialInspectorNormal,
		kMaterialInspectorWorldPosition,
		kMaterialInspectorRoughness,
		kMaterialInspectorMetallic,
		kMaterialInspectorOcclusion,
		kMaterialInspectorEmission
	};

	const std::string material_inspector_name_[8] =
	{
		"Shaded",
		"BaseColor",
		"Normal",
		"WorldPosition",
		"Roughness",
		"Metallic",
		"Occlusion",
		"Emission"
	};

	MaterialInspector material_inspector_;

	Vec3f dielectric_f0_; // 电介质F0值

	bool use_lut_;	      // 是否使用Lut							
};

class SkyBoxShader final :public IShader
{
public:
	SkyBoxShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer)
	{

	}

	Vec4f VertexShaderFunction(int index, Varyings& output) const override;
	Vec4f PixelShaderFunction(Varyings& input) const override;
	void HandleKeyEvents() override {};

	enum VaryingAttributes
	{
		VARYING_POSITION_WS = 0,		// 世界空间坐标
	};

public:

	std::vector<Vec3f> plane_vertex_ = {
		{0.5f,0.5f,0.5f},			// 右上角
		{-0.5f,0.5f,0.5f},			// 左上角
		{-0.5f,-0.5f,0.5f},		// 左下角
		{0.5f,-0.5f,0.5f} };		// 右下角
	std::vector<int> plane_index_ = { 0,1,2,     0,2,3 };
};