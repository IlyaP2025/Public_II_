#ifndef S21_GIF_RECORDER_H
#define S21_GIF_RECORDER_H

#include <QElapsedTimer>
#include <QImage>
#include <QObject>
#include <QOpenGLWidget>
#include <QTimer>

namespace s21 {

/**
 * @brief Класс для записи GIF-анимации из OpenGL-виджета.
 *        Использует библиотеку gif.h, но скрывает её детали за void*.
 */
class GifRecorder : public QObject {
  Q_OBJECT
 public:
  explicit GifRecorder(QOpenGLWidget* widget, QObject* parent = nullptr);
  ~GifRecorder();

  /**
   * @brief Начать запись.
   * @param fileName Имя файла для сохранения.
   * @param width Ширина кадра.
   * @param height Высота кадра.
   * @param fps Частота кадров.
   * @param durationSec Длительность записи в секундах.
   * @return true, если запись успешно начата.
   */
  bool startRecording(const QString& fileName, int width, int height, int fps,
                      int durationSec);

  /**
   * @brief Остановить запись досрочно и сохранить файл.
   */
  void stop();

  /**
   * @brief Проверить, идёт ли запись.
   */
  bool isRecording() const { return recording_; }

 signals:
  /**
   * @brief Сигнал завершения записи.
   * @param success true, если GIF успешно создан.
   * @param error Сообщение об ошибке (пустое при success).
   */
  void recordingFinished(bool success, const QString& error);

 private slots:
  void captureFrame();  // вызывается по таймеру для захвата кадра

 private:
  QOpenGLWidget* widget_;  // виджет, откуда захватываем кадры
  void* gifWriter_;  // указатель на GifWriter (скрытый тип)
  QString fileName_;     // имя файла для сохранения
  int frameIntervalMs_;  // интервал между кадрами (мс)
  int totalFrames_;  // общее количество кадров для записи
  int framesWritten_;      // сколько уже записано
  QTimer timer_;           // таймер захвата кадров
  QElapsedTimer elapsed_;  // для контроля времени
  int width_;  // ширина кадра (после масштабирования)
  int height_;      // высота кадра
  bool recording_;  // флаг записи
};

}  // namespace s21

#endif  // S21_GIF_RECORDER_H