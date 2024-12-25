#include "TaoRenderer.h"

#include <ranges>



/*
	文件内容：
	-渲染器类的定义
	-最近一次修改日期：2024.12.25
*/

#pragma region 初始化与清除

TaoRenderer::TaoRenderer(const int width, const int height) {
	render_frame_ = false;
	render_pixel_ = true;
	render_shadow_ = true;
	Init(width, height);
}

// 初始化渲染器
void TaoRenderer::Init(const int width, const int height)
{	
	// 设置一下渲染器宽和高
	frame_buffer_width_ = width;
	frame_buffer_height_ = height;
	shadow_buffer_width_ = width;
	shadow_buffer_height_ = height;

	// 初始化前景色和背景色
	color_foreground_ = Vec4f(1.0f);
	color_background_ = Vec4f(0.0f);
	//color_background_ = Vec4f(0.5f, 1.0f, 1.0f, 1.0f);

	// 设置DataBuffer
	data_buffer_ = DataBuffer::GetInstance();
	window_ = Window::GetInstance();

	// 默认不是正在渲染阴影贴图与天空盒
	is_rendering_shadowMap = false;
	is_rendering_skybox = false;

	// 数据缓冲初始化
	data_buffer_->Init(height, width);

	ClearFrameBuffer(true, true);
}

void TaoRenderer::CleanUp() {

}

void TaoRenderer::ClearFrameBuffer(bool clear_color_buffer, bool clear_depth_buffer) const
{	
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	float** depth_buffer_ = data_buffer_->depth_buffer_;

	// 如果清除颜色缓冲，而且color_buffer_不为空，将颜色缓冲设置为背景色
	if (clear_color_buffer && data_buffer_->color_buffer_)
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
				depth_buffer_[j][i] = 0.f;
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
// 判断某一裁剪空间内的点是否在裁剪空间内部
bool IsInsidePlane(TaoRenderer::ClipPlane clip_plane, const Vec4f& vertex)
{
	bool result = false;
	switch (clip_plane)
	{
		// 防止进行透视除法时出现数值溢出
	case TaoRenderer::W_Plane:
		result = vertex.w >= kEpsilon;
		break;
	case TaoRenderer::X_RIGHT:
		result = vertex.x <= vertex.w;
		break;
	case TaoRenderer::X_LEFT:
		result = vertex.x >= -vertex.w;
		break;
	case TaoRenderer::Y_TOP:
		result = vertex.y <= vertex.w;
		break;
	case TaoRenderer::Y_BOTTOM:
		result = vertex.y >= -vertex.w;
		break;
	case TaoRenderer::Z_NEAR:
		result = vertex.z >= 0;
		break;
	case TaoRenderer::Z_FAR:
		result = vertex.z <= vertex.w;
		break;

	default:;
	}

	return  result;
}

#pragma endregion

#pragma region 计算函数

TaoRenderer::Vertex& TaoRenderer::GetIntersectVertex(ClipPlane clip_plane, Vertex& v1, Vertex& v2) {
	float ratio = 1.0f;
	Vec4f p = v1.cs_position, c = v2.cs_position;
	// 根据平面来获取交点位于线段的ratio
	switch (clip_plane) {
		case TaoRenderer::X_RIGHT:
			ratio = (p.x * c.w - p.w * c.w) / (p.x * c.w  - c.x * p.w);
			break;
		case TaoRenderer::X_LEFT:
			ratio = (p.x * c.w + p.w * c.w) / (p.x * c.w - c.x * p.w);
			break;
		case TaoRenderer::Y_TOP:
			ratio = (p.y * c.w - p.w * c.w) / (p.y * c.w - c.y * p.w);
			break;
		case TaoRenderer::Y_BOTTOM:
			ratio = (p.y * c.w + p.w * c.w) / (p.y * c.w - c.y * p.w);
			break;
		case TaoRenderer::Z_NEAR:
			// 由于DirectX将near plane映射到z = 0上，因此比例系数的计算公式有改变  // why
			ratio = (p.z) / (p.z - c.z);
			break;
		case TaoRenderer::Z_FAR:
			ratio = (p.z * c.w + p.w * c.w) / (p.z * c.w - c.z * p.w);
			break;
	}

	Vertex* new_vertex = new Vertex();
	new_vertex->cs_position = vector_lerp(v1.cs_position, v2.cs_position, ratio); // 根据比率获取位置
	
	Varyings& context = new_vertex->context;
	Varyings& context_v1 = v1.context;
	Varyings& context_v2 = v2.context;
	// 将两个顶点的所有类型变量都进行插值，并赋给新vector
	for (const auto& key : context_v1.varying_float | std::views::keys) {
		float f0 = context_v1.varying_float[key];
		float f1 = context_v2.varying_float[key];
		context.varying_float[key] = std::lerp(f0, f1, ratio);
	}
	for (const auto& key : context_v1.varying_vec2f | std::views::keys) {
		const Vec2f& f0 = context_v1.varying_vec2f[key];
		const Vec2f& f1 = context_v2.varying_vec2f[key];
		context.varying_vec2f[key] = vector_lerp(f0, f1, ratio);
	}
	for (const auto& key : context_v1.varying_vec3f | std::views::keys) {
		const Vec3f& f0 = context_v1.varying_vec3f[key];
		const Vec3f& f1 = context_v2.varying_vec3f[key];
		context.varying_vec3f[key] = vector_lerp(f0, f1, ratio);
	}
	for (const auto& key : context_v1.varying_vec4f | std::views::keys) {
		const Vec4f& f0 = context_v1.varying_vec4f[key];
		const Vec4f& f1 = context_v2.varying_vec4f[key];
		context.varying_vec4f[key] = vector_lerp(f0, f1, ratio);
	}

	return *new_vertex;
}

int TaoRenderer::ClipWithPlane(ClipPlane clip_plane, Vertex vertex[3]) {
	int in_vertex_count = 0;
	const int vertex_count = 3;

	for (int i = 0; i < vertex_count; i++) {
		const int cur_index = i;										// 当前顶点的序号
		const int pre_index = (i - 1 + vertex_count) % vertex_count;	// 逆时针方向前一个顶点的序号
		
		Vec4f cur_vertex = vertex[cur_index].cs_position; // 当前顶点位置
		Vec4f pre_vertex = vertex[pre_index].cs_position; // 按照顺序前一个顶点的位置

		const bool is_cur_inside = IsInsidePlane(clip_plane, cur_vertex);
		const bool is_pre_inside = IsInsidePlane(clip_plane, pre_vertex);

		// 异或，当两个顶点分别处于裁剪平面两端时，需要计算出一个新的顶点，把新顶点加到clip_vertex里
		if (is_cur_inside ^ is_pre_inside) {
			// 计算得到新的顶点
			Vertex& new_vertex = GetIntersectVertex(clip_plane, vertex[pre_index], vertex[cur_index]);
			clip_vertex_[in_vertex_count++] = &new_vertex;
		}
		// 如果当前这个点在内部，把它加入到clip_vertex里
		if (is_cur_inside) {
			clip_vertex_[in_vertex_count++] = &vertex[cur_index];
		}
	}
	return in_vertex_count;
}

#pragma region

#pragma region 绘制函数

// 绘制阴影贴图
void TaoRenderer::DrawShadowMap() 
{
	if (!render_shadow_) return;
	
	is_rendering_shadowMap = true;
	DrawMesh();
	// 此时的DepthBuffer，正是shadowBuffer
	data_buffer_->CopyShadowBuffer();
	is_rendering_shadowMap = false;
}

// 绘制网格体
void TaoRenderer::DrawMesh()
{	
	UniformBuffer* uniform_buffer_ = data_buffer_->GetUniformBuffer();
	Attributes* attributes = data_buffer_->attributes_;
	int counter = 0; // 判断多少个三角面被绘制
	for (auto model : data_buffer_->model_list_) {
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
				vertex_[k].ws_position = uniform_buffer_->model_matrix * attributes[k].position_os.xyz1();

				// 判断当前是否正在渲染阴影，如果是，则使用阴影shadow_vertex_shader
				if(is_rendering_shadowMap) vertex_[k].cs_position = shadow_shader_->vertex_shader_(k, vertex_[k].context);
				else vertex_[k].cs_position = vertex_shader_(k, vertex_[k].context);
				
				vertex_[k].has_transformed = false;
			}

			/*
			* 裁剪空间中的近平面裁剪：
			*
			* 根据三角面的三个顶点与平面形成的四种情况，对三角面进行裁剪
			*
			* 顶点顺序：
			* obj格式中默认的顶点顺序是逆时针，即顶点v1，v2，v3按照逆时针顺序排列
			*/

			int in_vertex_count = 0; // 在近平面内的顶点个数
			if (IsInsidePlane(Z_NEAR, vertex_[0].cs_position) &&
				IsInsidePlane(Z_NEAR, vertex_[1].cs_position) &&
				IsInsidePlane(Z_NEAR, vertex_[2].cs_position))
			{
				in_vertex_count = 3;
				clip_vertex_[0] = &vertex_[0];
				clip_vertex_[1] = &vertex_[1];
				clip_vertex_[2] = &vertex_[2];
			}
			else
			{
				// 在裁剪空间中，针对近裁剪平面进行裁剪
				in_vertex_count = ClipWithPlane(Z_NEAR, vertex_);
			}

			// 接下来就对裁剪后得到的3个或4个顶点所形成的1个或2个三角面进行光栅化
			for (int i = 0; i < in_vertex_count - 2; i++) {
				Vertex* raster_vertex[3] = { clip_vertex_[0], clip_vertex_[i + 1], clip_vertex_[i + 2] };

				for (int k = 0; k < 3; k++) {
					Vertex* cur_vertex = raster_vertex[k];

					if (cur_vertex->has_transformed) continue;
					cur_vertex->has_transformed = true;
					// 透视除法
					cur_vertex->w_reciprocal = 1.0f / cur_vertex->cs_position.w;
					cur_vertex->cs_position *= cur_vertex->w_reciprocal;

					// 屏幕映射
					cur_vertex->screen_position_f.x = (cur_vertex->cs_position.x + 1.0f) * static_cast<float>(frame_buffer_width_ - 1) * 0.5f;
					cur_vertex->screen_position_f.y = (cur_vertex->cs_position.y + 1.0f) * static_cast<float>(frame_buffer_height_ - 1) * 0.5f;

					// 计算整数屏幕坐标
					cur_vertex->screen_position_i.x = static_cast<int>(floor(cur_vertex->screen_position_f.x));
					cur_vertex->screen_position_i.y = static_cast<int>(floor(cur_vertex->screen_position_f.y));
					cur_vertex->screen_position_f.x = cur_vertex->screen_position_i.x + 0.5f;
					cur_vertex->screen_position_f.y = cur_vertex->screen_position_i.y + 0.5f;
				}
				/*
				* NDC空间中的背面剔除：
				*
				* 在观察空间中进行判断，观察空间使用右手坐标系，即相机看向z轴负方向
				* 判断三角形朝向，剔除背对相机的三角形
				* 由于相机看向z轴负方向，因此三角形法线的z分量为负，说明背对相机
				* 
				* 顶点顺序：
				* obj格式中默认的顶点顺序是逆时针，即顶点v1，v2，v3按照逆时针顺序排列
				*/
				const Vec4f vector_01 = raster_vertex[1]->cs_position - raster_vertex[0]->cs_position;
				const Vec4f vector_02 = raster_vertex[2]->cs_position - raster_vertex[0]->cs_position;
				const Vec4f normal = vector_cross(vector_01, vector_02);
				if (normal.z <= 0) continue;

				RasterizeTriangle(raster_vertex);
				counter++;
			}
		}
		data_buffer_->MoveToNextModel();
	}
	if(!is_rendering_shadowMap) window_->SetLogMessage("SurfaceBeenDrawn", "SurfaceBeenDrawn: " + std::to_string(counter));
}

void TaoRenderer::RasterizeTriangle(Vertex* vertex[3]) {
	// 只绘制线框，不绘制像素，直接退出
	if (render_frame_ && !render_pixel_) {
		DrawWireFrame(vertex);
		return;
	}
	// 初始化矩形右上角顶点和左下角顶点
	Vec2i bounding_min(100000, 100000), bounding_max(-100000, -100000);
	// 遍历三个顶点寻找包围三角面的矩形范围
	for (int i = 0; i < 3; i++) {
		Vec2i screen_position_i = vertex[i]->screen_position_i;
		bounding_min.x = Min(bounding_min.x, screen_position_i.x);
		bounding_min.y = Min(bounding_min.y, screen_position_i.y);
		bounding_max.x = Max(bounding_max.x, screen_position_i.x);
		bounding_max.y = Max(bounding_max.y, screen_position_i.y);
	}
	// 确保bounding box边界不超过帧缓冲的边界
	bounding_min.x = Between(0, frame_buffer_width_ - 1, bounding_min.x);
	bounding_max.x = Between(0, frame_buffer_width_ - 1, bounding_max.x);
	bounding_min.y = Between(0, frame_buffer_height_ - 1, bounding_min.y);
	bounding_max.y = Between(0, frame_buffer_height_ - 1, bounding_max.y);

	// 先保存三个顶点的位置
	Vec2i p0 = vertex[0]->screen_position_i;
	Vec2i p1 = vertex[1]->screen_position_i;
	Vec2i p2 = vertex[2]->screen_position_i;
	// 构建边缘方程，用来判断平面上一点是否在三角形内部
	Vec2i bottom_left_point = bounding_min;
	edge_equation_[0].Initialize(p1, p2, bottom_left_point, vertex[0]->w_reciprocal);
	edge_equation_[1].Initialize(p2, p0, bottom_left_point, vertex[1]->w_reciprocal);
	edge_equation_[2].Initialize(p0, p1, bottom_left_point, vertex[2]->w_reciprocal);
	// 接下来需要遍历BB内的所有像素点
	for (int x = bounding_min.x; x <= bounding_max.x; x++) {
		for (int y = bounding_min.y; y <= bounding_max.y; y++) {
			Vec2i offset = { x - bounding_min.x, y - bounding_min.y };
			
			// 判断点属于(x,y)是否位于三角形内部或者三角形边缘
			// 当属于上边缘或者左边缘的时候，将e与1进行比较，从而跳过e=0的情况
			float e0 = edge_equation_[0].Evaluate(offset.x, offset.y);
			if (e0 < (edge_equation_[0].is_top_left ? 0 : 1)) continue;

			float e1 = edge_equation_[1].Evaluate(offset.x, offset.y);
			if (e1 < (edge_equation_[1].is_top_left ? 0 : 1)) continue;

			float e2 = edge_equation_[2].Evaluate(offset.x, offset.y);
			if (e2 < (edge_equation_[2].is_top_left ? 0 : 1)) continue;

			// 计算重心坐标
			float bc_denominator = e0 + e1 + e2;
			bc_denominator = 1.0f / bc_denominator;

			// 透视不正确的插值
			float bc_p0 = e0 * bc_denominator;
			float bc_p1 = e1 * bc_denominator;
			float bc_p2 = e2 * bc_denominator;

			// 计算透视正确的中心坐标
			float bc_correct_denominator =
				e0 * edge_equation_[0].w_reciprocal +
				e1 * edge_equation_[1].w_reciprocal +
				e2 * edge_equation_[2].w_reciprocal;
			bc_correct_denominator = 1.0f / bc_correct_denominator;

			float bc_correct_p0 = e0 * edge_equation_[0].w_reciprocal * bc_correct_denominator;
			float bc_correct_p1 = e1 * edge_equation_[1].w_reciprocal * bc_correct_denominator;
			float bc_correct_p2 = e2 * edge_equation_[2].w_reciprocal * bc_correct_denominator;

			// 接下来就是插值环节，该像素位于三角形内部，根据其重心坐标，插值每一个顶点属性
			// 首先插值深度, 但是不能用透视正确的重心坐标来插值重心，
			float depth = 
				vertex[0]->cs_position.z * bc_p0 +
				vertex[1]->cs_position.z * bc_p1 +
				vertex[2]->cs_position.z * bc_p2;

			// 保证深度缓存的深度为该像素位置离摄像机最近的片元
			if (1.0f - depth <= data_buffer_->depth_buffer_[y][x]) continue;
			float origin_depth_value = data_buffer_->depth_buffer_[y][x];	// 如果后续该片元透明度为0，被丢弃时，深度缓冲需要被重设
			data_buffer_->depth_buffer_[y][x] = 1.0f - depth;


			// 准备为当前各像素的varying进行插值
			// 准备为当前像素的各项 varying 进行插值

			if (is_rendering_shadowMap) {
				SetPixel(x, y, Vec4f(1.0f - depth));
				continue; // 如果正在渲染阴影贴图，那么就不需要插值各项属性了，因为用不上pixelshader
			}

			Varyings& context_p0 = vertex[0]->context;
			Varyings& context_p1 = vertex[1]->context;
			Varyings& context_p2 = vertex[2]->context;

			// 插值各项 varying
			if (!context_p0.varying_float.empty()) {
				for (const auto& key : context_p0.varying_float | std::views::keys) {
					float f0 = context_p0.varying_float[key];
					float f1 = context_p1.varying_float[key];
					float f2 = context_p2.varying_float[key];
					current_varyings_.varying_float[key] = bc_correct_p0 * f0 + bc_correct_p1 * f1 + bc_correct_p2 * f2;
				}
			}
			if (!context_p0.varying_vec2f.empty()) {
				for (const auto& key : context_p0.varying_vec2f | std::views::keys) {
					const Vec2f& f0 = context_p0.varying_vec2f[key];
					const Vec2f& f1 = context_p1.varying_vec2f[key];
					const Vec2f& f2 = context_p2.varying_vec2f[key];
					current_varyings_.varying_vec2f[key] = bc_correct_p0 * f0 + bc_correct_p1 * f1 + bc_correct_p2 * f2;
				}
			}
			if (!context_p0.varying_vec3f.empty()) {
				for (const auto& key : context_p0.varying_vec3f | std::views::keys) {
					const Vec3f& f0 = context_p0.varying_vec3f[key];
					const Vec3f& f1 = context_p1.varying_vec3f[key];
					const Vec3f& f2 = context_p2.varying_vec3f[key];
					current_varyings_.varying_vec3f[key] = bc_correct_p0 * f0 + bc_correct_p1 * f1 + bc_correct_p2 * f2;
				}
			}
			if (!context_p0.varying_vec4f.empty()) {
				for (const auto& key : context_p0.varying_vec4f | std::views::keys) {
					const Vec4f& f0 = context_p0.varying_vec4f[key];
					const Vec4f& f1 = context_p1.varying_vec4f[key];
					const Vec4f& f2 = context_p2.varying_vec4f[key];
					current_varyings_.varying_vec4f[key] = bc_correct_p0 * f0 + bc_correct_p1 * f1 + bc_correct_p2 * f2;
				}
			}

			// 执行像素着色器
			Vec4f color = { 1.0f };
			if (pixel_shader_ != nullptr) color = pixel_shader_(current_varyings_);

			// 如果该片元透明度为0，则丢弃片元
			if (color.a == 0) {
				data_buffer_->depth_buffer_[y][x] = origin_depth_value;
				continue;
			}
			// 如果渲染阴影，判断该pixel的深度与深度缓冲中的值大小
			if (render_shadow_ && !is_rendering_skybox) {
				Mat4x4f V = data_buffer_->GetUniformBuffer()->view_matrix;
				Mat4x4f P = data_buffer_->GetUniformBuffer()->project_matrix;
				// 计算该fragment的world pos
				Vec4f worldPos0 = vertex[0]->context.varying_vec3f[1].xyz1();
				Vec4f worldPos1 = vertex[1]->context.varying_vec3f[1].xyz1();
				Vec4f worldPos2 = vertex[2]->context.varying_vec3f[1].xyz1();
				Vec4f worldPos = worldPos0 * bc_correct_p0 + worldPos1 * bc_correct_p1 + worldPos2 * bc_correct_p2;
				Vec4f csPos = data_buffer_->GetUniformBuffer()->shadow_VP_matrix * worldPos;

				// 透视除法
				float w_reciprocal = 1.0f / csPos.w;
				csPos *= w_reciprocal;

				// 该片元在光源视角下的深度
				float depthFromLight = csPos.z;
				// 该片元位于ShadowBuffer的坐标
				Vec2i shadowScreenPos(0.f);
				// 屏幕映射
				shadowScreenPos.x = static_cast<int>((csPos.x + 1.0f) * static_cast<float>(shadow_buffer_width_ - 1) * 0.5f);
				shadowScreenPos.y = static_cast<int>((csPos.y + 1.0f) * static_cast<float>(shadow_buffer_height_ - 1) * 0.5f);

				// 避免数组越界
				if (shadowScreenPos.x < 0 || shadowScreenPos.x >= shadow_buffer_width_ || shadowScreenPos.y < 0 || shadowScreenPos.y >= shadow_buffer_height_) {
					SetPixel(x, y, color);
					continue;
				}

				// 阴影bias
				float shadow_bias = .0001f;
				if (1 - depthFromLight + shadow_bias <= data_buffer_->shadow_buffer_[shadowScreenPos.y][shadowScreenPos.x]) {
					color *= 0.3f;
				}
			}
			
			SetPixel(x, y, color);
		}
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

void TaoRenderer::DrawSkybox() {
	if (data_buffer_->color_buffer_ == nullptr || skybox_shader_ == nullptr) return;

	is_rendering_skybox = true;

	UniformBuffer* uniform_buffer_ = data_buffer_->GetUniformBuffer();
	Attributes* attributes_ = data_buffer_->attributes_;

	// 将shader设置为skybox的shader
	vertex_shader_ = skybox_shader_->vertex_shader_;
	pixel_shader_ = skybox_shader_->pixel_shader_;

	for (size_t i = 0; i < skybox_shader_->plane_vertex_.size() - 2; i++)
	{
		attributes_[0].position_os = skybox_shader_->plane_vertex_[0];
		attributes_[1].position_os = skybox_shader_->plane_vertex_[i + 1];
		attributes_[2].position_os = skybox_shader_->plane_vertex_[i + 2];

		// 顶点变换
		for (int k = 0; k < 3; k++) {
			vertex_[k].context.varying_float.clear();
			vertex_[k].context.varying_vec2f.clear();
			vertex_[k].context.varying_vec3f.clear();
			vertex_[k].context.varying_vec4f.clear();

			// 执行顶点着色程序，返回裁剪空间中的顶点坐标，此时没有进行透视除法
			vertex_[k].cs_position = vertex_shader_(k, vertex_[k].context);
		}

		Vertex* raster_vertex[3] = { &vertex_[0], &vertex_[1], &vertex_[2] };
		// 执行后续顶点处理
		for (int k = 0; k < 3; k++) {
			Vertex* current_vertex = raster_vertex[k];

			// 透视除法
			current_vertex->w_reciprocal = 1.0f / current_vertex->cs_position.w;
			current_vertex->cs_position *= current_vertex->w_reciprocal;

			// 屏幕映射：计算屏幕坐标（窗口坐标。详见RTR4 章节2.3.4
			current_vertex->screen_position_f.x = (current_vertex->cs_position.x + 1.0f) * static_cast<float>(frame_buffer_width_ - 1) * 0.5f;
			current_vertex->screen_position_f.y = (current_vertex->cs_position.y + 1.0f) * static_cast<float>(frame_buffer_height_ - 1) * 0.5f;

			// 计算整数屏幕坐标：d = floor(c)
			current_vertex->screen_position_i.x = static_cast<int>(floor(current_vertex->screen_position_f.x));
			current_vertex->screen_position_i.y = static_cast<int>(floor(current_vertex->screen_position_f.y));

			//计算整数屏幕坐标：c = d + 0.5
			current_vertex->screen_position_f.x = current_vertex->screen_position_i.x + 0.5f;
			current_vertex->screen_position_f.y = current_vertex->screen_position_i.y + 0.5f;
		}

		RasterizeTriangle(raster_vertex);
	}

	is_rendering_skybox = false;
}

// 在颜色缓冲中绘制点
void TaoRenderer::SetPixel(const int x, const int y, const Vec4f& cc) const {
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	SetBuffer(color_buffer_, x, y, cc); 
}
void TaoRenderer::SetPixel(const int x, const int y, const Vec3f& cc) const {
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	SetBuffer(color_buffer_, x, y, cc.xyz1()); 
}
// 在颜色缓冲中绘制线段
void TaoRenderer::DrawLine(const int x1, const int y1, const int x2, const int y2) const {
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	if (color_buffer_) DrawLine(x1, y1, x2, y2, color_foreground_);
}

#pragma endregion
