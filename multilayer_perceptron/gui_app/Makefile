CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC $(shell pkg-config --cflags Qt5Widgets)
LDFLAGS = $(shell pkg-config --libs Qt5Widgets) -L../libmlp -lmlp

INCLUDES = -I../libmlp/include/mlp

SOURCES = main.cpp view/MainWindow.cpp controller/AppController.cpp
OBJECTS = $(patsubst %.cpp,build/%.o,$(SOURCES))
TARGET = mlp_gui

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf build $(TARGET)
