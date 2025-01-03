#include "Texture.h"
#include "utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


/*
	文件内容：
	-贴图类的成员函数的定义
	-最近一次修改日期：2024.12.25
*/

#pragma region Texture
// 以文件路径字符串为参数的构造函数
Texture::Texture(const string& file_name) 
{	

	texture_data_ = stbi_load(file_name.c_str(), &texture_width_, &texture_height_, &texture_channels_, 4);
	has_data_ = texture_data_ == nullptr ? false : true;
}

// 析构函数，释放掉指针
Texture::~Texture()
{
	if (has_data_) stbi_image_free(texture_data_);
}

// 根据UV值来采样
ColorRGBA Texture::Sample2D(float u, float v) const
{
	if (!has_data_) return { 1.0f }; // 单参构造函数

	u = fmod(u, 1);
	v = fmod(v, 1);

	return SampleBilinear(u * texture_width_, v * texture_height_);
}

// 根据UV值来采样
ColorRGBA Texture::Sample2D(Vec2f uv) const
{
	uv.x = fmod(uv.x, 1);
	uv.y = fmod(uv.y, 1);

	return Sample2D(uv.x, uv.y);
}

// 获取x,y位置像素的颜色
ColorRGBA Texture::GetPixelColor(int x, int y) const {
	ColorRGBA color(1.0f);
	x = Between(0, texture_width_ - 1, x);
	y = Between(0, texture_height_ - 1, y);
	if (x >= 0 && x < texture_width_ &&
		y >= 0 && y < texture_height_) {
		const uint8_t* pixel_offset = texture_data_ + (x + y * texture_width_) * 4;
		color.r = pixel_offset[0] / 255.0f;
		color.g = pixel_offset[1] / 255.0f;
		color.b = pixel_offset[2] / 255.0f;
		color.a = pixel_offset[3] / 255.0f;
	}
	return color;
}

// 双线性采样
ColorRGBA Texture::SampleBilinear(const float x, const float y) const
{
	const auto x1 = static_cast<int>(floor(x));
	const auto y1 = static_cast<int>(floor(y));

	const auto x2 = static_cast<int> (ceil(x));
	const auto y2 = static_cast<int> (ceil(y));

	const float t_x = x - x1;
	const float t_y = y - y1;

	const ColorRGBA color00 = GetPixelColor(x1, y1);
	const ColorRGBA color01 = GetPixelColor(x2, y1);
	const ColorRGBA color10 = GetPixelColor(x1, y2);
	const ColorRGBA color11 = GetPixelColor(x2, y2);

	return BilinearInterpolation(color00, color01, color10, color11, t_x, t_y);
}

// 双线性插值
ColorRGBA Texture::BilinearInterpolation(const ColorRGBA& color00, const ColorRGBA& color01, const ColorRGBA& color10, const ColorRGBA& color11, float t_x, float t_y)
{	
	const ColorRGBA color0 = vector_lerp(color00, color01, t_x);
	const ColorRGBA color1 = vector_lerp(color10, color11, t_x);
	ColorRGBA color = vector_lerp(color0, color1, t_y);

	return color;
}

#pragma endregion

#pragma region Environment Map

CubeMap::CubeMap(const std::string& file_folder, CubeMapType cube_map_type, int mipmap_level)
{
	cube_map_type_ = cube_map_type;

	switch (cube_map_type_)
	{
	case kSkybox:
		cubemap_[0] = new Texture(file_folder + "m0_px.hdr");
		cubemap_[1] = new Texture(file_folder + "m0_nx.hdr");
		cubemap_[2] = new Texture(file_folder + "m0_py.hdr");
		cubemap_[3] = new Texture(file_folder + "m0_ny.hdr");
		cubemap_[4] = new Texture(file_folder + "m0_pz.hdr");
		cubemap_[5] = new Texture(file_folder + "m0_nz.hdr");
		break;
	case kIrradianceMap:
		cubemap_[0] = new Texture(file_folder + "i_px.hdr");
		cubemap_[1] = new Texture(file_folder + "i_nx.hdr");
		cubemap_[2] = new Texture(file_folder + "i_py.hdr");
		cubemap_[3] = new Texture(file_folder + "i_ny.hdr");
		cubemap_[4] = new Texture(file_folder + "i_pz.hdr");
		cubemap_[5] = new Texture(file_folder + "i_nz.hdr");
		break;
	case kSpecularMap:
		cubemap_[0] = new Texture(file_folder + "m" + std::to_string(mipmap_level) + "_px.hdr");
		cubemap_[1] = new Texture(file_folder + "m" + std::to_string(mipmap_level) + "_nx.hdr");
		cubemap_[2] = new Texture(file_folder + "m" + std::to_string(mipmap_level) + "_py.hdr");
		cubemap_[3] = new Texture(file_folder + "m" + std::to_string(mipmap_level) + "_ny.hdr");
		cubemap_[4] = new Texture(file_folder + "m" + std::to_string(mipmap_level) + "_pz.hdr");
		cubemap_[5] = new Texture(file_folder + "m" + std::to_string(mipmap_level) + "_nz.hdr");
		break;
	default:;
	}

}

CubeMap::~CubeMap()
{
	for (size_t i = 0; i < 6; i++)
	{
		delete cubemap_[i];
	}
	delete[] cubemap_;
}

Vec3f CubeMap::Sample(Vec3f& direction) const
{
	const auto [face_id, uv] = CalculateCubeMapUV(direction);
	return  cubemap_[face_id]->Sample2D(uv).xyz();
}

// 详见链接章节3.7.5
// https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
CubeMap::CubeMapUV& CubeMap::CalculateCubeMapUV(Vec3f& direction)
{
	CubeMapUV cubemap_uv;
	float ma = 0, sc = 0, tc = 0;
	const Vec3f direction_abs = vector_abs(direction);

	if (direction_abs.x > direction_abs.y &&		// x轴为主轴
		direction_abs.x > direction_abs.z)
	{
		ma = direction_abs.x;
		if (direction.x > 0)					/* positive x */
		{
			cubemap_uv.face_id = 0;
			sc = -direction.z;
			tc = -direction.y;
		}
		else									/* negative x */
		{
			cubemap_uv.face_id = 1;
			sc = +direction.z;
			tc = -direction.y;
		}
	}
	else if (direction_abs.y > direction_abs.z)		// y轴为主轴
	{
		ma = direction_abs.y;
		if (direction.y > 0)					/* positive y */
		{
			cubemap_uv.face_id = 2;
			sc = +direction.x;
			tc = +direction.z;
		}
		else									/* negative y */
		{
			cubemap_uv.face_id = 3;
			sc = +direction.x;
			tc = -direction.z;
		}
	}
	else											// z轴为主轴
	{
		ma = direction_abs.z;
		if (direction.z > 0)					/* positive z */
		{
			cubemap_uv.face_id = 4;
			sc = +direction.x;
			tc = -direction.y;
		}
		else									/* negative z */
		{
			cubemap_uv.face_id = 5;
			sc = -direction.x;
			tc = -direction.y;
		}
	}

	cubemap_uv.uv.u = (sc / ma + 1.0f) / 2.0f;
	cubemap_uv.uv.v = (tc / ma + 1.0f) / 2.0f;

	return  cubemap_uv;
}

SpecularCubeMap::SpecularCubeMap(const std::string& file_folder, CubeMap::CubeMapType cube_map_type)
{
	for (size_t i = 0; i < max_mipmap_level_; i++)
	{
		prefilter_maps_[i] = new CubeMap(file_folder, CubeMap::kSpecularMap, i);
	}
}

IBLMap::IBLMap(const std::string& skybox_path)
{
	skybox_name_ = GetFileNameWithoutExtension(skybox_path);
	skybox_folder_ = GetFileFolder(skybox_path) + "/" + skybox_name_ + "/";
	// 检查并生成IBL
	if (!CheckFileExist(skybox_folder_ + "brdf_lut.hdr"))
	{
		GenerateCubeMap(skybox_path);
	}

	// 加载IBL资源
	skybox_cubemap_ = new CubeMap(skybox_folder_, CubeMap::kSkybox);
	irradiance_cubemap_ = new CubeMap(skybox_folder_, CubeMap::kIrradianceMap);
	specular_cubemap_ = new SpecularCubeMap(skybox_folder_, CubeMap::kSpecularMap);
	brdf_lut_ = new Texture(skybox_folder_ + "brdf_lut.hdr");
}

#pragma endregion
