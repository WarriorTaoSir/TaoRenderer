#pragma once

#include "Texture.h"
#include "Model.h"
#include "Window.h"
/*
	文件内容：
	-场景类的声明
	-最近一次修改日期：2024.12.24
*/

inline std::vector<std::string> model_paths =
{
	"../assets/helmet/helmet.obj",
	//"../assets/plane/plane.obj"
};

inline std::vector<Mat4x4f> model_matrices =
{
	matrix_set_rotate(1.0f, 0.0f, 0.0f, -kPi * 0.5f) * matrix_set_scale(1, 1, 1),
	//matrix_set_rotate(0.0f, 1.0f, 0.0f, -kPi * 0.5f)* matrix_set_translate(0.0f,-1.5f,0.0f)* matrix_set_scale(3.0f, 3.0f,3.0f)
};

inline std::vector<std::string> skybox_paths =
{
	"../assets/kloofendal_48d_partly_cloudy_puresky/kloofendal_48d_partly_cloudy_puresky.hdr",
	"../assets/spruit_sunrise/spruit_sunrise.hdr",
	"../assets/brown_photostudio/brown_photostudio.hdr",
	"../assets/autumn_forest_04/autumn_forest_04.hdr"
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
	std::vector< IBLMap* >iblmaps_;
	IBLMap* current_iblmap_;
	int total_iblmap_count_;
	int current_iblmap_index_;

	Window* window_;

public:
	Scene();
	~Scene();
	
	void LoadNextModel();
	void LoadPrevModel();

	void LoadNextIBLMap();
	void LoadPrevIBLMap();
};