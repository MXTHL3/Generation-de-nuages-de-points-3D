CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
CXXFLAGS += $(shell pkg-config --cflags gtkmm-3.0 epoxy pdal libpng)
LDFLAGS  := $(shell pkg-config --libs   gtkmm-3.0 epoxy pdal libpng)
LDFLAGS  += -lgmp -lmpfr -lspdlog -lfmt

TARGET   := app

SRCS := main.cpp \
        main_window.cpp \
        gl.cpp \
        gl_shaders_utils.cpp \
        cgal.cpp \
        cgal_shape.cpp \
        handle_file.cpp \
        model_marker.cpp \
        pose.cpp \
        lidar.cpp \
        lidar_factory.cpp \
        entity.cpp \
        scene.cpp \
        point_cloud_exporter.cpp \
        menu_items_actions.cpp \
        noise_model.cpp \
        asset_manager.cpp \
        scene_utils.cpp \
        pipeline.cpp \
        horizontal_step.cpp \
        lidar_scanner.cpp \
        scan_strategy.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean