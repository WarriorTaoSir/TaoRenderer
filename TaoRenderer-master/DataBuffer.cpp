#include "DataBuffer.h"

/*
	文件内容：
	-数据缓冲类的定义
	-包含了一些渲染所需数据的get与set方法
	-例如UniformBuffer，CurrentModel，
	-最近一次修改日期：2024.11.24
*/

DataBuffer* DataBuffer::instance_ = nullptr;

// 单例模式
DataBuffer* DataBuffer::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new DataBuffer();
	}
	return instance_;
}



// 初始化数据Buffer
DataBuffer::DataBuffer() {
	uniform_buffer_ = nullptr;
	attributes_ = new Attributes[3];
	model_ = nullptr;
}

// 释放在构造函数中new的内存
DataBuffer::~DataBuffer() {
	delete[] attributes_;
}

 UniformBuffer* DataBuffer::GetUniformBuffer() const {
	return uniform_buffer_;
}

 Model* DataBuffer::GetModel() const {
	return model_;
}

void DataBuffer::SetUniformBuffer(UniformBuffer* uniform_buffer) {
	uniform_buffer_ = uniform_buffer;
}

void DataBuffer::SetModel(Model* model) {
	model_ = model;
}

void DataBuffer::Init(int height, int width) {
	height_ = height;
	width_ = width;
	color_buffer_ = new uint8_t[height_ * width_ * 4]; // RGBA 四通道

	depth_buffer_ = new float* [height_];  // 二维depthbuffer初始化
	shadow_buffer_ = new float* [height_];  // 二维shadowbuffer初始化
	for (int i = 0; i < height_; i++) {
		depth_buffer_[i] = new float[width_];
		shadow_buffer_[i] = new float[width_];
	}
}

void DataBuffer::CopyShadowBuffer() {
	for (int i = 0; i < height_; i++)
		std::memcpy(shadow_buffer_[i], depth_buffer_[i], sizeof(float) * width_);
}