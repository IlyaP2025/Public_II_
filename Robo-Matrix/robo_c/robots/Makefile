CC = gcc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra -pedantic
LIBS = -lm

# Определяем цели по умолчанию
DEFAULT_TARGET = robot
TARGET = $(DEFAULT_TARGET)

# ОБНОВЛЕННЫЙ СПИСОК ИСХОДНЫХ ФАЙЛОВ
SOURCES = src/main.c src/robot.c src/lidar.c src/logic.c src/states.c src/state_types.c src/pid.c
HEADERS = src/robot.h src/lidar.h src/logic.h src/init_robot.h src/states.h src/state_types.h src/pid.h
OBJDIR = bin
OBJECTS = $(SOURCES:src/%.c=$(OBJDIR)/%.o)

# Проверяем, является ли цель специальной (не файловой)
ifneq ($(filter clean,$(MAKECMDGOALS)),)
    # Если вызываем clean, не меняем TARGET
else ifneq ($(MAKECMDGOALS),)
    # Если указана цель и она не пустая, используем её как имя бинарника
    TARGET = $(firstword $(MAKECMDGOALS))
endif

# Цель по умолчанию
.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(OBJDIR)/%.o: src/%.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(DEFAULT_TARGET) $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean
