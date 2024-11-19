#pragma once

#include <iostream>
#include <assert.h>
using namespace std;

/*
	�ļ����ݣ�
	�������壬�����������������������
	-���һ���޸����ڣ�2024.11.15
*/

#pragma region ��������

// Nά����ģ�� 
template <size_t N, typename T> struct Vector {
	T m[N]; // �������� N ��Ԫ��
	Vector() { for (size_t i = 0; i < N; i++) m[i] = T(); } // �޲γ�ʼ��
	explicit Vector(const T* ptr) { for (size_t i = 0; i < N; i++) m[i] = ptr[i]; } // ����Ԫ�صĵ�ַ/ָ���ʼ��
	Vector(const Vector<N, T>& v) { for (size_t i = 0; i < N; i++) m[i] = v.m[i]; } // ��ͬ���ͱ�����ʼ��
	Vector(const initializer_list<T>& l) { auto it = l.begin(); for (size_t i = 0; i < N; i++) m[i] = *it++; } // ʹ�ó�ʼ���б�
	const T& operator[] (size_t i) const { assert(i < N); return m[i]; }      // ȷ�� i �ĺϷ���
	T& operator[] (size_t i) { assert(i < N); return m[i]; }
	void load(const T* ptr) { for (size_t i = 0; i < N; i++) m[i] = ptr[i]; } // �������������ص���������
	void save(T* ptr) { for (size_t i = 0; i < N; i++) ptr[i] = m[i]; }       // �����������浽����������
};
// ��������������ػ���CG�г���2,3,4ά����
// �ػ�2ά����
template <typename T> struct Vector<2, T> {
	union {
		struct { T x, y; }; // Ԫ�ر��� x,y
		struct { T u, v; }; // Ԫ�ر��� u,v
		T m[2];
	};
	inline Vector() : x(T()), y(T()) {}     // �޲γ�ʼ��
	inline Vector(T X) : x(X), y(X) {}		// ��������ʼ��
	inline Vector(T X, T Y) : x(X), y(Y) {} // ˫������ʼ��
	inline Vector(const Vector<2, T>& v) : x(v.x), y(v.y) {}	// ͬ���ͳ�ʼ��
	inline Vector(const T* ptr) : x(ptr[0]), y(ptr[1]) {}		// T����ָ���ʼ��
	inline const T& operator[] (size_t i) const{ assert(i < 2); return m[i]; } // ȷ�� i �ĺϷ���
	inline T& operator[] (size_t i) { assert(i < 2); return m[i]; }
	inline void load(const T* ptr) { for (size_t i = 0; i < 2; i++) m[i] = ptr[i]; }
	inline void save(T* ptr) { for (size_t i = 0; i < 2; i++) ptr[i] = m[i]; }

	inline Vector<2, T> xy() const { return *this; }						// ����ԭ2ά���� 
	inline Vector<3, T> xy1() const { return Vector<3, T>(x, y, 1); }		// ���غ�һλ��1��3ά����
	inline Vector<4, T> xy11() const { return Vector<4, T>(x, y, 1, 1); }	// ���غ���λ��1��4ά����
};

// �ػ�3ά����
template <typename T> struct Vector<3, T> {
	union {
		struct { T x, y, z; };    // Ԫ�ر���
		struct { T r, g, b; };    // Ԫ�ر���
		T m[3];                   // Ԫ������
	};
	inline Vector() : x(T()), y(T()), z(T()) {}
	inline Vector(T X) : x(X), y(X), z(X) {}
	inline Vector(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
	inline Vector(const Vector<3, T>& v) : x(v.x), y(v.y), z(v.z) {}
	inline Vector(const T* ptr) : x(ptr[0]), y(ptr[1]), z(ptr[2]) {}
	inline const T& operator[] (size_t i) const { assert(i < 3); return m[i]; }
	inline T& operator[] (size_t i) { assert(i < 3); return m[i]; }
	inline void load(const T* ptr) { for (size_t i = 0; i < 3; i++) m[i] = ptr[i]; }
	inline void save(T* ptr) { for (size_t i = 0; i < 3; i++) ptr[i] = m[i]; }
	inline Vector<2, T> xy() const { return Vector<2, T>(x, y); }			// �ضϵ�3ά������2ά����
	inline Vector<3, T> xyz() const { return *this; }						// ����ԭ3ά����
	inline Vector<4, T> xyz1() const { return Vector<4, T>(x, y, z, 1); }	// ���غ�һλ��1��4ά����
};

// �ػ�4ά����
template <typename T> struct Vector<4, T> {
	union {
		struct { T x, y, z, w; };    // Ԫ�ر���
		struct { T r, g, b, a; };    // Ԫ�ر���
		T m[4];                      // Ԫ������
	};
	inline Vector() : x(T()), y(T()), z(T()), w(T()) {}
	inline Vector(T X) : x(X), y(X), z(X), w(X) {}
	inline Vector(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
	inline Vector(const Vector<4, T>& u) : x(u.x), y(u.y), z(u.z), w(u.w) {}
	inline Vector(const T* ptr) : x(ptr[0]), y(ptr[1]), z(ptr[2]), w(ptr[3]) {}
	inline const T& operator[] (size_t i) const { assert(i < 4); return m[i]; }
	inline T& operator[] (size_t i) { assert(i < 4); return m[i]; }
	inline void load(const T* ptr) { for (size_t i = 0; i < 4; i++) m[i] = ptr[i]; }
	inline void save(T* ptr) { for (size_t i = 0; i < 4; i++) ptr[i] = m[i]; }
	inline Vector<2, T> xy() const { return Vector<2, T>(x, y); }			// �ضϵ�3,4ά������2ά����
	inline Vector<3, T> xyz() const { return Vector<3, T>(x, y, z); }		// �ضϵ�4ά������3ά����
	inline Vector<4, T> xyzw() const { return *this; }						// ����ԭ4ά����
};

#pragma endregion

#pragma region ��������
// ����������������֮�䳣����������������

// (+a) ����
template <size_t N, typename T>
inline Vector<N, T> operator + (const Vector<N, T>& a) {
	return a;
}

// (-a) ����
template <size_t N, typename T>
inline Vector<N, T> operator - (const Vector<N, T>& a) {
	Vector<N, T> b;
	for (size_t i = 0; i < N; i++) b[i] = -a[i];
	return b;
}

// (a == b)���ж����
template <size_t N, typename T>
inline bool operator == (const Vector<N, T>& a, const Vector<N, T>& b) {
	for (size_t i = 0; i < N; i++) if (a[i] != b[i]) return false;
	return true;
}

// (a != b)? �жϲ���
template <size_t N, typename T>
inline bool operator != (const Vector<N, T>& a, const Vector<N, T>& b) {
	for (size_t i = 0; i < N; i++) if (a[i] != b[i]) return true;
	return false;
}

// (a + b) ���
template <size_t N, typename T>
inline Vector<N, T> operator + (const Vector<N, T>& a, const Vector<N, T>& b) {
	Vector<N, T> sum;
	for (size_t i = 0; i < N; i++) sum[i] = a[i] + b[i];
	return sum;
}

// (a - b) ���
template <size_t N, typename T>
inline Vector<N, T> operator - (const Vector<N, T>& a, const Vector<N, T>& b) {
	Vector<N, T> diff;
	for (size_t i = 0; i < N; i++) diff[i] = a[i] - b[i];
	return diff;
}

// (a * b)�����ǵ��Ҳ���ǲ�ˣ����Ǹ���Ԫ�طֱ���ˣ�ɫ�ʼ���ʱ����
template <size_t N, typename T>
inline Vector<N, T> operator * (const Vector<N, T>& a, const Vector<N, T>& b) {
	Vector<N, T> c;
	for (size_t i = 0; i < N; i++) c[i] = a[i] * b[i];
	return c;
}
 
// (a / b)������Ԫ�����
template <size_t N, typename T>
inline Vector<N, T> operator / (const Vector<N, T>& a, const Vector<N, T>& b) {
	Vector<N, T> c;
	for (size_t i = 0; i < N; i++) c[i] = a[i] / b[i];
	return c;
}

//��a * x�� ����һ������, ��������
template <size_t N, typename T>
inline Vector<N, T> operator * (const Vector<N, T>& a, T x) {
	Vector<N, T> b;
	for (size_t i = 0; i < N; i++) b[i] = a[i] * x;
	return b;
}

//��x * a������һ������, ��������
template <size_t N, typename T>
inline Vector<N, T> operator * (T x, const Vector<N, T>& a) {
	Vector<N, T> b;
	for (size_t i = 0; i < N; i++) b[i] = a[i] * x;
	return b;
}

//��a / x����һ��������
template <size_t N, typename T>
inline Vector<N, T> operator / (const Vector<N, T>& a, T x) {
	Vector<N, T> b;
	for (size_t i = 0; i < N; i++) b[i] = a[i] / x;
	return b;
}

//��x / a����һ������
template <size_t N, typename T>
inline Vector<N, T> operator / (T x, const Vector<N, T>& a) {
	Vector<N, T> b;
	for (size_t i = 0; i < N; i++) b[i] = x / a[i];
	return b;
}

// a += b
template <size_t N, typename T>
inline Vector<N, T>& operator += (Vector<N, T>& a, const Vector<N, T>& b) {
	for (size_t i = 0; i < N; i++) a[i] += b[i];
	return a;
}

// a -= b
template <size_t N, typename T>
inline Vector<N, T>& operator -= (Vector<N, T>& a, const Vector<N, T>& b) {
	for (size_t i = 0; i < N; i++) a[i] -= b[i];
	return a;
}

// a *= b
template <size_t N, typename T>
inline Vector<N, T>& operator *= (Vector<N, T>& a, const Vector<N, T>& b) {
	for (size_t i = 0; i < N; i++) a[i] *= b[i];
	return a;
}

// a /= b
template <size_t N, typename T>
inline Vector<N, T>& operator /= (Vector<N, T>& a, const Vector<N, T>& b) {
	for (size_t i = 0; i < N; i++) a[i] /= b[i];
	return a;
}

// a *= x
template <size_t N, typename T>
inline Vector<N, T>& operator *= (Vector<N, T>& a, T x) {
	for (size_t i = 0; i < N; i++) a[i] *= x;
	return a;
}

// a /= x
template <size_t N, typename T>
inline Vector<N, T>& operator /= (Vector<N, T>& a, T x) {
	for (size_t i = 0; i < N; i++) a[i] /= x;
	return a;
}

#pragma endregion

#pragma region ��������
// �����˸����������õĺ���

// = |a| ^ 2 ����ģ��ƽ����
template<size_t N, typename T>
inline T vector_length_square(const Vector<N, T>& a) {
	T sum = 0;
	for (size_t i = 0; i < N; i++) sum += a[i] * a[i];
	return sum;
}

// |a| ����ģ��
template<size_t N, typename T>
inline T vector_length(const Vector<N, T>& a) {
	return sqrt(vector_length_square(a));
}

// |a| ����ģ�������ػ� float ���ͣ�ʹ�� sqrtf
template<size_t N>
inline float vector_length(const Vector<N, float>& a) {
	return sqrtf(vector_length_square(a));
}

// a / |a| ��������ģ�����Ӷ���һ��
template<size_t N, typename T>
inline Vector<N, T> vector_normalize(const Vector<N, T>& a) {
	return a / vector_length(a);
}

// �������
template<size_t N, typename T>
inline T vector_dot(const Vector<N, T>& a, const Vector<N, T>& b) {
	T sum = 0;
	for (size_t i = 0; i < N; i++) sum += a[i] * b[i];
	return sum;
}

// ��ά������ˣ��õ�����
template<typename T>
inline T vector_cross(const Vector<2, T>& a, const Vector<2, T>& b) {
	return a.x * b.y - a.y * b.x;
}

// ��ά������ˣ��õ�������
template<typename T>
inline Vector<3, T> vector_cross(const Vector<3, T>& a, const Vector<3, T>& b) {
	return Vector<3, T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// ��ά������ˣ�ǰ��ά��ˣ���һλ����
template<typename T>
inline Vector<4, T> vector_cross(const Vector<4, T>& a, const Vector<4, T>& b) {
	return Vector<4, T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, a.w);
}

// ���㷴������, ���淢�֣������������õ���������
template<typename T>
inline Vector<3, T> vector_reflect(const Vector<3, T>& v, const Vector<3, T>& n) {
	return 2.0f * vector_dot(v, n) * n - v;
}

// a + (b - a) * t ������ֵ
template<size_t N, typename T>
inline Vector<N, T> vector_lerp(const Vector<N, T>& a, const Vector<N, T>& b, float t) {
	return a + (b - a) * t;
}

// ����Ԫ��ȡ����ֵ
template<size_t N, typename T>
inline Vector<N, T> vector_abs(const Vector<N, T>& a) {
	Vector<N, T> c;
	for (size_t i = 0; i < N; i++) c[i] = std::abs(a[i]);
	return c;
}

// ����Ԫ��ȡ�������ֵ
template<size_t N, typename T>
inline Vector<N, T> vector_max(const Vector<N, T>& a, const Vector<N, T>& b) {
	Vector<N, T> c;
	for (size_t i = 0; i < N; i++) c[i] = (a[i] > b[i]) ? a[i] : b[i];
	return c;
}
// ����Ԫ��ȡ������Сֵ
template<size_t N, typename T>
inline Vector<N, T> vector_min(const Vector<N, T>& a, const Vector<N, T>& b) {
	Vector<N, T> c;
	for (size_t i = 0; i < N; i++) c[i] = (a[i] < b[i]) ? a[i] : b[i];
	return c;
}

// ��������ֵ������ min_x/max_x ��Χ��
template<size_t N, typename T>
inline Vector<N, T> vector_between(const Vector<N, T>& min_x, const Vector<N, T>& max_x, const Vector<N, T>& x) {
	return vector_min(vector_max(min_x, x), max_x);
}

// �ж���������֮������Ƿ�С��dist
template<size_t N, typename T>
inline bool vector_near(const Vector<N, T>& a, const Vector<N, T>& b, T dist) {
	return (vector_length_square(a - b) <= dist);
}

// �ж����������������Ƿ�������
template<size_t N>
inline bool vector_near_equal(const Vector<N, float>& a, const Vector<N, float>& b, float e = 0.0001) {
	return vector_near(a, b, e);
}

// �ж�����˫���������Ƿ�������
template<size_t N>
inline bool vector_near_equal(const Vector<N, double>& a, const Vector<N, double>& b, double e = 0.0000001) {
	return vector_near(a, b, e);
}

// ����ֵԪ�ط�Χ�ü���0��1
template<size_t N, typename T>
inline Vector<N, T> vector_clamp(const Vector<N, T>& a, T min_x = 0, T max_x = 1) {
	Vector<N, T> b;
	for (size_t i = 0; i < N; i++) {
		T x = (a[i] < min_x) ? min_x : a[i];
		b[i] = (x > max_x) ? max_x : x;
	}
	return b;
}

// ������ı���
template<size_t N, typename T>
inline std::ostream& operator << (std::ostream& os, const Vector<N, T>& a) {
	os << "[";
	for (size_t i = 0; i < N; i++) {
		os << a[i];
		if (i < N - 1) os << ", ";
	}
	os << "]";
	return os;
}

#pragma endregion