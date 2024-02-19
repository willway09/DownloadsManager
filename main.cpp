#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "imgui/misc/freetype/imgui_freetype.h"
#include <stdio.h>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <pthread.h>

#include <string>
#include <iostream>
#include <queue>

#include <julia.h>

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void watcher_thread_cleanup(void*) {
	// Cleanup julia
	jl_atexit_hook(0);
	std::cout << "Cleaned up Julia" << std::endl;
}

std::queue<std::string> filenames;
pthread_mutex_t filenames_mutex;

void* watcher_thread(void*) {
	jl_init();

	jl_eval_string("include(\"watcher.jl\")");

	jl_function_t* get_next_download = jl_get_function(jl_main_module, "get_next_download");

	if(get_next_download == 0) {
		std::cerr << "Could not get get_next_download function pointer" << std::endl;
		std::exit(-1);
	}

	pthread_cleanup_push(watcher_thread_cleanup, nullptr);

	while(true) {
		pthread_testcancel();
		jl_array_t* filename_arr = (jl_array_t*)jl_call0(get_next_download);
		std::string filename = (char*)jl_array_data(filename_arr);

		pthread_mutex_lock(&filenames_mutex);
		filenames.push(filename);
		pthread_mutex_unlock(&filenames_mutex);
	}

	pthread_cleanup_pop(0);

	return nullptr;
};

int get_file_duration(std::string filename) {
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

		std::string text = "How long should  " + filename + " be kept for?";
		const char* ff = text.c_str();

		ImGui::Text(ff);

		if (ImGui::Button("1 Hour")) keep_time = 1;
		ImGui::SameLine();
		if (ImGui::Button("1 Day" )) keep_time = 24 * 1;
		ImGui::SameLine();
		if (ImGui::Button("3 Days")) keep_time = 24 * 3;
		ImGui::SameLine();
		if (ImGui::Button("7 Days")) keep_time = 24 * 7;
		ImGui::SameLine();
		if (ImGui::Button("Forever")) keep_time = 0;
		if (ImGui::Button("Stop")) keep_time = -2;

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

	// Cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);

	return keep_time;
}

// Main code
int main(int, char**) {
	pthread_mutex_init(&filenames_mutex, NULL);

	pthread_t watcher;

	int s = pthread_create(&watcher, NULL, watcher_thread, NULL);
	if(s != 0) {
		std::cerr << "Could not start watcher thread" << std::endl;
		return -1;
	}

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	while(true) {
		pthread_mutex_lock(&filenames_mutex);
		if(filenames.size() > 0) {
			std::string filename = filenames.front();
			filenames.pop();

			pthread_mutex_unlock(&filenames_mutex);
			
			int duration = get_file_duration(filename);
			std::cout << duration << std::endl;

			if(duration == -2) break;

		} else {
			pthread_mutex_unlock(&filenames_mutex);
		}

		usleep(1000);
	}

	glfwTerminate();

	pthread_cancel(watcher);

	usleep(1000000);

	return 0;
}
