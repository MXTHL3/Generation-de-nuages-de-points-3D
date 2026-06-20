CXX = g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
CXXFLAGS += $(shell pkg-config --cflags gtkmm-3.0 epoxy pdal libpng spdlog)
LDFLAGS  := $(shell pkg-config --libs   gtkmm-3.0 epoxy pdal libpng spdlog)
LDFLAGS  += -lgmp -lmpfr -lfmt

# Nom de l'exécutable
TARGET = scan_lidar

# Fichier source
SOURCES = main.cpp \
		entity.cpp \
		lidar_factory.cpp \
		lidar.cpp \
		point_cloud_exporter.cpp \
		pose.cpp \
		scene.cpp \
		scene_utils.cpp \
		asset_manager.cpp \
		noise_model.cpp \
		pipeline.cpp \
		lidar_scanner.cpp \
		horizontal_step.cpp \
		scan_strategy.cpp \
		object.cpp \
		tlas_primitive.cpp \
		manifest.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)