#pragma once

#include <vector>
#include "Math.h"
#include "Texture.h"

using std::string;
using std::vector;

/*
	�ļ����ݣ�
	-ģ���������
	-���һ���޸����ڣ�2024.11.18
*/ 

// ģ����ÿһ�����������  os == object space
struct Attributes {
	Vec3f position_os;
	Vec2f texcoord;
	Vec3f normal_os;
	Vec4f tangent_os;
};

class Model {
public:
	std::vector<Attributes> attributes_;	// ��������
	Mat4x4f model_matrix_;					// ģ�;���

	std::string model_folder_, model_name_;	// ģ�ʹ�ŵ��ļ�����ģ����

	int vertex_number_, face_number_;		// ����������������

	Texture* base_color_map_;				// ������ɫ��ͼ
	Texture* normal_map_;					// ������ͼ
	//Texture* roughness_map_;				// �ֲڶ���ͼ
	//Texture* metallic_map_;				// ��������ͼ
	//Texture* occlusion_map_;				// ���ڱ���ͼ
	//Texture* emission_map_;				// �Է�����ͼ

	bool has_tangent_;
public:
	Model(const string& model_path, const Mat4x4f& model_matrix);	// ͨ���ļ�·��������model
	Model(vector<Vec3f>& vertex, const vector<int>& index);			// ���ڳ�������ģ��
	string PrintModelInfo();
	~Model();

private:
	void LoadModel(const string& model_name);
	void LoadModelByTinyObj(const string& model_name);

public:
	// ��ͼ����תstring
	static string GetTextureType(TextureType texture_type);
	// texture �ļ���
	static std::string GetTextureFileName(const std::string& file_path, const std::string& file_name, TextureType texture_type, const std::string& texture_format);
};