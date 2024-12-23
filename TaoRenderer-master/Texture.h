#pragma once
#include "Math.h"


/*
	�ļ����ݣ�
	-��ͼ�������
	-����������ͼ����������ͼ��Ԥ���˵Ļ�����ͼ��IBL��ͼ�ȵ�
	-���һ���޸����ڣ�2024.12.24
*/

enum TextureType
{
	kTextureTypeBaseColor,
	kTextureTypeNormal,
	kTextureTypeRoughness,
	kTextureTypeMetallic,
	kTextureTypeOcclusion,
	kTextureTypeEmission
};

// ����������ͼ
class Texture
{
public:
	int texture_width_;				// ������
	int texture_height_;			// ����߶�
	int texture_channels_;			// ����ͨ���� ����ȣ�

	bool has_data_;					// �Ƿ�ɹ�������ͼ, �Ƿ��������
	unsigned char* texture_data_;	// ʵ�ʵ�ͼ������ָ�룬 ���ݿ��ߣ�ͨ��������ȡͼƬ

public:
	Texture(const std::string& file_name); // ��ȡ�ļ�·������ʼ����ͼ
	~Texture();      

	// �������
	Vec4f Sample2D(float u, float v) const;	// �ֱ����u��v����
	Vec4f Sample2D(Vec2f uv) const;			// ����uv��������
private:
	ColorRGBA GetPixelColor(int x, int y) const;		// ��ȡframe��ĳһ���ص���ɫ
	ColorRGBA SampleBilinear(float x, float y) const;	// ˫���Բ���
	// ��ɫ˫���Բ�ֵ��ȷ���ĸ��ǵ���ɫ�Լ���ֵ���꣬��ȷ��������������ڲ�ĳһ�����ɫ
	static ColorRGBA BilinearInterpolation(const ColorRGBA& color00, const ColorRGBA& color01, const ColorRGBA& color10, const ColorRGBA& color11, float t_x, float t_y);
};

class CubeMap
{
public:
	struct CubeMapUV {
		int face_id;
		Vec2f uv;
	};

	enum CubeMapType
	{
		kSkybox,                // for skybox itself
		kIrradianceMap,			// for diffuse
		kSpecularMap            // for glossy
	};

public:
	CubeMap(const std::string& file_folder, CubeMapType cube_map_type, int mipmap_level = 0);
	~CubeMap();
	Vec3f Sample(Vec3f& direction) const;

	static CubeMapUV& CalculateCubeMapUV(Vec3f& direction);

public:
	Texture* cubemap_[6];
	CubeMapType cube_map_type_;
};


// Ԥ���˵Ļ�����ͼ
class SpecularCubeMap
{
public:
	SpecularCubeMap(const std::string& file_folder, CubeMap::CubeMapType cube_map_type);

public:
	static constexpr int max_mipmap_level_ = 10;
	CubeMap* prefilter_maps_[max_mipmap_level_];
};



// IBL ��պУ���պл����skybox��������ͼ��irradiance��������ͼ����specular��������ͼ
class IBLMap
{
public:
	IBLMap() = default;
	IBLMap(const std::string& skybox_path);

public:
	CubeMap* skybox_cubemap_;
	CubeMap* irradiance_cubemap_;
	SpecularCubeMap* specular_cubemap_;
	Texture* brdf_lut_;

	std::string skybox_name_;
	std::string skybox_folder_;
};
