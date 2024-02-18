#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "imgui/misc/freetype/imgui_freetype.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Downloads Manager", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	 // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	  // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL2_Init();

	// Use a wide character range to use all avaiable Unicode characters
	static ImWchar ranges[] = {0x01, 0x1FFFF, 0};

	static ImFontConfig cfg0;
	cfg0.OversampleH = cfg0.OversampleV = 1;
	cfg0.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

	io.Fonts->AddFontFromFileTTF("imgui/misc/fonts/Roboto-Medium.ttf", 15.0f, &cfg0, ranges);

	// Load a secondary emoji font
	static ImFontConfig cfg1;
	cfg1.OversampleH = cfg1.OversampleV = 1;
	cfg1.MergeMode = true;
	cfg1.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor; // Need to enable to render emojis
	io.Fonts->AddFontFromFileTTF("/usr/share/fonts/WindowsFonts/seguiemj.ttf", 15.0f, &cfg1, ranges);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	float frameRate = 60;
	int keep_time = -1;

	char buffer[12340];
	for(int i = 0; i < 12340; i++) buffer[i] = 0;

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Downloads Manager", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		std::string temp = "file.txt";

		std::string text = "How long should  " + temp + " be kept for?";
		const char* ff = text.c_str();

		ImGui::Text(ff);

		if (ImGui::Button("1 Hour")) keep_time = 1;
		ImGui::SameLine();
		if (ImGui::Button("1 Day" )) keep_time = 24 * 1;
		ImGui::SameLine();
		if (ImGui::Button("3 Days")) keep_time = 24 * 3;
		ImGui::SameLine();
		if (ImGui::Button("7 Days")) keep_time = 24 * 7;

		ImGui::InputText("Input Text", buffer, 12340);
		ImGui::TextWrapped(buffer);

		ImGui::End();

		// Show the demo window, remove later
		ImGui::ShowDemoWindow(&show_demo_window);

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);

		if(keep_time != -1) break;
	}

	std::cout << keep_time << std::endl;

	// Cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
