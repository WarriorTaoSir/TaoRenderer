#pragma once

#include <functional>
#include <cstdint>

#include "Shader.h"
#include "Window.h"
#include "DataBuffer.h"


/*
	�ļ����ݣ�
	-��Ⱦ���������
	-���һ���޸����ڣ�2024.11.26
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
		Vec4f ws_position;				// ����ռ�����
		Vec4f cs_position;				// �ü��ռ�����	��Χ[-1,1]
		Vec2f screen_position_f;		// ��Ļ����		��Χx~[0.5, frame_buffer_width_+ 0.5] y~[0.5, frame_buffer_height_+ 0.5]
		Vec2i screen_position_i;		// ������Ļ����  ��Χx~[0, frame_buffer_width_], y~[0, frame_buffer_height_]

		Vertex() {
			has_transformed = false;
		}

		~Vertex() = default;

	};
	// ��Ե����
	struct EdgeEquation
	{
		float a, b, c;		// ��Ե���̵�ϵ��
		bool is_top_left;	// ������Ե�Ƿ���������
		float origin;		// ��Ե���½ǵ���ʼֵ
		float w_reciprocal; // ����w�����ĵ���

		void Initialize(const Vec2i& p0, const Vec2i& p1, const Vec2i& bottom_left_point, float w_reciprocal) {
			// ���RTR4 ����23.2
			a = -(p1.y - p0.y);
			b = p1.x - p0.x;
			c = -a * p0.x - b * p0.y;

			origin = a * bottom_left_point.x + b * bottom_left_point.y + c;

			/*
			 * ��(x, y)λ���������ڲ��ͱ�Ե��ʱ����e>=0
			 *
			 * ���߻��߶���ǡ�þ�����������ʱ��ʹ��top-left�������ж����ع���
			 * ����������λ���ϱ�Ե���ߺ����Ե��ʱ����Ϊ�������λ���������ڲ�
			 * �ϱ�Ե����Ե�����е�a=0, b<0
			 * ���Ե����Ե�����е�a>0
			 *
			 */
			is_top_left = (NearEqual(a, 0.0f, 0.00001f) && b > 0) || a > 0;

			this->w_reciprocal = w_reciprocal;
		}

		float Evaluate(const int x, const int y) const
		{
			return origin + x * a + y * b;
		}
	};


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
	int frame_buffer_width_;	// frame buffer ���
	int frame_buffer_height_;	// frame buffer �߶�
	int shadow_buffer_width_;	// shadow buffer ���
	int shadow_buffer_height_;	// shadow buffer �߶�
	ColorRGBA color_foreground_;	// ǰ��ɫ�����ڻ���
	ColorRGBA color_background_;	// ����ɫ������Clear

	bool render_frame_;			// �Ƿ�����߿�
	bool render_pixel_;			// �Ƿ��������
	bool render_shadow_;		// �Ƿ���Ⱦ��Ӱ


	// ��Ⱦ��ʹ�õ���ʱ����
	Vertex vertex_[3];
	Vertex* clip_vertex_[4];	// ������ƽ��clip֮��Ķ���

	DataBuffer* data_buffer_;	// ���ݻ���
	Window* window_;			// ����

	ShaderType current_shader_type_; //��ǰʹ�õ�shader����

private:
	// ��Ե����
	EdgeEquation edge_equation_[3];
	// ��ǰ�����varyings
	Varyings current_varyings_;
	// object VS & PS
	VertexShader vertex_shader_;
	PixelShader pixel_shader_;
	// shadow VS
	ShadowShader* shadow_shader_;
	// skybox VS & PS
	SkyBoxShader* skybox_shader_;


	bool is_rendering_shadowMap; // �Ƿ�������Ⱦ��Ӱ��ͼ
	bool is_rendering_skybox;    // �Ƿ�������Ⱦ��պ�

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
	// ������Ӱ������ɫ��
	void SetShadowShader(ShadowShader* ss) { shadow_shader_ = ss; }
	// ������պж�����ɫ��
	void SetSkyboxShader(SkyBoxShader* ss) { skybox_shader_ = ss; }


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
	void DrawLine(const int x1, const int y1, const int x2, const int y2) const;
	// ����ɫ�����л��Ƶ�
	void SetPixel(const int x, const int y, const Vec4f& cc) const;
	void SetPixel(const int x, const int y, const Vec3f& cc) const;

	// ������պ�
	void DrawSkybox();

	// ����������
	void DrawMesh();

	// ������Ӱ��ͼ
	void DrawShadowMap();


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