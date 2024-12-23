#pragma once
#include "Math.h"


/*
	文件内容：
	-贴图类的声明
	-单张纹理贴图，立方体贴图，预过滤的环境贴图，IBL贴图等等
	-最近一次修改日期：2024.12.24
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

// 单张纹理贴图
class Texture
{
public:
	int texture_width_;				// 纹理宽度
	int texture_height_;			// 纹理高度
	int texture_channels_;			// 纹理通道数 （厚度）

	bool has_data_;					// 是否成功加载贴图, 是否包含数据
	unsigned char* texture_data_;	// 实际的图像数据指针， 根据宽，高，通道数来读取图片

public:
	Texture(const std::string& file_name); // 读取文件路径来初始化贴图
	~Texture();      

	// 纹理采样
	Vec4f Sample2D(float u, float v) const;	// 分别根据u和v采样
	Vec4f Sample2D(Vec2f uv) const;			// 根据uv向量采样
private:
	ColorRGBA GetPixelColor(int x, int y) const;		// 获取frame中某一像素的颜色
	ColorRGBA SampleBilinear(float x, float y) const;	// 双线性采样
	// 颜色双线性插值，确定四个角的颜色以及插值坐标，来确定这个矩形区域内部某一点的颜色
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


// 预过滤的环境贴图
class SpecularCubeMap
{
public:
	SpecularCubeMap(const std::string& file_folder, CubeMap::CubeMapType cube_map_type);

public:
	static constexpr int max_mipmap_level_ = 10;
	CubeMap* prefilter_maps_[max_mipmap_level_];
};



// IBL 天空盒，天空盒会包括skybox立方体贴图，irradiance立方体贴图，和specular立方体贴图
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
