#include "gif_recorder.h"

// Единственное место, где подключается gif.h
extern "C" {
#include "3rd_party/gif.h"
}

#include <QDebug>
#include <QPainter>

namespace s21 {

GifRecorder::GifRecorder(QOpenGLWidget* widget, QObject* parent)
    : QObject(parent), widget_(widget), gifWriter_(nullptr), recording_(false) {
  Q_ASSERT(widget_);
}

GifRecorder::~GifRecorder() {
  if (recording_) stop();
}

bool GifRecorder::startRecording(const QString& fileName, int width, int height,
                                 int fps, int durationSec) {
  if (recording_ || !widget_) return false;

  fileName_ = fileName;
  width_ = width;
  height_ = height;
  frameIntervalMs_ = 1000 / fps;
  totalFrames_ = fps * durationSec;
  framesWritten_ = 0;

  // Создаём объект GifWriter (полный тип известен здесь)
  GifWriter* writer = new GifWriter();
  if (!GifBegin(writer, fileName.toUtf8().constData(), width, height,
                frameIntervalMs_ / 10, 8, false)) {
    delete writer;
    return false;
  }
  gifWriter_ = static_cast<void*>(writer);

  recording_ = true;
  connect(&timer_, &QTimer::timeout, this, &GifRecorder::captureFrame);
  timer_.start(frameIntervalMs_);
  elapsed_.start();
  return true;
}

void GifRecorder::stop() {
  if (!recording_) return;
  timer_.stop();
  recording_ = false;

  bool success = false;
  QString error;
  if (gifWriter_) {
    GifWriter* writer = static_cast<GifWriter*>(gifWriter_);
    success = GifEnd(writer);
    if (!success) error = "Failed to finalize GIF";
    delete writer;
    gifWriter_ = nullptr;
  } else {
    error = "No active recording";
  }

  emit recordingFinished(success, error);
}

void GifRecorder::captureFrame() {
  if (!recording_ || !widget_ || !gifWriter_) return;

  QImage frame = widget_->grabFramebuffer();
  QImage scaled = frame.scaled(width_, height_, Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
  QImage rgba = scaled.convertToFormat(QImage::Format_RGBA8888);

  GifWriter* writer = static_cast<GifWriter*>(gifWriter_);
  GifWriteFrame(writer, rgba.constBits(), width_, height_,
                frameIntervalMs_ / 10, 8, false);
  framesWritten_++;

  if (framesWritten_ >= totalFrames_ ||
      elapsed_.elapsed() >= totalFrames_ * frameIntervalMs_) {
    stop();
  }
}

}  // namespace s21