#pragma once

#include "Math.h"
#include "Model.h"
#include "Shader.h"
#include "Scene.h"
#include "Window.h"
#include "DataBuffer.h"


/*
	�ļ����ݣ�
	-��Ⱦ���������
	-���һ���޸����ڣ�2024.11.21
*/

class TaoRenderer
{
	// һЩ�ṹ��
public:
	// ����ṹ��
	struct Vertex {
		bool has_transformed;			// �Ƿ��Ѿ�����˶���任
		Varyings context;			    // ������
		float w_reciprocal;				// w �ĵ���
		Vec4f position;					// �ü��ռ�����	��Χ[-1,1]
		Vec2f screen_position_f;		// ��Ļ����		��Χx~[0.5, frame_buffer_width_+ 0.5] y~[0.5, frame_buffer_height_+ 0.5]
		Vec2i screen_position_i;		// ������Ļ����  ��Χx~[0, frame_buffer_width_], y~[0, frame_buffer_height_]

		Vertex() {
			has_transformed = false;
		}

		~Vertex() = default;

	};
	// ��������֮������Բ�ֵ�������Զ���������ԵĲ�ֵ����˷���Renderer��������ǹ�������
	static Vertex& VertexLerp(Vertex& vertex_p0, Vertex& vertex_p1, float ratio);

	// �ü��ռ��µĲü�ƽ��
	enum ClipPlane
	{
		W_Plane,
		X_RIGHT,
		X_LEFT,
		Y_TOP,
		Y_BOTTOM,
		Z_NEAR,
		Z_FAR
	};

public:
	uint8_t* color_buffer_;		// ��ɫ����
	float** depth_buffer_;		// ��Ȼ���

	int frame_buffer_width_;	// frame buffer ���
	int frame_buffer_height_;	// frame buffer �߶�
	ColorRGBA color_foreground_;	// ǰ��ɫ�����ڻ���
	ColorRGBA color_background_;	// ����ɫ������Clear

	bool render_frame_;			// �Ƿ�����߿�
	bool render_pixel_;			// �Ƿ��������

	// ��Ⱦ��ʹ�õ���ʱ����
	Vertex vertex_[3];
	Vertex* clip_vertex_[4];	// ������ƽ��clip֮��Ķ���

	DataBuffer* data_buffer_;	// ���ݻ���
	Window* window_;			// ����

private:
	// VS �� PS
	VertexShader vertex_shader_;
	PixelShader pixel_shader_;

public:
	TaoRenderer() = default;
	
	TaoRenderer(const int width, const int height);		// ����frame buffer�ĳ�������ʼ����Ⱦ��

	~TaoRenderer() { CleanUp(); }

public:
	// ��ʼ�� frame buffer ��Ⱦǰ��Ҫ����
	void Init(int width, int height);

	// �ͷ���Դ
	void CleanUp();

	// ���frame buffer, ���Ҵ���clear_flag����Ϊ����
	void ClearFrameBuffer(bool clear_color, bool clear_depth) const;

	// ���ö�����ɫ��
	void SetVertexShader(const VertexShader& vs) { vertex_shader_ = vs; }
	// ����ƬԪ��ɫ��
	void SetPixelShader(const PixelShader& ps) { pixel_shader_ = ps; }

	// ���ñ���ɫ
	void SetBackgroundColor(const Vec4f& color) { color_background_ = color; }
	// ����ǰ��ɫ
	void SetForegroundColor(const Vec4f& color) { color_foreground_ = color; }

	// ������Ⱦ״̬
	void SetRenderState(const bool frame, const  bool pixel) {
		render_frame_ = frame; // �Ƿ���ʾ�߿�ͼ
		render_pixel_ = pixel; // �Ƿ����������
	}

	// ���û���(һά��ͨ��)
	void SetBuffer(uint8_t* buffer, const int x, const  int  y, const  Vec4f& color) const;

	// ����ɫ�����л����߶�
	void DrawLine(const int x1, const int y1, const int x2, const int y2) const {
		if (color_buffer_) DrawLine(x1, y1, x2, y2, color_foreground_);
	}
	// ����ɫ�����л��Ƶ�
	void SetPixel(const int x, const int y, const Vec4f& cc) const { SetBuffer(color_buffer_, x, y, cc); }
	void SetPixel(const int x, const int y, const Vec3f& cc)const { SetBuffer(color_buffer_, x, y, cc.xyz1()); }

	// TODO��������պ�

	// ����������
	void DrawMesh();


private:
	// ������
	void DrawLine(int x1, int y1, int x2, int y2, const Vec4f& color) const;
	// �����߿�
	void DrawWireFrame(Vertex* vertex[3]) const;
	// ��դ��������
	void RasterizeTriangle(Vertex* vertex[3]);
	// ��ƽ��ü�������
	int ClipWithPlane(ClipPlane clip_plane, Vertex vertex[3]);
	// ��ȡֱ����ƽ��Ľ���
	Vertex& GetIntersectVertex(ClipPlane clip_plane, Vertex& v1, Vertex& v2);

};