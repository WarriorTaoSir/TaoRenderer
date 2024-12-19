#pragma once

#include "Model.h"
#include "Shader.h"

/*
	文件内容：
	-场景类的声明
	-最近一次修改日期：2024.11.21
*/

inline std::vector<std::string> model_paths =
{
	"../assets/helmet/helmet.obj",
	"../assets/plane/plane.obj"
};

inline std::vector<Mat4x4f> model_matrices =
{
	matrix_set_rotate(1.0f, 0.0f, 0.0f, -kPi * 0.5f) * matrix_set_scale(1, 1, 1),
	matrix_set_rotate(0.0f, -1.0f, 0.0f, -kPi * 0.5f) * matrix_set_translate(0.0f,-2.0f,0.0f) * matrix_set_scale(3.0f, 3.0f,3.0f)
};

class Scene
{
public:
	// 模型列表
	std::vector< Model* >models_;
	Model* current_model_;
	int total_model_count_;
	int current_model_index_;

	// IBL列表
	//std::vector< IBLMap* >iblmaps_;
	//IBLMap* current_iblmap_;
	//int total_iblmap_count_;
	//int current_iblmap_index_;

	Window* window_;
	ShaderType current_shader_type_;
public:
	Scene();
	~Scene();
	
	//void HandleKeyEvents(PBRShader* pbr_shader, BlinnPhongShader*);  //负责切换shader
	
	void LoadNextModel();
	void LoadPrevModel();

	//void LoadNextIBLMap();
	//void LoadPrevIBLMap();
};