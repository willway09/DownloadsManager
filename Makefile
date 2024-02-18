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
IMGUI_FREETYPE_NAME = $(shell ls ${IMGUI_DIR}misc/freetype| grep cpp | sed "s/\\.cpp//g")
IMGUI_FREETYPE_SRC = $(addsuffix .cpp, $(addprefix ${IMGUI_DIR}misc/freetype/, ${IMGUI_FREETYPE_NAME}))
IMGUI_FREETYPE_OBJ = $(addsuffix .o, $(addprefix ${OBJ}, ${IMGUI_FREETYPE_NAME}))

CXX=g++
CPPFLAGS = -I${IMGUI_DIR} -I${IMGUI_DIR}backends/ -I/usr/include/freetype2 -g -DIMGUI_USE_WCHAR32 -DIMGUI_ENABLE_FREETYPE

main: main.cpp lib/libimgui.a lib/libglfw3.a
	$(CXX) $(CPPFLAGS) -o $@ $< lib/libimgui.a lib/libglfw3.a -lGL -lfreetype

# Only build if not already built
lib/libimgui.a: ${OBJ} ${LIB} ${IMGUI_OBJ} ${IMGUI_BACKENDS_OBJ} ${IMGUI_FREETYPE_OBJ}
	ar -ru ${LIB}libimgui.a ${IMGUI_OBJ} ${IMGUI_BACKENDS_OBJ} ${IMGUI_FREETYPE_OBJ}

# Compile ImGUI Sources
${IMGUI_OBJ}: ${OBJ}%.o: ${IMGUI_DIR}%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<
${IMGUI_BACKENDS_OBJ}: ${OBJ}%.o: ${IMGUI_DIR}backends/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<
${IMGUI_FREETYPE_OBJ}: ${OBJ}%.o: ${IMGUI_DIR}misc/freetype/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

# Only build if not already built
lib/libglfw3.a: ${OBJ} ${LIB}
ifeq ($(shell ls lib | grep libglfw3.a),libglfw3.a)
else
	mkdir -p ${OBJ}glfw
	cmake -B ${OBJ}glfw -S glfw
	make -C ${OBJ}glfw
	cp ${OBJ}glfw/src/libglfw3.a ${LIB}
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
