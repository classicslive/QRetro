#ifndef QRETRO_LOCATION_H
#define QRETRO_LOCATION_H

#if QRETRO_HAVE_LOCATION
#include <QGeoPositionInfoSource>
#else
#include <QObject>
#endif

class QRetroLocation : public QObject
{
public:
  QRetroLocation(QObject* parent);
  ~QRetroLocation();

  bool getPosition(double *lat, double *lon, double *horiz_accuracy,
    double *vert_accuracy, bool quiet = false);
  void setPosition(double lat, double lon, double horiz_accuracy = 0,
    double vert_accuracy = 0);
  void setUpdateInterval(int ms);
  bool startUpdates(void);
  bool stopUpdates(void);

#if QRETRO_HAVE_LOCATION
public slots:
  void positionUpdated(const QGeoPositionInfo &update);
#endif

private:
#if QRETRO_HAVE_LOCATION
  QGeoPositionInfoSource *m_InfoSource;
#endif
  double m_Latitude = 0;
  double m_Longitude = 0;
  double m_HorizontalAccuracy = 0;
  double m_VerticalAccuracy = 0;
  bool m_Updated = false;
  bool m_SetManually = false;
};

#endif
