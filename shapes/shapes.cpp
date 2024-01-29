#include "shapes.h"

namespace Dia {

	bool Shape::BasicSettings() {
		ImGui::PushID(this);
		static bool centerRot = true;
		ImGui::Checkbox("center rotation?", &centerRot);
		ImGui::DragInt("rotation", &rotation, 1, 0, 0, "%d deg");
		rotation %= 360;
		if (rotation < 0) rotation += 360;
		ImGui::SliderInt2("(x,y)", position, -100, 100);
		ImGui::BeginDisabled(centerRot);

		ImGui::SliderInt2("rel (x,y)", rotationCenter, -100, 100);

		ImGui::EndDisabled();
		ImGui::PopID();
		return centerRot;
	}
	void Shape::Settings()
	{
		ImGui::Begin("settings");

		BasicSettings();

		ImGui::End();
	}
	void Rectangle::Settings()
	{
		ImGui::Begin("rectangle");
		ImGui::PushID(this);
		if (BasicSettings()) {
			rotationCenter[1] = -sideLengths[1] / 2;
			rotationCenter[0] = 0;
		}
		ImGui::SliderInt2("side lengths", sideLengths, 1, 200, "%dy");

		ImGui::PopID();
		ImGui::End();
	}
	Shape::Shape() {
		rotation = 0;
		position[0] = 0;
		position[1] = 0;
	}
	Rectangle::Rectangle()
	{
		sideLengths[0] = 50;
		sideLengths[1] = 50;
	}

	void Shape::GeneratePoints() {
	}

	void Rectangle::GeneratePoints() {
		const double pi = 3.1415926535;
		Matrix2f rot = Dia::rotToMat(rotation);

		Eigen::Vector2f pos;
		pos << position[0], position[1];
		Eigen::Vector2f rpos;
		rpos << rotationCenter[0], rotationCenter[1];
		Matrix<float, 2, 4> rpoints;
		rpoints << 
			(Eigen::Vector2f() << 0, 0).finished()+rpos,
			(Eigen::Vector2f() << 0, sideLengths[1]).finished()+rpos,
			(Eigen::Vector2f() << sideLengths[0], sideLengths[1]).finished()+rpos,
			(Eigen::Vector2f() << sideLengths[0], 0).finished()+rpos;
		rpoints = rot * rpoints;
		for (auto c : rpoints.colwise()) {
			c = c + pos;
		}
		points = rpoints;


	}

	void Shape::RenderShape(ImVec2 origin) {
		GeneratePoints();


		static ImVector<ImVec2> rpoints;
		
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (auto c : points.colwise()) {
			rpoints.push_back(ImVec2(c(0), c(1)));
		}

		draw_list->PathClear();
		for (int n = 0; n <= rpoints.Size; n++){
			ImVec2 temp = ImVec2(origin.x + rpoints[n % rpoints.Size].x, origin.y + rpoints[n % rpoints.Size].y);
			draw_list->PathLineTo(temp);
		}
		draw_list->PathStroke(IM_COL32(255, 0, 0, 255), 0, 2.0f);
		for (int n = 0; n <= rpoints.Size; n++) {
			ImVec2 temp = ImVec2(origin.x + rpoints[n % rpoints.Size].x, origin.y + rpoints[n % rpoints.Size].y);
			draw_list->PathLineTo(temp);
		}
		draw_list->PathFillConvex(IM_COL32(255, 0, 0, 50));

		draw_list->AddRectFilled(ImVec2(origin.x + - 2 + position[0], origin.y - 2 + position[1]), ImVec2(origin.x  + 2 + position[0], origin.y + 2 + position[1]), IM_COL32(255, 255, 255, 255), 2);
		rpoints.clear();
	}
	ImVec2 renderGridStart()
	{
		static bool opt_enable_grid = true;
		static bool adding_line = false;
		static int grid_step = 64;
		ImGui::Checkbox("Enable grid", &opt_enable_grid);
		ImGui::SameLine();
		ImGui::SliderInt("grid size", &grid_step, 1, 100);

		ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
		ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available

		if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
		if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
		ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
		static ImVec2 scrolling(canvas_sz.x/2,canvas_sz.y/2);

		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		const bool is_hovered = ImGui::IsItemHovered(); // Hovered
		const bool is_active = ImGui::IsItemActive();   // Held
		const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
		const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

		if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 1))
		{
			scrolling.x += io.MouseDelta.x;
			scrolling.y += io.MouseDelta.y;
		}

		// Draw grid + all lines in the canvas
		draw_list->PushClipRect(canvas_p0, canvas_p1, true);
		if (opt_enable_grid)
		{
			for (float x = fmod(scrolling.x, grid_step); x < canvas_sz.x; x += grid_step)
				draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
			for (float y = fmod(scrolling.y, grid_step); y < canvas_sz.y; y += grid_step)
				draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
		}

		return origin;
	}
	void renderGridEnd()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->PopClipRect();
	}
	void Union::RenderUnion(ImVec2 origin)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (auto &s : shapes) {
			s->RenderShape(ImVec2(origin.x+position[0], origin.y+position[1]));
		}
		draw_list->AddRectFilled(ImVec2(origin.x + -2 + position[0], origin.y - 2 + position[1]), ImVec2(origin.x + 2 + position[0], origin.y + 2 + position[1]), IM_COL32(255, 255, 255, 255), 2);

	}
	bool Union::BasicSettings()
	{
		ImGui::PushID(this);
		static bool centerRot = true;
		ImGui::Checkbox("center rotation?", &centerRot);
		ImGui::DragInt("rotation", &rotation, 1, 0, 0, "%d deg");
		rotation %= 360;
		if (rotation < 0) rotation += 360;
		ImGui::SliderInt2("(x,y)", position, -100, 100);
		ImGui::BeginDisabled(centerRot);

		ImGui::SliderInt2("rel (x,y)", rotationCenter, -100, 100);

		ImGui::EndDisabled();

		for (auto &s : shapes) {
			s->Settings();
		}

		ImGui::PopID();
		return centerRot;
	}
}
