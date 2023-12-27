#ifndef QRETRO_CAMERA_H
#define QRETRO_CAMERA_H

#include <QImage>

#include <libretro.h>

#if QRETRO_HAVE_CAMERA
#include <QAbstractVideoSurface>
QT_FORWARD_DECLARE_CLASS(QCamera);
QT_FORWARD_DECLARE_CLASS(QVideoFrame);

class QRetroCameraSurface : public QAbstractVideoSurface
{
  Q_OBJECT

public:
  /*QRetroCameraSurface(const retro_camera_callback &cb, QObject *parent = nullptr) :
    QAbstractVideoSurface(parent)
  {
    m_RawCb = cb.frame_raw_framebuffer;
    m_OpenGlCb = cb.frame_opengl_texture;
  }*/

  QRetroCameraSurface(QObject *parent) : QAbstractVideoSurface{parent} {}

  bool present(const QVideoFrame &frame) override;

  QList<QVideoFrame::PixelFormat> supportedPixelFormats(
    QAbstractVideoBuffer::HandleType type) const override;

  QImage *image(void) { return &m_Image; }

  bool start(const QVideoSurfaceFormat& format) override { return QAbstractVideoSurface::start(format); }

  void stop() override { QAbstractVideoSurface::stop(); }

private:
  retro_camera_frame_raw_framebuffer_t m_RawCb = nullptr;
  retro_camera_frame_opengl_texture_t m_OpenGlCb = nullptr;
  QImage m_Image;
};
#endif

class QRetroCamera
{
public:
  ~QRetroCamera();

  void init(const retro_camera_callback* cb);
  bool start(void);
  void stop(void);

  /**
   * Called every frame before retro_run to update the spoofed camera, if
   * available.
   */
  void update(void);

  /**
   * Sets an image to be used as a virtual camera feed to the libretro core.
   */
  bool setImage(const QImage &image, bool quiet = false);

  bool spoofing(void) { return m_Spoofing; }

private:
  retro_camera_callback m_Callback = { 0, 0, 0, nullptr, nullptr, nullptr,
                                       nullptr, nullptr, nullptr };
  bool m_Initted = false;
  bool m_Spoofing = false;
  QImage m_SpoofImage;
#if QRETRO_HAVE_CAMERA
  QRetroCameraSurface *m_Surface = nullptr;
  QCamera *m_Camera = nullptr;
  QVideoFrame m_Frame;
#endif
};

#endif
