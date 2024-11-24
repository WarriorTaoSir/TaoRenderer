#pragma once

#include "Math.h"
#include "Window.h"
#include "Model.h"
#include "DataBuffer.h"
#include <map>
#include <functional>

/*
	�ļ����ݣ�
	-Shader�������
	-���һ���޸����ڣ�2024.11.21
*/

#pragma region ��ؽṹ��/����
// ��ɫ�������ģ��� VS ���ã�������Ⱦ������������ֵ�󣬹� PS ��ȡ
struct Varings {
	std::map<int, float> varying_float;    // ������ varying �б�
	std::map<int, Vec2f> varying_vec2f;    // ��άʸ�� varying �б�
	std::map<int, Vec3f> varying_vec3f;    // ��άʸ�� varying �б�
	std::map<int, Vec4f> varying_vec4f;    // ��άʸ�� varying �б�
};

enum ShaderType
{
	kBlinnPhongShader
};

// ������ɫ�������ض���Ĳü��ռ�����
typedef std::function<Vec4f(int index, Varings& output)> VertexShader;

// ������ɫ�����������ص���ɫ
typedef std::function<Vec4f(Varings& input)> PixelShader;

#pragma endregion

#pragma region shader�ඨ��

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

// һ����ʱʹ�õ�Ĭ��shader
class DefaultShader final : public IShader
{
public:
	enum VaryingAttributes
	{
		VARYING_TEXCOORD = 0,			// ��������
		VARYING_POSITION_WS = 1,		// ����ռ�����
		VARYING_NORMAL_WS = 2,			// ����ռ䷨��
		VARYING_TANGENT_WS = 3			// ����ռ�����
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