#ifndef QRETRO_INPUT_BACKEND_GAMEPAD_H
#define QRETRO_INPUT_BACKEND_GAMEPAD_H

#include "QRetroInputBackend.h"

#if QRETRO_HAVE_GAMEPAD

#include <QGamepad>
#include <QGamepadManager>

#include "QRetroInput.h"

class QRetroInputBackendQGamepad : public QRetroInputBackend
{
  Q_OBJECT

public:
  explicit QRetroInputBackendQGamepad(QObject *parent = nullptr);

  void init(QRetroInputJoypad *joypads, unsigned maxUsers) override;

  // QGamepad is signal-driven; nothing to do at poll time.
  void poll() override {}

private slots:
  void onGamepadConnected(int deviceId);
  void onGamepadDisconnected(int deviceId);

private:
  // Parallel array to the joypads; index == port. Owned by this backend.
  QGamepad *m_Gamepads[QRETRO_INPUT_DEFAULT_MAX_JOYPADS] = {};
  QRetroInputJoypad *m_Joypads = nullptr;
  unsigned m_MaxUsers = 0;

  void connectPort(unsigned port, int deviceId);
};

#endif
#endif
