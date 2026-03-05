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
  enum State
  {
    Uninitialized = 0,
    Started,
    Stopped,
  };

  QRetroLocation(QObject* parent);
  ~QRetroLocation();

  bool getPosition(double *lat, double *lon, double *horiz_accuracy,
    double *vert_accuracy, bool quiet = false);
  void setPosition(double lat, double lon, double horiz_accuracy = 0,
    double vert_accuracy = 0);
  void setInterval(unsigned ms, unsigned dist);
  bool startUpdates(void);
  bool stopUpdates(void);

  /* Spoof */
  void setSpoofEnabled(bool v) { m_SpoofEnabled = v; }
  void setSpoofLat(double v) { m_Latitude = v; }
  void setSpoofLon(double v) { m_Longitude = v; }
  void setSpoofHAcc(double v) { m_HorizontalAccuracy = v; }
  void setSpoofVAcc(double v) { m_VerticalAccuracy = v; }

  /* State getters */
  bool spoofEnabled(void) const { return m_SpoofEnabled; }
  double latitude(void) const { return m_Latitude; }
  double longitude(void) const { return m_Longitude; }
  double horizAccuracy(void) const { return m_HorizontalAccuracy; }
  double vertAccuracy(void) const { return m_VerticalAccuracy; }
  unsigned millisecondInterval(void) const { return m_MillisecondInterval; }
  unsigned distanceInterval(void) const { return m_DistanceInterval; }
  QRetroLocation::State state(void) const { return m_State; }

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
  unsigned m_MillisecondInterval = 0;
  unsigned m_DistanceInterval = 0;
  bool m_Updated = false;
  bool m_SetManually = false;
  bool m_SpoofEnabled = false;
  QRetroLocation::State m_State = QRetroLocation::Uninitialized;
};

#endif
