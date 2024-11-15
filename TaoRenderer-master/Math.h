#pragma once
#include "Vector.h"
#include "Matrix.h"
#include <cmath>

/*
	�ļ����ݣ���ѧ�����䣬��������Ⱦ��������Ҫ����ѧ���㺯��
	���һ���޸����ڣ�2024.11.15
*/

constexpr float kPi = 3.1415926f;
constexpr float kEpsilon = 1e-5f;

// �������ͱ���
typedef Vector<2, float>  Vec2f;
typedef Vector<2, double> Vec2d;
typedef Vector<2, int>    Vec2i;
typedef Vector<3, float>  Vec3f;
typedef Vector<3, double> Vec3d;
typedef Vector<3, int>    Vec3i;
typedef Vector<4, float>  Vec4f, ColorRGBA;
typedef Vector<4, double> Vec4d;
typedef Vector<4, int>    Vec4i;

// ʹ��ColorRGBA�����ɫ����ʹ��ColorRGBA_32bit���ͼ�����
typedef Vector<4, uint8_t> ColorRGBA32Bit;

// �������ͱ���
typedef Matrix<4, 4, float> Mat4x4f;
typedef Matrix<3, 3, float> Mat3x3f;
typedef Matrix<4, 3, float> Mat4x3f;
typedef Matrix<3, 4, float> Mat3x4f;

#pragma region ���ߺ���
// �����˾���ֵ�����ӽ����ضϣ��м�ֵ�ȵȹ��ߺ���
template<typename T> inline T Abs(T x) { return (x < 0) ? (-x) : x; }
template<typename T> inline T Max(T x, T y) { return (x < y) ? y : x; }
template<typename T> inline T Min(T x, T y) { return (x > y) ? y : x; }

template<typename T> inline bool NearEqual(T x, T y, T error) {
	return (Abs(x - y) < error);
}

template<typename T> inline T Between(T min_x, T max_x, T x) {
	return Min(Max(min_x, x), max_x);
}

// ��ȡ [0, 1] �ķ�Χ
template<typename T> inline T Saturate(T x) {
	return Between<T>(0, 1, x);
}
#pragma endregion

#pragma region 3D ��ѧ����

// ������ת��Ϊ32λ 4byte���޷��� ���Σ�������ʾ��ɫ
inline static uint32_t vector_to_color(const Vec4f& color) {
	const auto r = static_cast<uint32_t>(Between(0, 255, static_cast<int>(color.r * 255.0f)));
	const auto g = static_cast<uint32_t>(Between(0, 255, static_cast<int>(color.g * 255.0f)));
	const auto b = static_cast<uint32_t>(Between(0, 255, static_cast<int>(color.b * 255.0f)));
	const auto a = static_cast<uint32_t>(Between(0, 255, static_cast<int>(color.a * 255.0f)));
	return (r << 16) | (g << 8) | b | (a << 24);
}

// 
inline static ColorRGBA32Bit vector_to_32bit_color(const Vec4f& color) {
	const auto r = static_cast<uint8_t>(Between(0, 255, static_cast<int>(color.r * 255.0f)));
	const auto g = static_cast<uint8_t>(Between(0, 255, static_cast<int>(color.g * 255.0f)));
	const auto b = static_cast<uint8_t>(Between(0, 255, static_cast<int>(color.b * 255.0f)));
	const auto a = static_cast<uint8_t>(Between(0, 255, static_cast<int>(color.a * 255.0f)));
	return { r, g, b, a };
}

// �����0
inline static Mat4x4f matrix_set_zero() {
	Mat4x4f m;
	m.m[0][0] = m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][1] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][2] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = m.m[3][3] = 0.0f;
	return m;
}

// ����Ϊ��λ����
inline static Mat4x4f matrix_set_identity() {
	Mat4x4f m;
	m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
	m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	return m;
}

// ƽ�Ʊ任
inline static Mat4x4f matrix_set_translate(const float x, const float y, const float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[0][3] = x;
	m.m[1][3] = y;
	m.m[2][3] = z;
	return m;
}

// ��ת�任, Χ�ƣ�x,y,z��ʸ����תtheta�Ƕ�
inline static Mat4x4f matrix_set_rotate(float x, float y, float z, float theta) {
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	float w = qcos;
	Vec3f vec = vector_normalize(Vec3f(x, y, z));
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	Mat4x4f m;
	m.m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m.m[1][0] = 2 * x * y - 2 * w * z;
	m.m[2][0] = 2 * x * z + 2 * w * y;
	m.m[0][1] = 2 * x * y + 2 * w * z;
	m.m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m.m[2][1] = 2 * y * z - 2 * w * x;
	m.m[0][2] = 2 * x * z - 2 * w * y;
	m.m[1][2] = 2 * y * z + 2 * w * x;
	m.m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
	return m;
}

/*
	����۲�任���󡪡����RTR4 4.1.6
	����˼���ǣ��Ƚ����ƽ�Ƶ�����ԭ�㣬Ȼ���ٶԻ��׽��б任��
	ʹ������rָ��1,0,0��������uָ��0,1,0��������vָ��0,0,-1�������������-z
	ע �� ��������ϵ

*/
inline static Mat4x4f matrix_look_at(const Vec3f& camera_position, const Vec3f& target, const Vec3f& up) {
	const Vec3f axis_v = vector_normalize(target - camera_position);	// ����ӽǳ���
	const Vec3f axis_r = vector_normalize(vector_cross(axis_v, up));	// ����Ҳ�
	const Vec3f axis_u = vector_cross(axis_r, axis_v);					// ����Ϸ�
	const Vec3f translate = -camera_position;							// ƽ������

	Mat4x4f m;

	m.SetRow(0, Vec4f(axis_r.x, axis_r.y, axis_r.z, -vector_dot(translate, axis_r)));
	m.SetRow(1, Vec4f(axis_u.x, axis_u.y, axis_u.z, -vector_dot(translate, axis_u)));
	m.SetRow(2, Vec4f(-axis_v.x, -axis_v.y, -axis_v.z, -vector_dot(translate, axis_v)));
	m.SetRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
	return m;
}
/*
	��������ͶӰ���� (�������-z������������ϵ)
	
	�۲�ռ����Ϊ��ƽ��Ϊl,��ƽ��Ϊr����Ϊt����Ϊb����Ϊn��ԶΪf
	������ͶӰ����Ϊ��
	2/(r-l)	0		0			-(r+l)/(r-l)
	0		2/(t-b)	0			-(t+b)/(t-b)
	0		0		2/(n-f)		-(n+f)/(n-f)
	0		0		0			1
*/
inline static Mat4x4f matrix_set_orthograhpic(float right, float left, float top, float bottom, float near, float far) {
	Mat4x4f m = matrix_set_zero();

	m.m[0][0] = 2 / (right - left);
	m.m[1][1] = 2 / (top - bottom);
	m.m[2][2] = 2 / (near - far);
	m.m[3][3] = 1;
	m.m[0][3] = -(right + left) / (right - left);
	m.m[1][3] = -(top + bottom) / (top - bottom);
	m.m[2][3] = -(near + far) / (near - far);

	return m;
}

/*
	����͸��ͶӰ���� (�������-z������������ϵ)
	����˼���GAMES101���Ȱ���׵��ѹ����һ�������壬Ȼ���ٿ�������ͶӰ
	M_persp = M_ortho * T
	fov�ǽǶȣ�ͨ����ʾ���µļнǡ�
	������Ҫע����ǣ�ʹ�÷���zbuffer������DirectX�е����ã���zӳ�䵽[0,1]��
	��͸��ͶӰ����Ϊ��
  1/(aspect*tan(fovy/2))              0             0           0
                       0  1/tan(fovy/2)             0           0
                       0              0		  f/(n-f)     fn/(n-f)
                       0              0            -1           0

*/
inline static Mat4x4f matrix_set_perspective(float fov, const float aspect, const float near_plane, const float far_plane) {
	Mat4x4f m = matrix_set_zero();

	fov = fov / 180.0f * kPi;
	const float reciprocal_tan_half_fov = 1.0f / (float)tan(fov * 0.5f);

	m.m[0][0] = reciprocal_tan_half_fov / aspect;
	m.m[1][1] = reciprocal_tan_half_fov;
	m.m[2][2] = far_plane / (near_plane - far_plane);
	m.m[2][3] = far_plane * near_plane / (near_plane - far_plane);
	m.m[3][2] = -1; 

	return m;
}

/*
	����TBN��������Ŷ�����
	t.x		t.y		t.z		0
 	b.x		b.y		b.z		0
 	n.x		n.y		n.z		0
 	0		0		0		1

	TBN������������������� = ת�þ���
	TBN * tangent_ws = tangent_os
	tangent_ws = matrix_invert(TBN) * tangent_os
*/

inline static Vec3f calculate_normal(const Vec3f& normal_ws, const Vec4f& tangent_ws, const Vec3f& perturb_normal)
{
	const Vec3f n = vector_normalize(normal_ws);
	const Vec3f t = vector_normalize(tangent_ws.xyz());
	const Vec3f b = vector_cross(n, t);

	Mat4x4f tbn = matrix_set_zero();
	tbn.SetRow(0, Vec4f(t.x, t.y, t.z, 0));
	tbn.SetRow(1, Vec4f(b.x, b.y, b.z, 0));
	tbn.SetRow(2, Vec4f(n.x, n.y, n.z, 0));
	tbn.m[3][3] = 1;

	return (matrix_invert(tbn) * perturb_normal.xyz1()).xyz();
}

#pragma endregion