#include "TaoRenderer.h"
#include "DataBuffer.h"
#include "Scene.h"


/*
	文件内容：
	-渲染器类的定义
	-最近一次修改日期：2024.11.24
*/

#pragma region 初始化与清除

TaoRenderer::TaoRenderer(const int width, const int height) {
	color_buffer_ = nullptr;
	depth_buffer_ = nullptr;
	render_frame_ = false;
	render_pixel_ = true;
	Init(width, height);
}

// 初始化渲染器
void TaoRenderer::Init(const int width, const int height)
{	
	// 设置一下渲染器宽和高
	frame_buffer_width_ = width;
	frame_buffer_height_ = height;

	// 初始化前景色和背景色
	color_foreground_ = Vec4f(0.0f);
	color_background_ = Vec4f(0.5f, 1.0f, 1.0f, 1.0f);

	color_buffer_ = new uint8_t[height * width * 4]; // RGBA 四通道
	depth_buffer_ = new float* [height];  // 二维depthbuffer初始化
	for (int i = 0; i < height; i++)
		depth_buffer_[i] = new float[width];

	// 设置DataBuffer
	dataBuffer_ = DataBuffer::GetInstance();

	ClearFrameBuffer(true, true);
}

void TaoRenderer::CleanUp() {
	delete [] color_buffer_;
	delete [] depth_buffer_;
}

void TaoRenderer::ClearFrameBuffer(bool clear_color_buffer, bool clear_depth_buffer) const
{
	// 如果清除颜色缓冲，而且color_buffer_不为空，将颜色缓冲设置为背景色
	if (clear_color_buffer && color_buffer_)
	{
		const ColorRGBA32Bit color_32_bit = vector_to_32bit_color(color_background_);

		for (int j = 0; j < frame_buffer_height_; j++) {
			const int offset = frame_buffer_width_ * (4 * j);
			for (int i = 0; i < frame_buffer_width_; i++)
			{
				const int base_address = offset + 4 * i;
				//32 bit位图存储顺序，从低到高依次为BGRA
				color_buffer_[base_address] = color_32_bit.b;
				color_buffer_[base_address + 1] = color_32_bit.g;
				color_buffer_[base_address + 2] = color_32_bit.r;
				color_buffer_[base_address + 3] = color_32_bit.a;
			}
		}
	}
	// 如果清除深度缓存，而且depth_buffer_不为空，将深度缓存设置为0.0f
	if (clear_depth_buffer && depth_buffer_) {
		for (int j = 0; j < frame_buffer_height_; j++) {
			for (int i = 0; i < frame_buffer_width_; i++)
				depth_buffer_[j][i] = 0.0f;
		}
	}
}
// 设置颜色缓冲
void TaoRenderer::SetBuffer(uint8_t* buffer, const int x, const int y, const Vec4f& color) const
{
	if (x < 0 || x>frame_buffer_width_ - 1) return;
	if (y < 0 || y>frame_buffer_height_ - 1) return;

	const ColorRGBA32Bit color_32_bit = vector_to_32bit_color(color);
	const int base_address = frame_buffer_width_ * (4 * y) + 4 * x;
	//32 bit位图存储顺序，从低到高依次为BGRA
	buffer[base_address] = color_32_bit.b;
	buffer[base_address + 1] = color_32_bit.g;
	buffer[base_address + 2] = color_32_bit.r;
	buffer[base_address + 3] = color_32_bit.a;
}



#pragma endregion

#pragma region 判断函数

#pragma endregion

#pragma region 计算函数

#pragma region

#pragma region 绘制函数
// 绘制网格体
void TaoRenderer::DrawMesh() 
{	
	Attributes* attributes = dataBuffer_->attributes_;
	Model* model = dataBuffer_->GetModel();
	// 按照每三个顶点的方式，循环该模型所有顶点
	for (size_t i = 0; i < model->attributes_.size(); i += 3)
	{
		// 把对应model的三点的attributes设置给对应shader的attributes
		for (int j = 0; j < 3; j++) {
			attributes[j].position_os = model->attributes_[i + j].position_os;
			attributes[j].texcoord = model->attributes_[i + j].texcoord;
			attributes[j].normal_os = model->attributes_[i + j].normal_os;
			attributes[j].tangent_os = model->attributes_[i + j].tangent_os;
		}
		// 顶点变换
		for (int k = 0; k < 3; k++) {
			vertex_[k].context.varying_float.clear();
			vertex_[k].context.varying_vec2f.clear();
			vertex_[k].context.varying_vec3f.clear();
			vertex_[k].context.varying_vec4f.clear();

			// 执行顶点着色程序，返回裁剪空间中的顶点坐标，此时没有进行透视除法
			vertex_[k].position = vertex_shader_(k, vertex_[k].context);
			vertex_[k].has_transformed = false;

			// 透视除法
			vertex_[k].w_reciprocal = 1.0f / vertex_[k].position.w;
			vertex_[k].position *= vertex_[k].w_reciprocal;

			// 屏幕映射
			vertex_[k].screen_position_f.x = (vertex_[k].position.x + 1.0f) * static_cast<float>(frame_buffer_width_ - 1) * 0.5f;
			vertex_[k].screen_position_f.y = (vertex_[k].position.y + 1.0f) * static_cast<float>(frame_buffer_height_ - 1) * 0.5f;

			// 计算整数屏幕坐标
			vertex_[k].screen_position_i.x = static_cast<int>(floor(vertex_[k].screen_position_f.x));
			vertex_[k].screen_position_i.y = static_cast<int>(floor(vertex_[k].screen_position_f.y));
			vertex_[k].screen_position_f.x = vertex_[k].screen_position_i.x + 0.5f;
			vertex_[k].screen_position_f.y = vertex_[k].screen_position_i.y + 0.5f;
		}

		// 暂时先不进行裁剪
		Vertex* raster_vertex[3] = { &vertex_[0], &vertex_[1], &vertex_[2]};

		RasterizeTriangle(raster_vertex);
	}
}

void TaoRenderer::RasterizeTriangle(Vertex* vertex[3]) {
	// 只绘制线框，不绘制像素，直接退出
	if (render_frame_ && !render_pixel_) {
		DrawWireFrame(vertex);
		return;
	}
}


// 绘制线框就是绘制三条线
void TaoRenderer::DrawWireFrame(Vertex* vertex[3]) const
{
	DrawLine(vertex[0]->screen_position_i.x, vertex[0]->screen_position_i.y, vertex[1]->screen_position_i.x, vertex[1]->screen_position_i.y);
	DrawLine(vertex[1]->screen_position_i.x, vertex[1]->screen_position_i.y, vertex[2]->screen_position_i.x, vertex[2]->screen_position_i.y);
	DrawLine(vertex[2]->screen_position_i.x, vertex[2]->screen_position_i.y, vertex[0]->screen_position_i.x, vertex[0]->screen_position_i.y);
}

void TaoRenderer::DrawLine(int x1, int y1, int x2, int y2, const Vec4f& color) const
{
	int x, y;
	if (x1 == x2 && y1 == y2) {	// 两点重合
		SetPixel(x1, y1, color);
		return;
	}
	else if (x1 == x2) {		// 两点构成一条横线
		const int dir = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += dir) SetPixel(x1, y, color);
		SetPixel(x2, y2, color);
	}
	else if (y1 == y2) {		// 两点构成一条竖线
		const int dir = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += dir) SetPixel(x, y1, color);
		SetPixel(x2, y2, color);
	}
	else {						// 两点构成一条斜线
		// 选择绘制的主轴，也就是沿着跨度较大的轴进行绘制，这样可以减少锯齿
		const int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		const int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		int rem = 0;
		if (dx >= dy) {			// X为主轴
			if (x2 < x1) {		// 交换(x1, y1)和(x2, y2)使得x1较小
				std::swap(x1, x2);
				std::swap(y1, y2);
				x = x1; y = y1;
			}
			for (x = x1, y = y1; x <= x2; x++) {
				SetPixel(x, y, color);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y2 >= y1) ? 1 : -1; SetPixel(x, y, color);
				}
			}
			SetPixel(x2, y2, color);
		}
		else {					
			if (y2 < y1) {		// 交换(x1, y1)和(x1, y1)，使得y1较小
				std::swap(x1, x2);
				std::swap(y1, y2);
				x = x1; y = y1;
			}
			for (x = x1, y = y1; y <= y2; y++) {
				SetPixel(x, y, color);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x2 >= x1) ? 1 : -1; SetPixel(x, y, color);
				}
			}
			SetPixel(x2, y2, color);
		}
	}

}

#pragma endregion
