#include "QRetroCommon.h"
#include "QRetroLocation.h"

QRetroLocation::QRetroLocation(QObject* parent)
{
  setParent(parent);

#if QRETRO_HAVE_LOCATION
  m_InfoSource = QGeoPositionInfoSource::createDefaultSource(this);

  if (m_InfoSource)
  {
    connect(m_InfoSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
            this, SLOT(positionUpdated(QGeoPositionInfo)));
  }
#endif
}

QRetroLocation::~QRetroLocation()
{
#if QRETRO_HAVE_LOCATION
  delete m_InfoSource;
#endif
}

bool QRetroLocation::getPosition(double *lat, double *lon,
  double *horiz_accuracy, double *vert_accuracy, bool quiet)
{
  if (!m_Updated)
    *lat = *lon = *horiz_accuracy = *vert_accuracy = 0;
  else
  {
    *lat = m_Latitude;
    *lon = m_Longitude;
    *horiz_accuracy = m_HorizontalAccuracy;
    *vert_accuracy = m_VerticalAccuracy;

    if (!quiet)
      m_Updated = false;
  }

  return true;
}

#if QRETRO_HAVE_LOCATION
void QRetroLocation::positionUpdated(const QGeoPositionInfo &update)
{
  m_Latitude = update.coordinate().latitude();
  m_Longitude = update.coordinate().longitude();
  m_HorizontalAccuracy = update.attribute(QGeoPositionInfo::HorizontalAccuracy);
  m_VerticalAccuracy = update.attribute(QGeoPositionInfo::VerticalAccuracy);
  m_Updated = true;
}
#endif

void QRetroLocation::setPosition(double lat, double lon, double horiz_accuracy,
  double vert_accuracy)
{
  m_Latitude = lat;
  m_Longitude = lon;
  m_HorizontalAccuracy = horiz_accuracy;
  m_VerticalAccuracy = vert_accuracy;
  m_Updated = true;
  m_SetManually = true;
}

void QRetroLocation::setUpdateInterval(int ms)
{
#if QRETRO_HAVE_LOCATION
  if (m_InfoSource)
    m_InfoSource->setUpdateInterval(ms);
#else
  Q_UNUSED(ms)
#endif
}

bool QRetroLocation::startUpdates(void)
{
#if QRETRO_HAVE_LOCATION
  if (m_InfoSource)
    m_InfoSource->startUpdates();
  return true;
#else
  return false;
#endif
}

bool QRetroLocation::stopUpdates(void)
{
#if QRETRO_HAVE_LOCATION
  if (m_InfoSource)
    m_InfoSource->stopUpdates();
  return true;
#else
  return false;
#endif
}
