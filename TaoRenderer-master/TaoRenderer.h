#pragma once

#include "Math.h"
#include "Model.h"
#include "Shader.h"
#include "Scene.h"
#include "Window.h"
#include "DataBuffer.h"


/*
	文件内容：
	-渲染器类的声明
	-最近一次修改日期：2024.11.21
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
		Vec4f position;					// 裁剪空间坐标	范围[-1,1]
		Vec2f screen_position_f;		// 屏幕坐标		范围x~[0.5, frame_buffer_width_+ 0.5] y~[0.5, frame_buffer_height_+ 0.5]
		Vec2i screen_position_i;		// 整数屏幕坐标  范围x~[0, frame_buffer_width_], y~[0, frame_buffer_height_]

		Vertex() {
			has_transformed = false;
		}

		~Vertex() = default;

	};
	// 两个顶点之间的线性插值，包括对顶点各个属性的插值，因此放在Renderer类里，而不是工具类里
	static Vertex& VertexLerp(Vertex& vertex_p0, Vertex& vertex_p1, float ratio);

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
	uint8_t* color_buffer_;		// 颜色缓存
	float** depth_buffer_;		// 深度缓存

	int frame_buffer_width_;	// frame buffer 宽度
	int frame_buffer_height_;	// frame buffer 高度
	ColorRGBA color_foreground_;	// 前景色，用于画线
	ColorRGBA color_background_;	// 背景色，用于Clear

	bool render_frame_;			// 是否绘制线框
	bool render_pixel_;			// 是否填充像素

	// 渲染中使用的临时数据
	Vertex vertex_[3];
	Vertex* clip_vertex_[4];	// 经过近平面clip之后的顶点

	DataBuffer* data_buffer_;	// 数据缓冲
	Window* window_;			// 窗口

private:
	// VS 与 PS
	VertexShader vertex_shader_;
	PixelShader pixel_shader_;

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
	void DrawLine(const int x1, const int y1, const int x2, const int y2) const {
		if (color_buffer_) DrawLine(x1, y1, x2, y2, color_foreground_);
	}
	// 在颜色缓冲中绘制点
	void SetPixel(const int x, const int y, const Vec4f& cc) const { SetBuffer(color_buffer_, x, y, cc); }
	void SetPixel(const int x, const int y, const Vec3f& cc)const { SetBuffer(color_buffer_, x, y, cc.xyz1()); }

	// TODO：绘制天空盒

	// 绘制网格体
	void DrawMesh();


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