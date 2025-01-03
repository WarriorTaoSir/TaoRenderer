#include "Model.h"
#include "utility.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using std::cout;
using std::endl;

/*
	文件内容：
	-模型类的定义
	-最近一次修改日期：2024.11.18
*/

Model::Model(const string& model_path, const Mat4x4f& model_matrix)
{
	// 加载obj模型
	LoadModelByTinyObj(model_path);

	// 获取模型路径与文件名
	model_folder_ = GetFileFolder(model_path);
	model_name_ = GetFileNameWithoutExtension(model_path);

	// 加载basecolor贴图，一个模型至少有一个basecolor贴图
	const string basecolor_file_name = GetFilePathByFileName(model_folder_, Model::GetTextureType(kTextureTypeBaseColor));
	string texture_format = GetFileExtension(basecolor_file_name);

	// 加载其他纹理
	base_color_map_ = new Texture(GetTextureFileName(model_folder_, model_name_, kTextureTypeBaseColor, texture_format));
	normal_map_ = new Texture(GetTextureFileName(model_folder_, model_name_, kTextureTypeNormal, texture_format));
	roughness_map_ = new Texture(GetTextureFileName(model_folder_, model_name_, kTextureTypeRoughness, texture_format));
	metallic_map_ = new Texture(GetTextureFileName(model_folder_, model_name_, kTextureTypeMetallic, texture_format));
	occlusion_map_ = new Texture(GetTextureFileName(model_folder_, model_name_, kTextureTypeOcclusion, texture_format));
	emission_map_ = new Texture(GetTextureFileName(model_folder_, model_name_, kTextureTypeEmission, texture_format));
	
	model_matrix_ = model_matrix;
}

void Model::LoadModel(const std::string& model_name)
{	
	// 模型各个顶点的位置，uv，法线，切线
	vector<Vec3f> positions;
	vector<Vec2f> texcoords;
	vector<Vec3f> normals;
	vector<Vec4f> tangents;

	vector<int> position_indices, texcoord_indices, normal_indices;

	constexpr int LINE_SIZE = 256; // 文件中一行的上限是256个字符
	char line[LINE_SIZE];
	FILE* file = fopen(model_name.c_str(), "rb"); // 以二进制方式读取文件
	while (true) {
		if (fgets(line, LINE_SIZE, file) == nullptr) {
			break;
		}
		else if (strncmp(line, "v ", 2) == 0) {		// 判断该行是否以"v "开头，即顶点位置
			Vec3f position;
			sscanf(line, "v %f %f %f", & position.x, & position.y, & position.z);
			positions.push_back(position);
		}
		else if (strncmp(line, "vt ", 3) == 0) {	// 判断该行是否以"vt "开头，即uv坐标位置
			Vec2f texcoord;
			sscanf(line, "vt %f %f", &texcoord.x, &texcoord.y);
			texcoords.push_back(texcoord);
		}
		else if (strncmp(line, "vn ", 3) == 0) {	// 判断该行是否以"vn "开头，即法线坐标位置
			Vec3f normal;
			sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
			normals.push_back(normal);
		}
		else if (strncmp(line, "f ", 2) == 0) {		// 判断该行是否以"f "开头，即面信息
			int i;
			int pos_indices[3], uv_indices[3], n_indices[3];
			sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&pos_indices[0], &uv_indices[0], &n_indices[0],
				&pos_indices[1], &uv_indices[1], &n_indices[1],
				&pos_indices[2], &uv_indices[2], &n_indices[2]);

			// 将第三角形的第一个顶点的各个信息输出
			cout << pos_indices[0] << " ";
			cout << uv_indices[0] << " ";
			cout << n_indices[0] << endl;

			for (i = 0; i < 3; i++) {
				position_indices.push_back(pos_indices[i] - 1);
				texcoord_indices.push_back(uv_indices[i] - 1);
				normal_indices.push_back(n_indices[i] - 1);
			}
		}
		else if (strncmp(line, "# ext.tangent ", 14) == 0) {
			Vec4f tangent;
			sscanf(line, "# ext.tangent %f %f %f %f", &tangent.x, &tangent.y, &tangent.z, &tangent.w);
			tangents.push_back(tangent);
		}

		face_number_ = position_indices.size() / 3;	// 模型中所有面的数量
		vertex_number_ = position_indices.size();	// 模型中所有顶点数量

		// 对于每一个顶点，将vector中的值存入成员变量attribute结构体中
		for (int i = 0; i < position_indices.size(); i++) {
			int position_index = position_indices[i];
			int texcoord_index = texcoord_indices[i];
			int normal_index = normal_indices[i];

			Attributes attribute{};
			attribute.position_os = positions[position_index];
			attribute.normal_os = normals[normal_index];

			// 部分uv值大于1，先将uv值转换到[0-1]区间中
			// uv坐标的原点位于左下角，贴图数据的原点位于左上角，因此需要再v轴上反向
			float u = texcoords[texcoord_index].u;
			float v = texcoords[texcoord_index].v;
			v = 1.0f - fmod(v, 1.0f);
			attribute.texcoord = { u,v };

			if (tangents.size() > 0)
			{
				attribute.tangent_os = tangents[position_index];
			}
			else 	
			{
				attribute.tangent_os = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
			}

			attributes_.push_back(attribute);
		}

		has_tangent_ = tangents.size() > 0;
	}
}

void Model::LoadModelByTinyObj(const std::string& model_name)
{
	// 加载切线
	std::pmr::vector<Vec4f> tangents;
	constexpr int LINE_SIZE = 256;
	char line[LINE_SIZE];
	FILE* file = fopen(model_name.c_str(), "rb");
	while (true) {
		int items;
		if (fgets(line, LINE_SIZE, file) == nullptr) {
			break;
		}
		if (strncmp(line, "# ext.tangent ", 14) == 0) {  /* tangent */
			Vec4f tangent;
			items = sscanf(line, "# ext.tangent %f %f %f %f",
				&tangent.x, &tangent.y, &tangent.z, &tangent.w);
			tangents.push_back(tangent);
		}
	}
	has_tangent_ = tangents.size() > 0;

	{
		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		if (!LoadObj(&attributes, &shapes, &materials, &warn, &err, model_name.c_str())) {
			throw std::runtime_error(warn + err);
		}
		vertex_number_ = 0;
		face_number_ = 0;
		for (const auto& shape : shapes) {
			for (size_t face_id = 0; face_id < shape.mesh.indices.size();) {
				for (size_t i = 0; i < 3; i++) {
					Attributes attribute{};
					auto& index = shape.mesh.indices[face_id + i];
					attribute.position_os = {
						attributes.vertices[3 * index.vertex_index + 0],
						attributes.vertices[3 * index.vertex_index + 1],
						attributes.vertices[3 * index.vertex_index + 2]
					};
					attribute.texcoord = {
						attributes.texcoords[2 * index.texcoord_index + 0],
						1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
					};

					float u = attributes.texcoords[2 * index.texcoord_index + 0];
					float v = attributes.texcoords[2 * index.texcoord_index + 1];
					v = 1.0f - fmod(v, 1.0f);
					attribute.texcoord = { u,v };
					attribute.normal_os = {
						attributes.normals[3 * index.normal_index + 0],
						attributes.normals[3 * index.normal_index + 1],
						attributes.normals[3 * index.normal_index + 2],
					};

					if (tangents.size() > 0)
					{
						attribute.tangent_os = tangents[index.vertex_index];
					}
					else
					{
						attribute.tangent_os = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
					}
					attributes_.push_back(attribute);
				}
				face_id += 3;
				vertex_number_ += 3;
				face_number_ += 1;
			}
		}
	}
}

Model::Model(vector<Vec3f>& vertex, const vector<int>& index)
{
	for (int i : index)
	{
		Attributes attribute{};
		attribute.position_os = {
			vertex[i]
		};
		attributes_.push_back(attribute);
	}
}

string Model::PrintModelInfo()
{
	const string model_message =
		"vertex count: " + std::to_string(vertex_number_);
		//"  face count: " + std::to_string(face_number_) + "\n";

	return model_message;
}

Model::~Model()
{
	delete base_color_map_;
	delete normal_map_;
	delete roughness_map_;
	delete metallic_map_;
	delete occlusion_map_;
	delete emission_map_;

	attributes_.clear();
}

std::string Model::GetTextureType(const TextureType texture_type)
{

	switch (texture_type)
	{
	case kTextureTypeBaseColor:			return "basecolor";
	case kTextureTypeNormal:			return "normal";
	case kTextureTypeRoughness:			return "roughness";
	case kTextureTypeMetallic:			return "metallic";
	case kTextureTypeOcclusion:			return "occlusion";
	case kTextureTypeEmission:			return "emission";

	default:							return "unknown";
	}
}

// 获取贴图文件名
string Model::GetTextureFileName(const std::string& file_path, const std::string& file_name, const TextureType texture_type, const std::string& texture_format)
{
	const std::string texture_name = file_path + "/" + file_name + "_" + GetTextureType(texture_type) + texture_format;

	return texture_name;
}