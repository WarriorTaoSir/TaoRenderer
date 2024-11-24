#pragma once

#include "Math.h"
#include "Window.h"
#include "Model.h"
#include "DataBuffer.h"
#include <map>
#include <functional>

/*
	文件内容：
	-Shader类的声明
	-最近一次修改日期：2024.11.21
*/

#pragma region 相关结构体/定义
// 着色器上下文，由 VS 设置，再由渲染器按像素逐点插值后，供 PS 读取
struct Varings {
	std::map<int, float> varying_float;    // 浮点数 varying 列表
	std::map<int, Vec2f> varying_vec2f;    // 二维矢量 varying 列表
	std::map<int, Vec3f> varying_vec3f;    // 三维矢量 varying 列表
	std::map<int, Vec4f> varying_vec4f;    // 四维矢量 varying 列表
};

enum ShaderType
{
	kBlinnPhongShader
};

// 顶点着色器：返回顶点的裁剪空间坐标
typedef std::function<Vec4f(int index, Varings& output)> VertexShader;

// 像素着色器：返回像素的颜色
typedef std::function<Vec4f(Varings& input)> PixelShader;

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

		vertex_shader_ = [&](const int index, Varings& output)->Vec4f
			{
				return VertexShaderFunction(index, output);
			};
		pixel_shader_ = [&](Varings& input)->Vec4f
			{
				return PixelShaderFunction(input);
			};
	}
	virtual ~IShader() = default;

	virtual Vec4f VertexShaderFunction(int index, Varings& output) const = 0;
	virtual Vec4f PixelShaderFunction(Varings& input) const = 0;
	//virtual  void HandleKeyEvents() = 0;
	
};

// 一个临时使用的默认shader
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
	};

public:
	DefaultShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer) {}
	Vec4f VertexShaderFunction(int index, Varings& output) const override;
	Vec4f PixelShaderFunction(Varings& input) const override;
};