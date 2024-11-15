#include <iostream>
#include <cstdio>
#include "Vector.h"
#include "Matrix.h"


int main() {
	Vector<4, int> a(1, 2, 3, 4);
	Vector<4, int> b = { 1,2,3,4 };
	Vector<4, int> c = Vector<4, int>(1,2,3,4);
	Vector<1, int> d = {1};

	Matrix<1, 4, int> m1 = { a };
	Matrix<4, 1, int> m2 = { d,d,d,d };

	std:: cout << m2 * m1 << std::endl;

}