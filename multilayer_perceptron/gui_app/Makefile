CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC $(shell pkg-config --cflags Qt5Widgets)
LDFLAGS = $(shell pkg-config --libs Qt5Widgets) -L../libmlp -lmlp

INCLUDES = -I. -I../libmlp/include/mlp

# Утилита moc
MOC = /usr/lib/qt5/bin/moc

SOURCES = main.cpp view/MainWindow.cpp controller/AppController.cpp view/DrawWidget.cpp view/ErrorGraph.cpp
MOC_SRCS = view/moc_MainWindow.cpp controller/moc_AppController.cpp view/moc_DrawWidget.cpp view/moc_ErrorGraph.cpp
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

view/moc_DrawWidget.cpp: view/DrawWidget.h
	@mkdir -p $(dir $@)
	$(MOC) $< -o $@i

view/moc_ErrorGraph.cpp: view/ErrorGraph.h
	@mkdir -p $(dir $@)
	$(MOC) $< -o $@

# Удаление сгенерированных moc-файлов при очистке
clean:
	rm -rf build $(TARGET) view/moc_MainWindow.cpp controller/moc_AppController.cpp view/moc_DrawWidget.cpp view/moc_ErrorGraph.cpp
