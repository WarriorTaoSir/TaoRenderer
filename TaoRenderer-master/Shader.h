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
	-���һ���޸����ڣ�2024.12.16
*/

#pragma region ��ؽṹ��/����
// ��ɫ�������ģ��� VS ���ã�������Ⱦ������������ֵ�󣬹� PS ��ȡ
struct Varyings {
	std::map<int, float> varying_float;    // ������ varying �б�
	std::map<int, Vec2f> varying_vec2f;    // ��άʸ�� varying �б�
	std::map<int, Vec3f> varying_vec3f;    // ��άʸ�� varying �б�
	std::map<int, Vec4f> varying_vec4f;    // ��άʸ�� varying �б�
};

enum ShaderType
{
	kBlinnPhongShader,
	kPbrShader,
	kSkyBoxShader
};

// ������ɫ�������ض���Ĳü��ռ�����
typedef std::function<Vec4f(int index, Varyings& output)> VertexShader;

// ������ɫ�����������ص���ɫ
typedef std::function<Vec4f(Varyings& input)> PixelShader;

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

// ������Ⱦ��Ӱ��ͼ��shader
class ShadowShader final : public IShader
{
public:
	enum VaryingAttributes
	{
		VARYING_TEXCOORD = 0,			// ��������
		VARYING_POSITION_WS = 1,		// ����ռ�����
		VARYING_NORMAL_WS = 2,			// ����ռ䷨��
		VARYING_TANGENT_WS = 3,			// ����ռ�����
	};
public:
	ShadowShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer) {}
	Vec4f VertexShaderFunction(int index, Varyings& output) const override;
	Vec4f PixelShaderFunction(Varyings& input) const override;
	void HandleKeyEvents()  override;
};

// һ����ʱʹ�õ�Ĭ��shader ���Կ�����Blinn-Phong��ɫģ��
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


// PBR����ģ�ͣ� ʹ��metallic������
class PBRShader final :public IShader
{
public:
	PBRShader(UniformBuffer* uniform_buffer) : IShader(uniform_buffer) {}
	Vec4f VertexShaderFunction(int index, Varyings& output) const override;
	Vec4f PixelShaderFunction(Varyings& input) const override;
	void HandleKeyEvents() override;    // ���ʲ鿴��

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

	Vec3f dielectric_f0_;

	//CubeMap* irradiance_cubemap_;			// ����irradiance��ͼ
	//SpecularCubeMap* specular_cubemap_;		// �߹�cubemap

	Texture* brdf_lut_;

	bool use_lut_;								
};