#if QRETRO_HAVE_CAMERA
#include <QCamera>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QVideoSink>
#else
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#endif
#endif

#include "QRetroCamera.h"

#if QRETRO_HAVE_CAMERA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

QRetroCameraSurface::QRetroCameraSurface(QObject *parent)
  : QObject(parent)
{
  m_Sink = new QVideoSink(this);
  connect(m_Sink, &QVideoSink::videoFrameChanged, this, &QRetroCameraSurface::onFrameChanged);
}

QImage *QRetroCameraSurface::image(void)
{
  return &m_Image;
}

void QRetroCameraSurface::onFrameChanged(const QVideoFrame &frame)
{
  if (!m_RawCb)
    return;

  QVideoFrame copy = frame;
  if (!copy.map(QVideoFrame::ReadOnly))
    return;

  m_RawCb(reinterpret_cast<const uint32_t *>(copy.bits(0)), static_cast<unsigned>(copy.width()),
    static_cast<unsigned>(copy.height()), static_cast<unsigned>(copy.bytesPerLine(0)));

  copy.unmap();
}

#else

QRetroCameraSurface::QRetroCameraSurface(QObject *parent)
  : QAbstractVideoSurface{ parent }
{
}

QImage *QRetroCameraSurface::image(void)
{
  return &m_Image;
}

bool QRetroCameraSurface::start(const QVideoSurfaceFormat &format)
{
  return QAbstractVideoSurface::start(format);
}

void QRetroCameraSurface::stop()
{
  QAbstractVideoSurface::stop();
}

bool QRetroCameraSurface::present(const QVideoFrame &frame)
{
  auto copy = QVideoFrame(frame);

  if (m_RawCb)
    m_RawCb(reinterpret_cast<const uint32_t *>(copy.bits()), static_cast<unsigned>(copy.width()),
      static_cast<unsigned>(copy.height()), static_cast<unsigned>(copy.bytesPerLine()));

  return true;
}

QList<QVideoFrame::PixelFormat> QRetroCameraSurface::supportedPixelFormats(
  QAbstractVideoBuffer::HandleType type) const
{
  Q_UNUSED(type)
  return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
}

#endif
#endif

QRetroCamera::~QRetroCamera()
{
  if (m_Initted && m_Callback.deinitialized)
    m_Callback.deinitialized();
#if QRETRO_HAVE_CAMERA
  delete m_Camera;
  delete m_Surface;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  delete m_Session;
#endif
#endif
}

void QRetroCamera::init(const retro_camera_callback *cb)
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
    m_Surface = new QRetroCameraSurface(nullptr);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_Session = new QMediaCaptureSession();
    m_Session->setCamera(m_Camera);
    m_Session->setVideoSink(m_Surface->sink());
#else
    QVideoSurfaceFormat format(
      QSize(static_cast<int>(m_Callback.width), static_cast<int>(m_Callback.height)),
      QVideoFrame::Format_RGB32, QAbstractVideoBuffer::NoHandle);
    m_Surface->start(format);
    m_Camera->setViewfinder(m_Surface);
#endif
  }
  if (m_Surface)
  {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_Camera->start();
#else
    m_Camera->start();
#endif
    return true;
  }
#endif

  return true;
}

bool QRetroCamera::setImage(const QImage &image, bool quiet)
{
  if (!image.isNull())
  {
    m_SpoofImage =
      image.scaled(static_cast<int>(m_Callback.width), static_cast<int>(m_Callback.height));
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
  if (m_Initted && !m_SpoofImage.isNull() && m_Spoofing && m_Callback.frame_raw_framebuffer)
    m_Callback.frame_raw_framebuffer(reinterpret_cast<uint32_t *>(m_SpoofImage.bits()),
      static_cast<unsigned>(m_SpoofImage.width()), static_cast<unsigned>(m_SpoofImage.height()),
      static_cast<unsigned>(m_SpoofImage.bytesPerLine()));
}
