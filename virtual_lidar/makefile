CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3
LIBS = -lgmp -lmpfr

# Nom de l'exécutable
TARGET = scan_lidar

# Fichier source
SOURCES = lidar_scan.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)