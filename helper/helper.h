#pragma once
#include "../Eigen/Dense"


namespace Dia {
	const double pi = 3.1415926535;
	inline Eigen::Matrix2f rotToMat(int r) {
		return Eigen::Rotation2D<float>((-r * pi) / 180.0).toRotationMatrix();
	}
}