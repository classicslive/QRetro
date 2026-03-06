#ifndef QRETRO_CONFIG_H
#define QRETRO_CONFIG_H

#include <QMap>
#include <QTimer>
#include <QWidget>

#include "libretro.h"

class QFormLayout;
class QLabel;
class QRetro;

class QRetroConfig : public QWidget
{
  Q_OBJECT

public:
  QRetroConfig(QRetro *owner);

  retro_language getLanguage()     const { return m_Language; }
  bool           integerScaling()  const { return m_IntegerScaling; }
  bool           bilinearFilter()  const { return m_BilinearFilter; }

  bool  spoofAccelEnabled()  const { return m_SpoofAccelEnabled; }
  float spoofAccelX()        const { return m_SpoofAccel[0]; }
  float spoofAccelY()        const { return m_SpoofAccel[1]; }
  float spoofAccelZ()        const { return m_SpoofAccel[2]; }

  bool  spoofGyroEnabled()   const { return m_SpoofGyroEnabled; }
  float spoofGyroX()         const { return m_SpoofGyro[0]; }
  float spoofGyroY()         const { return m_SpoofGyro[1]; }
  float spoofGyroZ()         const { return m_SpoofGyro[2]; }

  bool  spoofIllumEnabled()  const { return m_SpoofIllumEnabled; }
  float spoofIllum()         const { return m_SpoofIllum; }

  void update();

protected:
  void showEvent(QShowEvent *event) override;

public:
  void setAccelXHasBeenRead(bool v) { if (m_AccelAxisWidget[0]) m_AccelAxisWidget[0]->setEnabled(v); }
  void setAccelYHasBeenRead(bool v) { if (m_AccelAxisWidget[1]) m_AccelAxisWidget[1]->setEnabled(v); }
  void setAccelZHasBeenRead(bool v) { if (m_AccelAxisWidget[2]) m_AccelAxisWidget[2]->setEnabled(v); }
  void setGyroXHasBeenRead (bool v) { if (m_GyroAxisWidget[0])  m_GyroAxisWidget[0] ->setEnabled(v); }
  void setGyroYHasBeenRead (bool v) { if (m_GyroAxisWidget[1])  m_GyroAxisWidget[1] ->setEnabled(v); }
  void setGyroZHasBeenRead (bool v) { if (m_GyroAxisWidget[2])  m_GyroAxisWidget[2] ->setEnabled(v); }
  void setIllumHasBeenRead (bool v) { if (m_IllumValueWidget)   m_IllumValueWidget  ->setEnabled(v); }

signals:
  void integerScalingChanged(bool value);
  void bilinearFilterChanged(bool value);
  void audioEnabledChanged(bool value);

  void spoofAccelChanged(bool enabled, float x, float y, float z);
  void spoofGyroChanged(bool enabled, float x, float y, float z);
  void spoofIllumChanged(bool enabled, float value);

  void spoofLocationChanged(bool enabled, double lat, double lon,
                            double hAcc, double vAcc);

private:
  void load();
  void save();

  retro_language m_Language       = RETRO_LANGUAGE_ENGLISH;
  bool           m_IntegerScaling = false;
  bool           m_BilinearFilter = true;
  bool           m_AudioEnabled   = true;

  bool  m_SpoofAccelEnabled = false;
  float m_SpoofAccel[3]     = {0, 0, 0};

  bool  m_SpoofGyroEnabled  = false;
  float m_SpoofGyro[3]      = {0, 0, 0};

  bool  m_SpoofIllumEnabled = false;
  float m_SpoofIllum        = 0;

  bool   m_SpoofLocationEnabled = false;
  double m_SpoofLat             = 0;
  double m_SpoofLon             = 0;
  double m_SpoofHAcc            = 0;
  double m_SpoofVAcc            = 0;

  QRetro    *m_Owner      = nullptr;
  QLabel    *m_DescLabel  = nullptr;
  QTimer    *m_SaveTimer  = nullptr;
  QString    m_Filename;
  QStringList m_ProcSymbols;

  QWidget *m_AccelAxisWidget[3] = {nullptr, nullptr, nullptr};
  QWidget *m_GyroAxisWidget[3]  = {nullptr, nullptr, nullptr};
  QWidget *m_IllumValueWidget   = nullptr;

  QLabel  *m_AccelEnabledLabel  = nullptr;
  QLabel  *m_AccelRateLabel     = nullptr;
  QLabel  *m_GyroEnabledLabel   = nullptr;
  QLabel  *m_GyroRateLabel      = nullptr;
  QLabel  *m_IllumEnabledLabel  = nullptr;
  QLabel  *m_IllumRateLabel     = nullptr;

  QLabel  *m_LocationStateLabel    = nullptr;
  QLabel  *m_LocationIntervalLabel = nullptr;
  QLabel  *m_LocationDistLabel     = nullptr;
  QWidget *m_LocationSpoofWidgets[4] = {nullptr, nullptr, nullptr, nullptr};

  QFormLayout        *m_LedForm        = nullptr;
  QLabel             *m_LedEmptyLabel  = nullptr;
  QMap<int, QLabel*>  m_LedLabels;

  QLabel *m_MemDataPtrLabel[4]  = {nullptr, nullptr, nullptr, nullptr};
  QLabel *m_MemDataSizeLabel[4] = {nullptr, nullptr, nullptr, nullptr};

  QFormLayout *m_MemMapsForm       = nullptr;
  int          m_MemMapsShownCount = -1;

  QLabel *m_CoreAchievementsLabel   = nullptr;
  QLabel *m_CorePerfLevelLabel      = nullptr;
  QLabel *m_CorePixelFormatLabel    = nullptr;
  QLabel *m_CoreSerializationLabel  = nullptr;
  QLabel *m_CoreSupportsNoGameLabel = nullptr;
};

#endif
