#include "Scene.h"

/*
	�ļ����ݣ�
	-������Ķ���
	-���һ���޸����ڣ�2024.11.21
*/

Scene::Scene()
{
	// ����ģ��
	for (size_t i = 0; i < model_paths.size(); i++) 
	{	
		auto model = new Model(model_paths[i], model_matrices[i]);
		models_.push_back(model);
	}
	std::cout << "�Ѽ���" << model_paths.size() << "��ģ��" << std::endl;

	// ����IBL��Դ
	for (size_t i = 0; i < skybox_paths.size(); i++)
	{
		auto ibl_map = new IBLMap(skybox_paths[i]);
		iblmaps_.push_back(ibl_map);
	}

	total_model_count_ = models_.size();
	current_model_index_ = 0;
	current_model_ = models_[current_model_index_];

	total_iblmap_count_ = iblmaps_.size();
	current_iblmap_index_ = 0;
	current_iblmap_ = iblmaps_[current_iblmap_index_];

	window_ = Window::GetInstance();
	//window_->SetLogMessage("Shading Model", "Shading Model: PBR + IBL");
}

Scene::~Scene() {
	models_.clear();  // ���models list
}

void Scene::LoadNextModel()
{
	current_model_index_ = (current_model_index_ + 1) % total_model_count_;
	current_model_ = models_[current_model_index_];
}

void Scene::LoadPrevModel()
{
	current_model_index_ = (current_model_index_ - 1 + total_model_count_) % total_model_count_;
	current_model_ = models_[current_model_index_];
}

void Scene::LoadNextIBLMap()
{
	current_iblmap_index_ = (current_iblmap_index_ + 1) % total_iblmap_count_;
	current_iblmap_ = iblmaps_[current_iblmap_index_];
}

void Scene::LoadPrevIBLMap()
{
	current_iblmap_index_ = (current_iblmap_index_ - 1 + total_iblmap_count_) % total_iblmap_count_;
	current_iblmap_ = iblmaps_[current_iblmap_index_];
}


