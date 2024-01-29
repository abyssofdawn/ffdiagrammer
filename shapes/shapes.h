#pragma once
#include <array>
#include <vector>
#include "imgui.h"
#include "../Eigen/Dense"
#include <iostream>
#include "../helper/helper.h"

using Eigen::MatrixXf;
using Eigen::Matrix;
using Eigen::Rotation2D;
using Eigen::Matrix2f;

namespace Dia {
	class Shape {
	public:
		MatrixXf points;
		int rotation;
		int position[2];
		int rotationCenter[2];
		ImColor col[2];
		virtual void Settings();
		bool BasicSettings();
		virtual void RenderShape(ImVec2 origin);
		virtual void GeneratePoints();
		Shape();
	};

	class Rectangle : public Shape {
	public:
		void Settings() override;
		Rectangle();
		int sideLengths[2] = { 1, 1 };
		void GeneratePoints() override;
	};

	class Union {
	public:
		std::vector<std::unique_ptr<Shape>> shapes;
		std::vector<MatrixXf> paths;
		MatrixXf points;
		void RenderUnion(ImVec2 origin);
		bool BasicSettings();
		int rotation;
		int position[2];
		int rotationCenter[2];
	};

	extern ImVec2 renderGridStart();
	extern void renderGridEnd();
}