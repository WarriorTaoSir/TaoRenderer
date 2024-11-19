#include <iostream>
#include <cstdio>
#include "Vector.h"
#include "Matrix.h"
#include "Math.h"
#include "Model.h"


int main() {
	Model* python = new Model("../assets/helmet/helmet.obj", matrix_set_identity());

	std:: cout << python->PrintModelInfo() << std::endl;
}