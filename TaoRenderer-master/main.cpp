#include <iostream>
#include <cstdio>
#include "Vector.h"
#include "Matrix.h"
#include "Math.h"


int main() {
	Vector<4, int> a(1, 2, 3, 4);
	Vector<4, int> b = { 1,2,3,4 };
	Vector<4, int> c = Vector<4, int>(1,2,3,4);
	Vector<1, int> d = {1};

	Matrix<1, 4, int> m1 = { a };
	Matrix<4, 1, int> m2 = { d,d,d,d };

	Mat4x4f m_ortho = matrix_set_orthograhpic(10,-10,10,-10,10,-10);

	std:: cout << m_ortho << std::endl;

}