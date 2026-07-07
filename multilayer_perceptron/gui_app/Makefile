CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC $(shell pkg-config --cflags Qt5Widgets)
LDFLAGS = $(shell pkg-config --libs Qt5Widgets) -L../libmlp -lmlp

INCLUDES = -I. -I../libmlp/include/mlp

# Утилита moc
MOC = /usr/lib/qt5/bin/moc

SOURCES = main.cpp view/MainWindow.cpp controller/AppController.cpp
MOC_SRCS = view/moc_MainWindow.cpp controller/moc_AppController.cpp
OBJECTS = $(patsubst %.cpp,build/%.o,$(SOURCES) $(MOC_SRCS))
TARGET = mlp_gui

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Генерация moc-файлов
view/moc_MainWindow.cpp: view/MainWindow.h
	@mkdir -p build/view
	$(MOC) $< -o $@

controller/moc_AppController.cpp: controller/AppController.h
	@mkdir -p build/controller
	$(MOC) $< -o $@

# Удаление сгенерированных moc-файлов при очистке
clean:
	rm -rf build $(TARGET) view/moc_MainWindow.cpp controller/moc_AppController.cpp
