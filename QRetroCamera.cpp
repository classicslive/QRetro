#if QRETRO_HAVE_CAMERA
#include <QAbstractVideoSurface>
#include <QCamera>
#include <QVideoSurfaceFormat>
#endif

#include "QRetroCamera.h"

#if QRETRO_HAVE_CAMERA
bool QRetroCameraSurface::present(const QVideoFrame &frame)
{
  auto copy = QVideoFrame(frame);

  if (m_RawCb)
    m_RawCb(reinterpret_cast<const uint32_t*>(copy.bits()),
            static_cast<unsigned>(copy.width()),
            static_cast<unsigned>(copy.height()),
            static_cast<unsigned>(copy.bytesPerLine()));

  return true;
}

QList<QVideoFrame::PixelFormat> QRetroCameraSurface::supportedPixelFormats(
  QAbstractVideoBuffer::HandleType type) const
{
  Q_UNUSED(type)
  return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
}
#endif

QRetroCamera::~QRetroCamera()
{
  if (m_Initted && m_Callback.deinitialized)
    m_Callback.deinitialized();
#if QRETRO_HAVE_CAMERA
  delete m_Camera;
  delete m_Surface;
#endif
}

void QRetroCamera::init(const retro_camera_callback* cb)
{
  if (cb)
  {
    m_Callback = *cb;
#if QRETRO_HAVE_CAMERA
    m_Camera = new QCamera();
#endif
    setImage(QImage("://camera.png"));
    m_Initted = true;

    if (m_Callback.initialized)
      m_Callback.initialized();
  }
}

bool QRetroCamera::start(void)
{
#if QRETRO_HAVE_CAMERA
  if (m_Camera && !m_Surface)
  {
    QVideoSurfaceFormat format(QSize(static_cast<int>(m_Callback.width), static_cast<int>(m_Callback.height)),
                               QVideoFrame::Format_RGB32,
                               /**
                                * @todo Support OpenGL texture
                                * m_Callback.frame_opengl_texture ? QAbstractVideoBuffer::GLTextureHandle :
                                *                                   QAbstractVideoBuffer::NoHandle);
                                */
                               QAbstractVideoBuffer::NoHandle);

    m_Surface = new QRetroCameraSurface(nullptr);
    m_Surface->start(format);
    m_Camera->setViewfinder(m_Surface);
  }
  if (m_Surface)
  {
    m_Camera->start();

    return true;
  }
#endif

  return true;
}

bool QRetroCamera::setImage(const QImage &image, bool quiet)
{
  if (!image.isNull())
  {
    m_SpoofImage = image.scaled(static_cast<int>(m_Callback.width),
                                static_cast<int>(m_Callback.height));
    if (!quiet)
      m_Spoofing = true;

    return true;
  }
  else
    return false;
}

void QRetroCamera::stop(void)
{
#if QRETRO_HAVE_CAMERA
  if (m_Camera)
    m_Camera->stop();
#endif
}

void QRetroCamera::update(void)
{
  if (m_Initted &&
      !m_SpoofImage.isNull() &&
      m_Spoofing &&
      m_Callback.frame_raw_framebuffer)
    m_Callback.frame_raw_framebuffer(reinterpret_cast<uint32_t*>(m_SpoofImage.bits()),
                                     static_cast<unsigned>(m_SpoofImage.width()),
                                     static_cast<unsigned>(m_SpoofImage.height()),
                                     static_cast<unsigned>(m_SpoofImage.bytesPerLine()));
}
