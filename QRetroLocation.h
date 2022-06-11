#ifndef QRETRO_LOCATION_H
#define QRETRO_LOCATION_H

#include <QGeoPositionInfoSource>

class QRetroLocation : public QObject
{
public:
  QRetroLocation(QObject* parent);
  ~QRetroLocation();

  bool getPosition(double *lat, double *lon, double *horiz_accuracy,
    double *vert_accuracy, bool quiet = false);
  void setPosition(double lat, double lon, double horiz_accuracy = 0,
    double vert_accuracy = 0);
  QGeoPositionInfoSource* infoSource() { return m_InfoSource; }

public slots:
  void positionUpdated(const QGeoPositionInfo &update);

private:
  QGeoPositionInfoSource *m_InfoSource;
  double m_Latitude = 0;
  double m_Longitude = 0;
  double m_HorizontalAccuracy = 0;
  double m_VerticalAccuracy = 0;
  bool m_Updated = false;
  bool m_SetManually = false;
};

#endif
