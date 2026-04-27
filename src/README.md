# 3DViewer_v2.0

## Структура проекта

```plaintext
3DViewer_v2.0/
├── src/                           # Исходный код приложения
│   ├── common/                    # Общие типы и утилиты
│   │   ├── point.h                # Структура Point (x, y, z)
│   │   ├── edge.h                 # Структура Edge (индексы вершин)
│   │   ├── color.h                # Структура Color (RGBA)
│   │   ├── transform.h            # Класс Transform (позиция, поворот, масштаб)
│   │   ├── matrix4.h              # RAII-обёртка для матриц 4x4 (s21_matrix+)
│   │   └── math_utils.h           # Вспомогательные математические функции
│   │
│   ├── scene/                      # Модель сцены и объекты
│   │   ├── scene_object.h          # Абстрактный базовый класс SceneObject
│   │   ├── mesh.h                  # Класс Mesh (сетка с вершинами и рёбрами)
│   │   ├── camera.h                # Класс Camera (камера, матрицы вида/проекции)
│   │   ├── light.h                 # Класс Light (источник света) – опционально
│   │   ├── scene.h                 # Класс Scene (корневые объекты, выделение, наблюдатели)
│   │   ├── scene_observer.h        # Интерфейс SceneObserver (для обновления UI)
│   │   └── scene_serializer.h      # Сериализация сцены в JSON
│   │
│   ├── loaders/                    # Загрузчики моделей из файлов
│   │   ├── iloader.h               # Интерфейс ILoader
│   │   ├── loader_factory.h        # Фабрика загрузчиков (синглтон)
│   │   └── obj_loader.h            # Загрузчик .obj (реализация ILoader)
│   │
│   ├── commands/                    # Команды для Undo/Redo
│   │   ├── command.h                # Абстрактный базовый класс Command
│   │   ├── command_manager.h        # Менеджер команд (стеки Undo/Redo)
│   │   ├── transform_commands.h     # Команды трансформации (Translate, Rotate, Scale, Reset)
│   │   ├── load_file_command.h      # Команда загрузки файла
│   │   ├── add_delete_commands.h    # Команды добавления/удаления объектов
│   │   ├── set_property_command.h   # Команда изменения свойств объекта
│   │   └── macro_command.h          # Макрокоманда (группа команд)
│   │
│   ├── controller/                   # Контроллер (тонкий слой)
│   │   └── controller.h              # Класс Controller (наследует QObject, валидация, асинхронность)
│   │
│   ├── facade/                       # Фасад – единая точка доступа для GUI
│   │   └── facade.h                   # Класс Facade (содержит Scene, Controller, CommandManager, Settings)
│   │
│   ├── settings/                      # Настройки приложения
│   │   └── settings.h                  # Класс Settings (загрузка/сохранение через QSettings, сигналы)
│   │
│   ├── renderer/                       # Рендерер и OpenGL обёртки
│   │   ├── renderer.h                   # Интерфейс Renderer
│   │   ├── opengl_renderer.h            # Реализация OpenGLRenderer
│   │   ├── gl_vertex_array.h            # RAII-обёртка для VAO
│   │   ├── gl_buffer.h                  # RAII-обёртка для VBO/EBO
│   │   └── gl_shader.h                  # RAII-обёртка для шейдерных программ
│   │
│   ├── strategies/                      # Стратегии рендеринга
│   │   ├── render_strategy.h            # Интерфейс RenderStrategy
│   │   ├── wireframe_strategy.h         # Отрисовка рёбер
│   │   ├── vertex_strategy.h            # Отрисовка вершин
│   │   ├── projection_strategy.h        # Установка матриц проекции и вида
│   │   └── shading_strategy.h           # (Опционально) закраска с освещением
│   │
│   ├── recorder/                         # Запись GIF и изображений
│   │   ├── gif_recorder.h                # Класс GifRecorder (RAII-обёртка для записи GIF)
│   │   └── image_saver.h                 # Утилиты для сохранения BMP/JPEG
│   │
│   ├── view/                              # Графический интерфейс (Qt)
│   │   ├── main_window.h                  # Главное окно (MainWindow)
│   │   ├── gl_widget.h                    # Виджет OpenGL (GLWidget)
│   │   ├── scene_tree_widget.h            # Дерево сцены (SceneTreeWidget)
│   │   ├── property_panel.h                # Панель свойств (PropertyPanel)
│   │   └── transform_gizmo.h               # (Опционально) интерактивный Gizmo
│   │
│   └── main.cpp                            # Точка входа в приложение
│
├── tests/                               # Модульные тесты (Google Test)
│   ├── test_main.cpp                     # Запуск тестов
│   ├── test_scene.cpp                     # Тесты Scene, SceneObject, Mesh
│   ├── test_loader.cpp                    # Тесты загрузчиков (ObjLoader)
│   ├── test_commands.cpp                  # Тесты команд и CommandManager
│   ├── test_matrix.cpp                    # Тесты Matrix4 (RAII-обёртка)
│   └── test_transform.cpp                  # Тесты аффинных преобразований
│
├── Makefile                             # Сборочный файл (цели all, install, uninstall, clean, dvi, dist, tests)
├── Doxyfile                             # Конфигурация Doxygen для генерации документации
├── README.md                            # Описание проекта, сборка, использование
└── .gitignore                           # Игнорируемые файлы (включая тяжёлые .obj >10 МБ)