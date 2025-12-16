# copied from
# https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/Makefile

CXX = g++
IMGUI_DIR = imgui
CXXFLAGS = -g -lGL -lGLEW -lglfw -I imgui -I imgui/backends/
SOURCES = main.cpp $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main: $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f main
