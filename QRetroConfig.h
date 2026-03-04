#ifndef QRETRO_CONFIG_H
#define QRETRO_CONFIG_H

#include <QTimer>
#include <QWidget>

#include "libretro.h"

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

  bool  fakeAccelEnabled()  const { return m_FakeAccelEnabled; }
  float fakeAccelX()        const { return m_FakeAccel[0]; }
  float fakeAccelY()        const { return m_FakeAccel[1]; }
  float fakeAccelZ()        const { return m_FakeAccel[2]; }

  bool  fakeGyroEnabled()   const { return m_FakeGyroEnabled; }
  float fakeGyroX()         const { return m_FakeGyro[0]; }
  float fakeGyroY()         const { return m_FakeGyro[1]; }
  float fakeGyroZ()         const { return m_FakeGyro[2]; }

  bool  fakeIllumEnabled()  const { return m_FakeIllumEnabled; }
  float fakeIllum()         const { return m_FakeIllum; }

  void update();

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

  void fakeAccelChanged(bool enabled, float x, float y, float z);
  void fakeGyroChanged(bool enabled, float x, float y, float z);
  void fakeIllumChanged(bool enabled, float value);

private:
  void load();
  void save();

  retro_language m_Language       = RETRO_LANGUAGE_ENGLISH;
  bool           m_IntegerScaling = false;
  bool           m_BilinearFilter = true;
  bool           m_AudioEnabled   = true;

  bool  m_FakeAccelEnabled = false;
  float m_FakeAccel[3]     = {0, 0, 0};

  bool  m_FakeGyroEnabled  = false;
  float m_FakeGyro[3]      = {0, 0, 0};

  bool  m_FakeIllumEnabled = false;
  float m_FakeIllum        = 0;

  QRetro    *m_Owner      = nullptr;
  QLabel    *m_DescLabel  = nullptr;
  QTimer    *m_SaveTimer  = nullptr;
  QString    m_Filename;

  QWidget *m_AccelAxisWidget[3] = {nullptr, nullptr, nullptr};
  QWidget *m_GyroAxisWidget[3]  = {nullptr, nullptr, nullptr};
  QWidget *m_IllumValueWidget   = nullptr;
};

#endif
