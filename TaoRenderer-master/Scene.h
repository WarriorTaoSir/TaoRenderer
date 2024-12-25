#pragma once

#include "Texture.h"
#include "Model.h"
#include "Window.h"
/*
	文件内容：
	-场景类的声明
	-最近一次修改日期：2024.12.26
*/

inline std::vector<std::string> model_paths =
{	
	//"../assets/helmet/helmet.obj",	// helmet
	//"../assets/plane/plane.obj"		// plane

	// Furina
	//"../assets/Furina/FurinaBody/FurinaBody.obj",			// body
	//"../assets/Furina/FurinaCloth/FurinaCloth.obj",			// cloth
	//"../assets/Furina/FurinaDecorate/FurinaDecorate.obj",	// decorate
	//"../assets/Furina/FurinaFace/FurinaFace.obj",			// face
	//"../assets/Furina/FurinaTailhair/FurinaTailhair.obj"	// tailhair

	// SantaTree
	"../assets/SantaTree/STDecoration/STDecoration.obj",		// Decoration
	"../assets/SantaTree/STLeaf/STLeaf.obj",					// Leaf
	"../assets/SantaTree/STTrunk/STTrunk.obj",					// Trunk
	"../assets/SantaTree/STGift/STGift.obj",					// Gift
	"../assets/SantaTree/STSnow/STSnow.obj",					// Snow
};

inline std::vector<Mat4x4f> model_matrices =
{
	//matrix_set_rotate(1.0f, 0.0f, 0.0f, -kPi * 0.5f) * matrix_set_scale(1, 1, 1),													// helmet
	// matrix_set_rotate(0.0f, 1.0f, 0.0f, -kPi * 0.5f)* matrix_set_translate(0.0f,-1.5f,0.0f)* matrix_set_scale(3.0f, 3.0f,3.0f)		// plane

	// Furina
	//matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f,-1.0f,0.0f)* matrix_set_scale(1, 1, 1),
	//matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f,-1.0f,0.0f)* matrix_set_scale(1, 1, 1),
	//matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f,-1.0f,0.0f)* matrix_set_scale(1, 1, 1),
	//matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f,-1.0f,0.0f)* matrix_set_scale(1, 1, 1),
	//matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f,-1.0f,0.0f)* matrix_set_scale(1, 1, 1)

	// SantaTree
	matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f, -2.5f, 0.0f)* matrix_set_scale(0.02f, 0.02f, 0.02f),
	matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f, -2.5f, 0.0f)* matrix_set_scale(0.02f, 0.02f, 0.02f),
	matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f, -2.5f, 0.0f)* matrix_set_scale(0.02f, 0.02f, 0.02f),
	matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f, -2.5f, 0.0f)* matrix_set_scale(0.02f, 0.02f, 0.02f),
	matrix_set_rotate(0.0f, 1.f, 0.0f, -kPi * 1.0f)* matrix_set_translate(0.0f, -2.5f, 0.0f)* matrix_set_scale(0.02f, 0.02f, 0.02f),
};

inline std::vector<std::string> skybox_paths =
{	
	//"../assets/blaubeuren_church_square/blaubeuren_church_square.hdr",
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