#pragma once
#include "Math.h"


/*
	�ļ����ݣ�
	-��ͼ�������
	-����������ͼ����������ͼ��Ԥ���˵Ļ�����ͼ��IBL��ͼ�ȵ�
	-���һ���޸����ڣ�2024.11.18
*/

enum TextureType
{
	kTextureTypeBaseColor,
	kTextureTypeNormal,
	kTextureTypeRoughness
};

// ����������ͼ
class Texture
{
public:
	int texture_width_;				// ������
	int texture_height_;			// ����߶�
	int texture_channels_;			// ����ͨ���� ����ȣ�

	bool has_data_;					// �Ƿ�ɹ�������ͼ, �Ƿ��������
	unsigned char* texture_data_;	// ʵ�ʵ�ͼ������ָ�룬 ���ݿ��ߣ�ͨ��������ȡͼƬ

public:
	Texture(const std::string& file_name); // ��ȡ�ļ�·������ʼ����ͼ
	~Texture();      

	// �������
	Vec4f Sample2D(float u, float v) const;	// �ֱ����u��v����
	Vec4f Sample2D(Vec2f uv) const;			// ����uv��������
private:
	ColorRGBA GetPixelColor(int x, int y) const;		// ��ȡframe��ĳһ���ص���ɫ
	ColorRGBA SampleBilinear(float x, float y) const;	// ˫���Բ���
	// ��ɫ˫���Բ�ֵ��ȷ���ĸ��ǵ���ɫ�Լ���ֵ���꣬��ȷ��������������ڲ�ĳһ�����ɫ
	static ColorRGBA BilinearInterpolation(const ColorRGBA& color00, const ColorRGBA& color01, const ColorRGBA& color10, const ColorRGBA& color11, float t_x, float t_y);
};

