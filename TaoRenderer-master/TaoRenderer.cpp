#include "TaoRenderer.h"

#include <ranges>



/*
	�ļ����ݣ�
	-��Ⱦ����Ķ���
	-���һ���޸����ڣ�2024.12.25
*/

#pragma region ��ʼ�������

TaoRenderer::TaoRenderer(const int width, const int height) {
	render_frame_ = false;
	render_pixel_ = true;
	render_shadow_ = true;
	Init(width, height);
}

// ��ʼ����Ⱦ��
void TaoRenderer::Init(const int width, const int height)
{	
	// ����һ����Ⱦ����͸�
	frame_buffer_width_ = width;
	frame_buffer_height_ = height;
	shadow_buffer_width_ = width;
	shadow_buffer_height_ = height;

	// ��ʼ��ǰ��ɫ�ͱ���ɫ
	color_foreground_ = Vec4f(1.0f);
	color_background_ = Vec4f(0.0f);
	//color_background_ = Vec4f(0.5f, 1.0f, 1.0f, 1.0f);

	// ����DataBuffer
	data_buffer_ = DataBuffer::GetInstance();
	window_ = Window::GetInstance();

	// Ĭ�ϲ���������Ⱦ��Ӱ��ͼ����պ�
	is_rendering_shadowMap = false;
	is_rendering_skybox = false;

	// ���ݻ����ʼ��
	data_buffer_->Init(height, width);

	ClearFrameBuffer(true, true);
}

void TaoRenderer::CleanUp() {

}

void TaoRenderer::ClearFrameBuffer(bool clear_color_buffer, bool clear_depth_buffer) const
{	
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	float** depth_buffer_ = data_buffer_->depth_buffer_;

	// ��������ɫ���壬����color_buffer_��Ϊ�գ�����ɫ��������Ϊ����ɫ
	if (clear_color_buffer && data_buffer_->color_buffer_)
	{
		const ColorRGBA32Bit color_32_bit = vector_to_32bit_color(color_background_);

		for (int j = 0; j < frame_buffer_height_; j++) {
			const int offset = frame_buffer_width_ * (4 * j);
			for (int i = 0; i < frame_buffer_width_; i++)
			{
				const int base_address = offset + 4 * i;
				//32 bitλͼ�洢˳�򣬴ӵ͵�������ΪBGRA
				color_buffer_[base_address] = color_32_bit.b;
				color_buffer_[base_address + 1] = color_32_bit.g;
				color_buffer_[base_address + 2] = color_32_bit.r;
				color_buffer_[base_address + 3] = color_32_bit.a;
			}
		}
	}
	// ��������Ȼ��棬����depth_buffer_��Ϊ�գ�����Ȼ�������Ϊ0.0f
	if (clear_depth_buffer && depth_buffer_) {
		for (int j = 0; j < frame_buffer_height_; j++) {
			for (int i = 0; i < frame_buffer_width_; i++)
				depth_buffer_[j][i] = 0.f;
		}
	}
}
// ������ɫ����
void TaoRenderer::SetBuffer(uint8_t* buffer, const int x, const int y, const Vec4f& color) const
{
	if (x < 0 || x>frame_buffer_width_ - 1) return;
	if (y < 0 || y>frame_buffer_height_ - 1) return;

	const ColorRGBA32Bit color_32_bit = vector_to_32bit_color(color);
	const int base_address = frame_buffer_width_ * (4 * y) + 4 * x;
	//32 bitλͼ�洢˳�򣬴ӵ͵�������ΪBGRA
	buffer[base_address] = color_32_bit.b;
	buffer[base_address + 1] = color_32_bit.g;
	buffer[base_address + 2] = color_32_bit.r;
	buffer[base_address + 3] = color_32_bit.a;
}



#pragma endregion

#pragma region �жϺ���
// �ж�ĳһ�ü��ռ��ڵĵ��Ƿ��ڲü��ռ��ڲ�
bool IsInsidePlane(TaoRenderer::ClipPlane clip_plane, const Vec4f& vertex)
{
	bool result = false;
	switch (clip_plane)
	{
		// ��ֹ����͸�ӳ���ʱ������ֵ���
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

#pragma region ���㺯��

TaoRenderer::Vertex& TaoRenderer::GetIntersectVertex(ClipPlane clip_plane, Vertex& v1, Vertex& v2) {
	float ratio = 1.0f;
	Vec4f p = v1.cs_position, c = v2.cs_position;
	// ����ƽ������ȡ����λ���߶ε�ratio
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
			// ����DirectX��near planeӳ�䵽z = 0�ϣ���˱���ϵ���ļ��㹫ʽ�иı�  // why
			ratio = (p.z) / (p.z - c.z);
			break;
		case TaoRenderer::Z_FAR:
			ratio = (p.z * c.w + p.w * c.w) / (p.z * c.w - c.z * p.w);
			break;
	}

	Vertex* new_vertex = new Vertex();
	new_vertex->cs_position = vector_lerp(v1.cs_position, v2.cs_position, ratio); // ���ݱ��ʻ�ȡλ��
	
	Varyings& context = new_vertex->context;
	Varyings& context_v1 = v1.context;
	Varyings& context_v2 = v2.context;
	// ������������������ͱ��������в�ֵ����������vector
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
		const int cur_index = i;										// ��ǰ��������
		const int pre_index = (i - 1 + vertex_count) % vertex_count;	// ��ʱ�뷽��ǰһ����������
		
		Vec4f cur_vertex = vertex[cur_index].cs_position; // ��ǰ����λ��
		Vec4f pre_vertex = vertex[pre_index].cs_position; // ����˳��ǰһ�������λ��

		const bool is_cur_inside = IsInsidePlane(clip_plane, cur_vertex);
		const bool is_pre_inside = IsInsidePlane(clip_plane, pre_vertex);

		// ��򣬵���������ֱ��ڲü�ƽ������ʱ����Ҫ�����һ���µĶ��㣬���¶���ӵ�clip_vertex��
		if (is_cur_inside ^ is_pre_inside) {
			// ����õ��µĶ���
			Vertex& new_vertex = GetIntersectVertex(clip_plane, vertex[pre_index], vertex[cur_index]);
			clip_vertex_[in_vertex_count++] = &new_vertex;
		}
		// �����ǰ��������ڲ����������뵽clip_vertex��
		if (is_cur_inside) {
			clip_vertex_[in_vertex_count++] = &vertex[cur_index];
		}
	}
	return in_vertex_count;
}

#pragma region

#pragma region ���ƺ���

// ������Ӱ��ͼ
void TaoRenderer::DrawShadowMap() 
{
	if (!render_shadow_) return;
	
	is_rendering_shadowMap = true;
	DrawMesh();
	// ��ʱ��DepthBuffer������shadowBuffer
	data_buffer_->CopyShadowBuffer();
	is_rendering_shadowMap = false;
}

// ����������
void TaoRenderer::DrawMesh()
{	
	UniformBuffer* uniform_buffer_ = data_buffer_->GetUniformBuffer();
	Attributes* attributes = data_buffer_->attributes_;
	int counter = 0; // �ж϶��ٸ������汻����
	for (auto model : data_buffer_->model_list_) {
		// ����ÿ��������ķ�ʽ��ѭ����ģ�����ж���
		for (size_t i = 0; i < model->attributes_.size(); i += 3)
		{
			// �Ѷ�Ӧmodel�������attributes���ø���Ӧshader��attributes
			for (int j = 0; j < 3; j++) {
				attributes[j].position_os = model->attributes_[i + j].position_os;
				attributes[j].texcoord = model->attributes_[i + j].texcoord;
				attributes[j].normal_os = model->attributes_[i + j].normal_os;
				attributes[j].tangent_os = model->attributes_[i + j].tangent_os;
			}
			// ����任
			for (int k = 0; k < 3; k++) {
				vertex_[k].context.varying_float.clear();
				vertex_[k].context.varying_vec2f.clear();
				vertex_[k].context.varying_vec3f.clear();
				vertex_[k].context.varying_vec4f.clear();

				// ִ�ж�����ɫ���򣬷��زü��ռ��еĶ������꣬��ʱû�н���͸�ӳ���
				vertex_[k].ws_position = uniform_buffer_->model_matrix * attributes[k].position_os.xyz1();

				// �жϵ�ǰ�Ƿ�������Ⱦ��Ӱ������ǣ���ʹ����Ӱshadow_vertex_shader
				if(is_rendering_shadowMap) vertex_[k].cs_position = shadow_shader_->vertex_shader_(k, vertex_[k].context);
				else vertex_[k].cs_position = vertex_shader_(k, vertex_[k].context);
				
				vertex_[k].has_transformed = false;
			}

			/*
			* �ü��ռ��еĽ�ƽ��ü���
			*
			* ���������������������ƽ���γɵ��������������������вü�
			*
			* ����˳��
			* obj��ʽ��Ĭ�ϵĶ���˳������ʱ�룬������v1��v2��v3������ʱ��˳������
			*/

			int in_vertex_count = 0; // �ڽ�ƽ���ڵĶ������
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
				// �ڲü��ռ��У���Խ��ü�ƽ����вü�
				in_vertex_count = ClipWithPlane(Z_NEAR, vertex_);
			}

			// �������ͶԲü���õ���3����4���������γɵ�1����2����������й�դ��
			for (int i = 0; i < in_vertex_count - 2; i++) {
				Vertex* raster_vertex[3] = { clip_vertex_[0], clip_vertex_[i + 1], clip_vertex_[i + 2] };

				for (int k = 0; k < 3; k++) {
					Vertex* cur_vertex = raster_vertex[k];

					if (cur_vertex->has_transformed) continue;
					cur_vertex->has_transformed = true;
					// ͸�ӳ���
					cur_vertex->w_reciprocal = 1.0f / cur_vertex->cs_position.w;
					cur_vertex->cs_position *= cur_vertex->w_reciprocal;

					// ��Ļӳ��
					cur_vertex->screen_position_f.x = (cur_vertex->cs_position.x + 1.0f) * static_cast<float>(frame_buffer_width_ - 1) * 0.5f;
					cur_vertex->screen_position_f.y = (cur_vertex->cs_position.y + 1.0f) * static_cast<float>(frame_buffer_height_ - 1) * 0.5f;

					// ����������Ļ����
					cur_vertex->screen_position_i.x = static_cast<int>(floor(cur_vertex->screen_position_f.x));
					cur_vertex->screen_position_i.y = static_cast<int>(floor(cur_vertex->screen_position_f.y));
					cur_vertex->screen_position_f.x = cur_vertex->screen_position_i.x + 0.5f;
					cur_vertex->screen_position_f.y = cur_vertex->screen_position_i.y + 0.5f;
				}
				/*
				* NDC�ռ��еı����޳���
				*
				* �ڹ۲�ռ��н����жϣ��۲�ռ�ʹ����������ϵ�����������z�Ḻ����
				* �ж������γ����޳����������������
				* �����������z�Ḻ������������η��ߵ�z����Ϊ����˵���������
				* 
				* ����˳��
				* obj��ʽ��Ĭ�ϵĶ���˳������ʱ�룬������v1��v2��v3������ʱ��˳������
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
	// ֻ�����߿򣬲��������أ�ֱ���˳�
	if (render_frame_ && !render_pixel_) {
		DrawWireFrame(vertex);
		return;
	}
	// ��ʼ���������ϽǶ�������½Ƕ���
	Vec2i bounding_min(100000, 100000), bounding_max(-100000, -100000);
	// ������������Ѱ�Ұ�Χ������ľ��η�Χ
	for (int i = 0; i < 3; i++) {
		Vec2i screen_position_i = vertex[i]->screen_position_i;
		bounding_min.x = Min(bounding_min.x, screen_position_i.x);
		bounding_min.y = Min(bounding_min.y, screen_position_i.y);
		bounding_max.x = Max(bounding_max.x, screen_position_i.x);
		bounding_max.y = Max(bounding_max.y, screen_position_i.y);
	}
	// ȷ��bounding box�߽粻����֡����ı߽�
	bounding_min.x = Between(0, frame_buffer_width_ - 1, bounding_min.x);
	bounding_max.x = Between(0, frame_buffer_width_ - 1, bounding_max.x);
	bounding_min.y = Between(0, frame_buffer_height_ - 1, bounding_min.y);
	bounding_max.y = Between(0, frame_buffer_height_ - 1, bounding_max.y);

	// �ȱ������������λ��
	Vec2i p0 = vertex[0]->screen_position_i;
	Vec2i p1 = vertex[1]->screen_position_i;
	Vec2i p2 = vertex[2]->screen_position_i;
	// ������Ե���̣������ж�ƽ����һ���Ƿ����������ڲ�
	Vec2i bottom_left_point = bounding_min;
	edge_equation_[0].Initialize(p1, p2, bottom_left_point, vertex[0]->w_reciprocal);
	edge_equation_[1].Initialize(p2, p0, bottom_left_point, vertex[1]->w_reciprocal);
	edge_equation_[2].Initialize(p0, p1, bottom_left_point, vertex[2]->w_reciprocal);
	// ��������Ҫ����BB�ڵ��������ص�
	for (int x = bounding_min.x; x <= bounding_max.x; x++) {
		for (int y = bounding_min.y; y <= bounding_max.y; y++) {
			Vec2i offset = { x - bounding_min.x, y - bounding_min.y };
			
			// �жϵ�����(x,y)�Ƿ�λ���������ڲ����������α�Ե
			// �������ϱ�Ե�������Ե��ʱ�򣬽�e��1���бȽϣ��Ӷ�����e=0�����
			float e0 = edge_equation_[0].Evaluate(offset.x, offset.y);
			if (e0 < (edge_equation_[0].is_top_left ? 0 : 1)) continue;

			float e1 = edge_equation_[1].Evaluate(offset.x, offset.y);
			if (e1 < (edge_equation_[1].is_top_left ? 0 : 1)) continue;

			float e2 = edge_equation_[2].Evaluate(offset.x, offset.y);
			if (e2 < (edge_equation_[2].is_top_left ? 0 : 1)) continue;

			// ������������
			float bc_denominator = e0 + e1 + e2;
			bc_denominator = 1.0f / bc_denominator;

			// ͸�Ӳ���ȷ�Ĳ�ֵ
			float bc_p0 = e0 * bc_denominator;
			float bc_p1 = e1 * bc_denominator;
			float bc_p2 = e2 * bc_denominator;

			// ����͸����ȷ����������
			float bc_correct_denominator =
				e0 * edge_equation_[0].w_reciprocal +
				e1 * edge_equation_[1].w_reciprocal +
				e2 * edge_equation_[2].w_reciprocal;
			bc_correct_denominator = 1.0f / bc_correct_denominator;

			float bc_correct_p0 = e0 * edge_equation_[0].w_reciprocal * bc_correct_denominator;
			float bc_correct_p1 = e1 * edge_equation_[1].w_reciprocal * bc_correct_denominator;
			float bc_correct_p2 = e2 * edge_equation_[2].w_reciprocal * bc_correct_denominator;

			// ���������ǲ�ֵ���ڣ�������λ���������ڲ����������������꣬��ֵÿһ����������
			// ���Ȳ�ֵ���, ���ǲ�����͸����ȷ��������������ֵ���ģ�
			float depth = 
				vertex[0]->cs_position.z * bc_p0 +
				vertex[1]->cs_position.z * bc_p1 +
				vertex[2]->cs_position.z * bc_p2;

			// ��֤��Ȼ�������Ϊ������λ��������������ƬԪ
			if (1.0f - depth <= data_buffer_->depth_buffer_[y][x]) continue;
			float origin_depth_value = data_buffer_->depth_buffer_[y][x];	// ���������ƬԪ͸����Ϊ0��������ʱ����Ȼ�����Ҫ������
			data_buffer_->depth_buffer_[y][x] = 1.0f - depth;


			// ׼��Ϊ��ǰ�����ص�varying���в�ֵ
			// ׼��Ϊ��ǰ���صĸ��� varying ���в�ֵ

			if (is_rendering_shadowMap) {
				SetPixel(x, y, Vec4f(1.0f - depth));
				continue; // ���������Ⱦ��Ӱ��ͼ����ô�Ͳ���Ҫ��ֵ���������ˣ���Ϊ�ò���pixelshader
			}

			Varyings& context_p0 = vertex[0]->context;
			Varyings& context_p1 = vertex[1]->context;
			Varyings& context_p2 = vertex[2]->context;

			// ��ֵ���� varying
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

			// ִ��������ɫ��
			Vec4f color = { 1.0f };
			if (pixel_shader_ != nullptr) color = pixel_shader_(current_varyings_);

			// �����ƬԪ͸����Ϊ0������ƬԪ
			if (color.a == 0) {
				data_buffer_->depth_buffer_[y][x] = origin_depth_value;
				continue;
			}
			// �����Ⱦ��Ӱ���жϸ�pixel���������Ȼ����е�ֵ��С
			if (render_shadow_ && !is_rendering_skybox) {
				Mat4x4f V = data_buffer_->GetUniformBuffer()->view_matrix;
				Mat4x4f P = data_buffer_->GetUniformBuffer()->project_matrix;
				// �����fragment��world pos
				Vec4f worldPos0 = vertex[0]->context.varying_vec3f[1].xyz1();
				Vec4f worldPos1 = vertex[1]->context.varying_vec3f[1].xyz1();
				Vec4f worldPos2 = vertex[2]->context.varying_vec3f[1].xyz1();
				Vec4f worldPos = worldPos0 * bc_correct_p0 + worldPos1 * bc_correct_p1 + worldPos2 * bc_correct_p2;
				Vec4f csPos = data_buffer_->GetUniformBuffer()->shadow_VP_matrix * worldPos;

				// ͸�ӳ���
				float w_reciprocal = 1.0f / csPos.w;
				csPos *= w_reciprocal;

				// ��ƬԪ�ڹ�Դ�ӽ��µ����
				float depthFromLight = csPos.z;
				// ��ƬԪλ��ShadowBuffer������
				Vec2i shadowScreenPos(0.f);
				// ��Ļӳ��
				shadowScreenPos.x = static_cast<int>((csPos.x + 1.0f) * static_cast<float>(shadow_buffer_width_ - 1) * 0.5f);
				shadowScreenPos.y = static_cast<int>((csPos.y + 1.0f) * static_cast<float>(shadow_buffer_height_ - 1) * 0.5f);

				// ��������Խ��
				if (shadowScreenPos.x < 0 || shadowScreenPos.x >= shadow_buffer_width_ || shadowScreenPos.y < 0 || shadowScreenPos.y >= shadow_buffer_height_) {
					SetPixel(x, y, color);
					continue;
				}

				// ��Ӱbias
				float shadow_bias = .0001f;
				if (1 - depthFromLight + shadow_bias <= data_buffer_->shadow_buffer_[shadowScreenPos.y][shadowScreenPos.x]) {
					color *= 0.3f;
				}
			}
			
			SetPixel(x, y, color);
		}
	}
}


// �����߿���ǻ���������
void TaoRenderer::DrawWireFrame(Vertex* vertex[3]) const
{
	DrawLine(vertex[0]->screen_position_i.x, vertex[0]->screen_position_i.y, vertex[1]->screen_position_i.x, vertex[1]->screen_position_i.y);
	DrawLine(vertex[1]->screen_position_i.x, vertex[1]->screen_position_i.y, vertex[2]->screen_position_i.x, vertex[2]->screen_position_i.y);
	DrawLine(vertex[2]->screen_position_i.x, vertex[2]->screen_position_i.y, vertex[0]->screen_position_i.x, vertex[0]->screen_position_i.y);
}

void TaoRenderer::DrawLine(int x1, int y1, int x2, int y2, const Vec4f& color) const
{	
	int x, y;
	if (x1 == x2 && y1 == y2) {	// �����غ�
		SetPixel(x1, y1, color);
		return;
	}
	else if (x1 == x2) {		// ���㹹��һ������
		const int dir = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += dir) SetPixel(x1, y, color);
		SetPixel(x2, y2, color);
	}
	else if (y1 == y2) {		// ���㹹��һ������
		const int dir = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += dir) SetPixel(x, y1, color);
		SetPixel(x2, y2, color);
	}
	else {						// ���㹹��һ��б��
		// ѡ����Ƶ����ᣬҲ�������ſ�Ƚϴ������л��ƣ��������Լ��پ��
		const int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		const int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		int rem = 0;
		if (dx >= dy) {			// XΪ����
			if (x2 < x1) {		// ����(x1, y1)��(x2, y2)ʹ��x1��С
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
			if (y2 < y1) {		// ����(x1, y1)��(x1, y1)��ʹ��y1��С
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

	// ��shader����Ϊskybox��shader
	vertex_shader_ = skybox_shader_->vertex_shader_;
	pixel_shader_ = skybox_shader_->pixel_shader_;

	for (size_t i = 0; i < skybox_shader_->plane_vertex_.size() - 2; i++)
	{
		attributes_[0].position_os = skybox_shader_->plane_vertex_[0];
		attributes_[1].position_os = skybox_shader_->plane_vertex_[i + 1];
		attributes_[2].position_os = skybox_shader_->plane_vertex_[i + 2];

		// ����任
		for (int k = 0; k < 3; k++) {
			vertex_[k].context.varying_float.clear();
			vertex_[k].context.varying_vec2f.clear();
			vertex_[k].context.varying_vec3f.clear();
			vertex_[k].context.varying_vec4f.clear();

			// ִ�ж�����ɫ���򣬷��زü��ռ��еĶ������꣬��ʱû�н���͸�ӳ���
			vertex_[k].cs_position = vertex_shader_(k, vertex_[k].context);
		}

		Vertex* raster_vertex[3] = { &vertex_[0], &vertex_[1], &vertex_[2] };
		// ִ�к������㴦��
		for (int k = 0; k < 3; k++) {
			Vertex* current_vertex = raster_vertex[k];

			// ͸�ӳ���
			current_vertex->w_reciprocal = 1.0f / current_vertex->cs_position.w;
			current_vertex->cs_position *= current_vertex->w_reciprocal;

			// ��Ļӳ�䣺������Ļ���꣨�������ꡣ���RTR4 �½�2.3.4
			current_vertex->screen_position_f.x = (current_vertex->cs_position.x + 1.0f) * static_cast<float>(frame_buffer_width_ - 1) * 0.5f;
			current_vertex->screen_position_f.y = (current_vertex->cs_position.y + 1.0f) * static_cast<float>(frame_buffer_height_ - 1) * 0.5f;

			// ����������Ļ���꣺d = floor(c)
			current_vertex->screen_position_i.x = static_cast<int>(floor(current_vertex->screen_position_f.x));
			current_vertex->screen_position_i.y = static_cast<int>(floor(current_vertex->screen_position_f.y));

			//����������Ļ���꣺c = d + 0.5
			current_vertex->screen_position_f.x = current_vertex->screen_position_i.x + 0.5f;
			current_vertex->screen_position_f.y = current_vertex->screen_position_i.y + 0.5f;
		}

		RasterizeTriangle(raster_vertex);
	}

	is_rendering_skybox = false;
}

// ����ɫ�����л��Ƶ�
void TaoRenderer::SetPixel(const int x, const int y, const Vec4f& cc) const {
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	SetBuffer(color_buffer_, x, y, cc); 
}
void TaoRenderer::SetPixel(const int x, const int y, const Vec3f& cc) const {
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	SetBuffer(color_buffer_, x, y, cc.xyz1()); 
}
// ����ɫ�����л����߶�
void TaoRenderer::DrawLine(const int x1, const int y1, const int x2, const int y2) const {
	uint8_t* color_buffer_ = data_buffer_->color_buffer_;
	if (color_buffer_) DrawLine(x1, y1, x2, y2, color_foreground_);
}

#pragma endregion
