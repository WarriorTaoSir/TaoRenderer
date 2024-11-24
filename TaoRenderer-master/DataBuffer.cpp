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
	model_ = nullptr;
}

// �ͷ��ڹ��캯����new���ڴ�
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