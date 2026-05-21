CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3
LIBS = -lgmp -lmpfr

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
		noise_model.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)