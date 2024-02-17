OBJ = obj/
LIB = lib/
IMGUI_DIR = imgui/

# List the desired ImGUI backends for the build, take from the following list:
# allegro5 android dx9 dx10 dx11 dx12 glfw glut opengl2 opengl3 sdl2 sdl3 sdlrenderer2 sdlrenderer3 vulkan wgpu win32
IMGUI_BACKENDS = glfw opengl2

IMGUI_NAME = $(shell ls ${IMGUI_DIR}| grep cpp | sed "s/\\.cpp//g")
IMGUI_SRC = $(addsuffix .cpp, $(addprefix ${IMGUI_DIR}, ${IMGUI_NAME}))
IMGUI_OBJ = $(addsuffix .o, $(addprefix ${OBJ}, ${IMGUI_NAME}))
#IMGUI_BACKENDS_NAME = $(shell ls ${IMGUI_DIR}backends | grep cpp | sed "s/\\.cpp//g")
IMGUI_BACKENDS_NAME = $(addprefix imgui_impl_, ${IMGUI_BACKENDS})
IMGUI_BACKENDS_SRC = $(addsuffix .cpp, $(addprefix ${IMGUI_DIR}backends/, ${IMGUI_BACKENDS_NAME}))
IMGUI_BACKENDS_OBJ = $(addsuffix .o, $(addprefix ${OBJ}, ${IMGUI_BACKENDS_NAME}))

CXX=g++
CPPFLAGS = -I${IMGUI_DIR} -I${IMGUI_DIR}backends/

main: main.cpp lib/libimgui.a lib/libglfw.a
	$(CXX) $(CPPFLAGS) -o $@ $< lib/libimgui.a lib/liblglfw.a -lGL

# Only build if not already built
lib/libimgui.a: ${OBJ} ${LIB}
ifeq ($(shell ls lib | grep libimgui.a),libimgui.a)
else
	make -C . ${IMGUI_OBJ} ${IMGUI_BACKENDS_OBJ}
	ar -ru ${LIB}libimgui.a ${IMGUI_OBJ} ${IMGUI_BACKENDS_OBJ}

endif

# Compile ImGUI Sources
${IMGUI_OBJ}: ${OBJ}%.o: ${IMGUI_DIR}%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<
${IMGUI_BACKENDS_OBJ}: ${OBJ}%.o: ${IMGUI_DIR}backends/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

# Only build if not already built
lib/libglfw.a: ${OBJ} ${LIB}
ifeq ($(shell ls lib | grep libglfw.a),libglfw.a)
else
	mkdir ${OBJ}glfw
	make -C . ${IMGUI_OBJ} ${IMGUI_BACKENDS_OBJ}
	ar -ru ${LIB}liblibglfw.a ${IMGUI_OBJ} ${IMGUI_BACKENDS_OBJ}
endif

# Build Output Directories
${LIB}:
	mkdir ${LIB}
${OBJ}:
	mkdir ${OBJ}

# Clean
clean:
	rm -rf ${OBJ} main
cleanall: clean
	rm -rf lib
