#include "DataBuffer.h"

/*
	�ļ����ݣ�
	-���ݻ�����Ķ���
	-������һЩ��Ⱦ�������ݵ�get��set����
	-����UniformBuffer��CurrentModel��
	-���һ���޸����ڣ�2024.11.24
*/

DataBuffer* DataBuffer::instance_ = nullptr;

// ����ģʽ
DataBuffer* DataBuffer::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new DataBuffer();
	}
	return instance_;
}



// ��ʼ������Buffer
DataBuffer::DataBuffer() {
	uniform_buffer_ = nullptr;
	attributes_ = new Attributes[3];
}

// �ͷ��ڹ��캯����new���ڴ�
DataBuffer::~DataBuffer() {
	delete[] attributes_;
}

 UniformBuffer* DataBuffer::GetUniformBuffer() const {
	return uniform_buffer_;
}

void DataBuffer::SetUniformBuffer(UniformBuffer* uniform_buffer) {
	uniform_buffer_ = uniform_buffer;
}

void DataBuffer::Init(int height, int width) {
	height_ = height;
	width_ = width;
	color_buffer_ = new uint8_t[height_ * width_ * 4]; // RGBA ��ͨ��

	depth_buffer_ = new float* [height_];  // ��άdepthbuffer��ʼ��
	shadow_buffer_ = new float* [height_];  // ��άshadowbuffer��ʼ��
	for (int i = 0; i < height_; i++) {
		depth_buffer_[i] = new float[width_];
		shadow_buffer_[i] = new float[width_];
	}

	current_model_index_ = 0;
}

void DataBuffer::CopyShadowBuffer() {
	for (int i = 0; i < height_; i++)
		std::memcpy(shadow_buffer_[i], depth_buffer_[i], sizeof(float) * width_);
}

void DataBuffer::MoveToNextModel() {
	current_model_index_++;
	if (current_model_index_ >= model_list_.size()) {
		current_model_index_ = 0;
	}
	uniform_buffer_->model_matrix = GetModelBeingRendered()->model_matrix_;
	uniform_buffer_->CalculateRestMatrix();
}

Model* DataBuffer::GetModelBeingRendered() {
	return model_list_[current_model_index_];
}