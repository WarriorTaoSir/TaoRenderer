#pragma once

#include <vector>
#include "Math.h"
#include "Texture.h"

using std::string;
using std::vector;

/*
	文件内容：
	-模型类的声明
	-最近一次修改日期：2024.11.18
*/ 

// 模型上每一个顶点的属性  os == object space
struct Attributes {
	Vec3f position_os;
	Vec2f texcoord;
	Vec3f normal_os;
	Vec4f tangent_os;
};

class Model {
public:
	std::vector<Attributes> attributes_;	// 顶点属性
	Mat4x4f model_matrix_;					// 模型矩阵

	std::string model_folder_, model_name_;	// 模型存放的文件夹与模型名

	int vertex_number_, face_number_;		// 顶点数量，面数量

	Texture* base_color_map_;				// 基础颜色贴图
	Texture* normal_map_;					// 法线贴图
	//Texture* roughness_map_;				// 粗糙度贴图
	//Texture* metallic_map_;				// 金属度贴图
	//Texture* occlusion_map_;				// 自遮蔽贴图
	//Texture* emission_map_;				// 自发光贴图

	bool has_tangent_;
public:
	Model(const string& model_path, const Mat4x4f& model_matrix);	// 通过文件路径来导入model
	Model(vector<Vec3f>& vertex, const vector<int>& index);			// 用于程序化生成模型
	string PrintModelInfo();
	~Model();

private:
	void LoadModel(const string& model_name);
	void LoadModelByTinyObj(const string& model_name);

public:
	// 贴图类型转string
	static string GetTextureType(TextureType texture_type);
	// texture 文件名
	static std::string GetTextureFileName(const std::string& file_path, const std::string& file_name, TextureType texture_type, const std::string& texture_format);
};