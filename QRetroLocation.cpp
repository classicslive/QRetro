#include "QRetroCommon.h"
#include "QRetroLocation.h"

QRetroLocation::QRetroLocation(QObject* parent)
{
  setParent(parent);
  m_InfoSource = QGeoPositionInfoSource::createDefaultSource(this);

  if (m_InfoSource)
  {
    connect(m_InfoSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
            this, SLOT(positionUpdated(QGeoPositionInfo)));
  }
}

QRetroLocation::~QRetroLocation()
{
  delete m_InfoSource;
}

bool QRetroLocation::getPosition(double *lat, double *lon,
  double *horiz_accuracy, double *vert_accuracy, bool quiet)
{
  if (!m_InfoSource && !m_SetManually)
    return false;
  else if (!m_Updated)
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

void QRetroLocation::positionUpdated(const QGeoPositionInfo &update)
{
  m_Latitude = update.coordinate().latitude();
  m_Longitude = update.coordinate().longitude();
  m_HorizontalAccuracy = update.attribute(QGeoPositionInfo::HorizontalAccuracy);
  m_VerticalAccuracy = update.attribute(QGeoPositionInfo::VerticalAccuracy);
  m_Updated = true;
}

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
