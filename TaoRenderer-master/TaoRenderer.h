#pragma once

#include <functional>
#include <cstdint>

#include "Shader.h"
#include "Window.h"
#include "DataBuffer.h"


/*
	文件内容：
	-渲染器类的声明
	-最近一次修改日期：2024.11.26
*/

class TaoRenderer
{
	// 一些结构体
public:
	// 顶点结构体
	struct Vertex {
		bool has_transformed;			// 是否已经完成了顶点变换
		Varyings context;			    // 上下文
		float w_reciprocal;				// w 的倒数
		Vec4f ws_position;				// 世界空间坐标
		Vec4f cs_position;				// 裁剪空间坐标	范围[-1,1]
		Vec2f screen_position_f;		// 屏幕坐标		范围x~[0.5, frame_buffer_width_+ 0.5] y~[0.5, frame_buffer_height_+ 0.5]
		Vec2i screen_position_i;		// 整数屏幕坐标  范围x~[0, frame_buffer_width_], y~[0, frame_buffer_height_]

		Vertex() {
			has_transformed = false;
		}

		~Vertex() = default;

	};
	// 边缘方程
	struct EdgeEquation
	{
		float a, b, c;		// 边缘方程的系数
		bool is_top_left;	// 这条边缘是否属于左上
		float origin;		// 边缘左下角的起始值
		float w_reciprocal; // 顶点w分量的倒数

		void Initialize(const Vec2i& p0, const Vec2i& p1, const Vec2i& bottom_left_point, float w_reciprocal) {
			// 详见RTR4 方程23.2
			a = -(p1.y - p0.y);
			b = p1.x - p0.x;
			c = -a * p0.x - b * p0.y;

			origin = a * bottom_left_point.x + b * bottom_left_point.y + c;

			/*
			 * 当(x, y)位于三角形内部和边缘上时，有e>=0
			 *
			 * 当边或者顶点恰好经过像素中心时，使用top-left规则来判断像素归属
			 * 当像素中心位于上边缘或者和左边缘的时候，认为这个像素位于三角形内部
			 * 上边缘：边缘方程中的a=0, b<0
			 * 左边缘：边缘方程中的a>0
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


	// 裁剪空间下的裁剪平面
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
	int frame_buffer_width_;	// frame buffer 宽度
	int frame_buffer_height_;	// frame buffer 高度
	int shadow_buffer_width_;	// shadow buffer 宽度
	int shadow_buffer_height_;	// shadow buffer 高度
	ColorRGBA color_foreground_;	// 前景色，用于画线
	ColorRGBA color_background_;	// 背景色，用于Clear

	bool render_frame_;			// 是否绘制线框
	bool render_pixel_;			// 是否填充像素
	bool render_shadow_;		// 是否渲染阴影


	// 渲染中使用的临时数据
	Vertex vertex_[3];
	Vertex* clip_vertex_[4];	// 经过近平面clip之后的顶点

	DataBuffer* data_buffer_;	// 数据缓冲
	Window* window_;			// 窗口

	ShaderType current_shader_type_; //当前使用的shader类型

private:
	// 边缘方程
	EdgeEquation edge_equation_[3];
	// 当前顶点的varyings
	Varyings current_varyings_;
	// object VS & PS
	VertexShader vertex_shader_;
	PixelShader pixel_shader_;
	// shadow VS
	ShadowShader* shadow_shader_;
	// skybox VS & PS
	SkyBoxShader* skybox_shader_;


	bool is_rendering_shadowMap; // 是否正在渲染阴影贴图
	bool is_rendering_skybox;    // 是否正在渲染天空盒

public:
	TaoRenderer() = default;
	
	TaoRenderer(const int width, const int height);		// 利用frame buffer的长宽来初始化渲染器

	~TaoRenderer() { CleanUp(); }

public:
	// 初始化 frame buffer 渲染前需要调用
	void Init(int width, int height);

	// 释放资源
	void CleanUp();

	// 清空frame buffer, 并且传入clear_flag来作为参数
	void ClearFrameBuffer(bool clear_color, bool clear_depth) const;

	// 设置顶点着色器
	void SetVertexShader(const VertexShader& vs) { vertex_shader_ = vs; }
	// 设置片元着色器
	void SetPixelShader(const PixelShader& ps) { pixel_shader_ = ps; }
	// 设置阴影顶点着色器
	void SetShadowShader(ShadowShader* ss) { shadow_shader_ = ss; }
	// 设置天空盒顶点着色器
	void SetSkyboxShader(SkyBoxShader* ss) { skybox_shader_ = ss; }


	// 设置背景色
	void SetBackgroundColor(const Vec4f& color) { color_background_ = color; }
	// 设置前景色
	void SetForegroundColor(const Vec4f& color) { color_foreground_ = color; }

	// 设置渲染状态
	void SetRenderState(const bool frame, const  bool pixel) {
		render_frame_ = frame; // 是否显示线框图
		render_pixel_ = pixel; // 是否填充三角形
	}

	// 设置缓冲(一维四通道)
	void SetBuffer(uint8_t* buffer, const int x, const  int  y, const  Vec4f& color) const;

	// 在颜色缓冲中绘制线段
	void DrawLine(const int x1, const int y1, const int x2, const int y2) const;
	// 在颜色缓冲中绘制点
	void SetPixel(const int x, const int y, const Vec4f& cc) const;
	void SetPixel(const int x, const int y, const Vec3f& cc) const;

	// 绘制天空盒
	void DrawSkybox();

	// 绘制网格体
	void DrawMesh();

	// 绘制阴影贴图
	void DrawShadowMap();


private:
	// 绘制线
	void DrawLine(int x1, int y1, int x2, int y2, const Vec4f& color) const;
	// 绘制线框
	void DrawWireFrame(Vertex* vertex[3]) const;
	// 光栅化三角形
	void RasterizeTriangle(Vertex* vertex[3]);
	// 用平面裁剪三角形
	int ClipWithPlane(ClipPlane clip_plane, Vertex vertex[3]);
	// 获取直线与平面的交点
	Vertex& GetIntersectVertex(ClipPlane clip_plane, Vertex& v1, Vertex& v2);
};