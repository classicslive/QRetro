#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "QRetroConfig.h"
#include "QRetro.h"

static const struct { const char *name; retro_language id; } k_languages[] = {
  { "English",               RETRO_LANGUAGE_ENGLISH             },
  { "Japanese",              RETRO_LANGUAGE_JAPANESE            },
  { "French",                RETRO_LANGUAGE_FRENCH              },
  { "Spanish",               RETRO_LANGUAGE_SPANISH             },
  { "German",                RETRO_LANGUAGE_GERMAN              },
  { "Italian",               RETRO_LANGUAGE_ITALIAN             },
  { "Dutch",                 RETRO_LANGUAGE_DUTCH               },
  { "Portuguese (Brazil)",   RETRO_LANGUAGE_PORTUGUESE_BRAZIL   },
  { "Portuguese (Portugal)", RETRO_LANGUAGE_PORTUGUESE_PORTUGAL },
  { "Russian",               RETRO_LANGUAGE_RUSSIAN             },
  { "Korean",                RETRO_LANGUAGE_KOREAN              },
  { "Chinese (Traditional)", RETRO_LANGUAGE_CHINESE_TRADITIONAL },
  { "Chinese (Simplified)",  RETRO_LANGUAGE_CHINESE_SIMPLIFIED  },
  { "Esperanto",             RETRO_LANGUAGE_ESPERANTO           },
  { "Polish",                RETRO_LANGUAGE_POLISH              },
  { "Vietnamese",            RETRO_LANGUAGE_VIETNAMESE          },
  { "Arabic",                RETRO_LANGUAGE_ARABIC              },
  { "Greek",                 RETRO_LANGUAGE_GREEK               },
  { "Turkish",               RETRO_LANGUAGE_TURKISH             },
  { "Slovak",                RETRO_LANGUAGE_SLOVAK              },
  { "Persian",               RETRO_LANGUAGE_PERSIAN             },
  { "Hebrew",                RETRO_LANGUAGE_HEBREW              },
  { "Asturian",              RETRO_LANGUAGE_ASTURIAN            },
  { "Finnish",               RETRO_LANGUAGE_FINNISH             },
  { "Indonesian",            RETRO_LANGUAGE_INDONESIAN          },
  { "Swedish",               RETRO_LANGUAGE_SWEDISH             },
  { "Ukrainian",             RETRO_LANGUAGE_UKRAINIAN           },
  { "Czech",                 RETRO_LANGUAGE_CZECH               },
  { "Catalan (Valencia)",    RETRO_LANGUAGE_CATALAN_VALENCIA    },
  { "Catalan",               RETRO_LANGUAGE_CATALAN             },
  { "English (British)",     RETRO_LANGUAGE_BRITISH_ENGLISH     },
  { "Hungarian",             RETRO_LANGUAGE_HUNGARIAN           },
  { "Belarusian",            RETRO_LANGUAGE_BELARUSIAN          },
  { "Galician",              RETRO_LANGUAGE_GALICIAN            },
  { "Norwegian",             RETRO_LANGUAGE_NORWEGIAN           },
  { "Irish",                 RETRO_LANGUAGE_IRISH               },
};

static const int k_languageCount = int(sizeof(k_languages) / sizeof(k_languages[0]));

QRetroConfig::QRetroConfig(QRetro *owner)
{
  m_Owner    = owner;
  m_Filename = QDir::currentPath() + "/config.ini";

  m_SaveTimer = new QTimer(this);
  m_SaveTimer->setSingleShot(true);
  m_SaveTimer->setInterval(400);
  connect(m_SaveTimer, &QTimer::timeout, [this]() { save(); });

  load();

  /* Apply persisted values to the owning QRetro's modules. */
  m_Owner->m_AudioEnabled   = m_AudioEnabled;
  m_Owner->m_BilinearFilter = m_BilinearFilter;
  m_Owner->m_IntegerScaling = m_IntegerScaling;

  m_Owner->sensors()->setFakeAccelEnabled(m_FakeAccelEnabled);
  m_Owner->sensors()->setFakeAccel(m_FakeAccel[0], m_FakeAccel[1], m_FakeAccel[2]);
  m_Owner->sensors()->setFakeGyroEnabled(m_FakeGyroEnabled);
  m_Owner->sensors()->setFakeGyro(m_FakeGyro[0], m_FakeGyro[1], m_FakeGyro[2]);
  m_Owner->sensors()->setFakeIllumEnabled(m_FakeIllumEnabled);
  m_Owner->sensors()->setFakeIllum(m_FakeIllum);

  /* Poll sensor read-tracking flags and enable/disable per-axis UI widgets. */
  auto *sensorReadTimer = new QTimer(this);
  sensorReadTimer->setInterval(200);
  connect(sensorReadTimer, &QTimer::timeout, [this]() {
    auto *s = m_Owner->sensors();
    setAccelXHasBeenRead(s->accelXHasBeenRead());
    setAccelYHasBeenRead(s->accelYHasBeenRead());
    setAccelZHasBeenRead(s->accelZHasBeenRead());
    setGyroXHasBeenRead (s->gyroXHasBeenRead());
    setGyroYHasBeenRead (s->gyroYHasBeenRead());
    setGyroZHasBeenRead (s->gyroZHasBeenRead());
    setIllumHasBeenRead (s->illumHasBeenRead());
  });
  sensorReadTimer->start();

  update();
  setWindowTitle(tr("QRetro Settings"));
}

void QRetroConfig::load()
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  settings.beginGroup("QRetro");
  m_Language       = static_cast<retro_language>(
    settings.value("language",       RETRO_LANGUAGE_ENGLISH).toInt());
  m_IntegerScaling = settings.value("integerScaling", false).toBool();
  m_BilinearFilter = settings.value("bilinearFilter", true).toBool();
  m_AudioEnabled   = settings.value("audioEnabled",   true).toBool();

  m_FakeAccelEnabled = settings.value("fakeAccelEnabled", false).toBool();
  m_FakeGyroEnabled  = settings.value("fakeGyroEnabled",  false).toBool();
  m_FakeIllumEnabled = settings.value("fakeIllumEnabled", false).toBool();
}

void QRetroConfig::save()
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  settings.beginGroup("QRetro");
  settings.setValue("language",       static_cast<int>(m_Language));
  settings.setValue("integerScaling", m_IntegerScaling);
  settings.setValue("bilinearFilter", m_BilinearFilter);
  settings.setValue("audioEnabled",   m_AudioEnabled);

  settings.setValue("fakeAccelEnabled", m_FakeAccelEnabled);
  settings.setValue("fakeGyroEnabled",  m_FakeGyroEnabled);
  settings.setValue("fakeIllumEnabled", m_FakeIllumEnabled);

  settings.sync();
}

void QRetroConfig::update()
{
  if (layout())
  {
    QLayoutItem *item;
    while ((item = layout()->takeAt(0)))
    {
      delete item->widget();
      delete item;
    }
    delete layout();
  }

  /* Wraps a form widget in a borderless scroll area. */
  auto makeScrollPage = [](QWidget *inner) -> QScrollArea* {
    auto *scroll = new QScrollArea();
    scroll->setWidget(inner);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    return scroll;
  };

  /* ── Video ─────────────────────────────────────────────────── */
  auto *videoPage = new QWidget();
  {
    auto *form = new QFormLayout(videoPage);
    form->setContentsMargins(12, 12, 12, 12);
    form->setVerticalSpacing(10);

    auto *intScaling = new QCheckBox();
    intScaling->setChecked(m_IntegerScaling);
    connect(intScaling, &QCheckBox::stateChanged, [this](int state) {
      m_IntegerScaling = state != Qt::Unchecked;
      m_SaveTimer->start();
      m_Owner->m_IntegerScaling = m_IntegerScaling;
      m_Owner->updateScaling();
      emit integerScalingChanged(m_IntegerScaling);
    });
    form->addRow(tr("Integer Scaling"), intScaling);

    auto *bilinear = new QCheckBox();
    bilinear->setChecked(m_BilinearFilter);
    connect(bilinear, &QCheckBox::stateChanged, [this](int state) {
      m_BilinearFilter = state != Qt::Unchecked;
      m_SaveTimer->start();
      m_Owner->m_BilinearFilter = m_BilinearFilter;
      emit bilinearFilterChanged(m_BilinearFilter);
    });
    form->addRow(tr("Bilinear Filter"), bilinear);
  }

  /* ── Audio ─────────────────────────────────────────────────── */
  auto *audioPage = new QWidget();
  {
    auto *form = new QFormLayout(audioPage);
    form->setContentsMargins(12, 12, 12, 12);
    form->setVerticalSpacing(10);

    auto *enabled = new QCheckBox();
    enabled->setChecked(m_AudioEnabled);
    connect(enabled, &QCheckBox::stateChanged, [this](int state) {
      m_AudioEnabled = state != Qt::Unchecked;
      m_SaveTimer->start();
      m_Owner->m_AudioEnabled = m_AudioEnabled;
      if (m_Owner->m_Audio)
        m_Owner->m_Audio->setEnabled(m_AudioEnabled);
      emit audioEnabledChanged(m_AudioEnabled);
    });
    form->addRow(tr("Enabled"), enabled);
  }

  /* ── Environment ────────────────────────────────────────────── */
  auto *envPage = new QWidget();
  {
    auto *form  = new QFormLayout(envPage);
    form->setContentsMargins(12, 12, 12, 12);
    form->setVerticalSpacing(10);
    auto *combo = new QComboBox();

    for (int i = 0; i < k_languageCount; i++)
      combo->addItem(k_languages[i].name, static_cast<int>(k_languages[i].id));
    combo->setCurrentIndex(combo->findData(static_cast<int>(m_Language)));

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, combo](int) {
      m_Language = static_cast<retro_language>(combo->currentData().toInt());
      m_SaveTimer->start();
    });
    form->addRow(tr("Language"), combo);
  }

  /* ── Sensors ────────────────────────────────────────────────── */
  auto *sensorsPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(sensorsPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    /*
     * Creates a [slider | spinbox] row widget with bidirectional sync.
     *
     * The slider operates in integer steps (scaled by `scale`), the spinbox
     * shows the real float value.  Editing either updates the other:
     *   slider changed → spinBox.setValue (fires valueChanged → emit lambda)
     *   spinbox changed → slider.setValue (blocked to break the loop) + emit
     *
     * Returns the QDoubleSpinBox so callers can connect their emit lambda to
     * its valueChanged signal.
     */
    auto makeAxisWidget = [](float initVal, double lo, double hi,
                             int scale, int decimals = 2) -> QPair<QWidget*, QDoubleSpinBox*>
    {
      auto *spin = new QDoubleSpinBox();
      spin->setRange(lo, hi);
      spin->setSingleStep(1.0 / scale);
      spin->setDecimals(decimals);
      spin->setValue(initVal);
      spin->setFixedWidth(80);

      auto *slider = new QSlider(Qt::Horizontal);
      slider->setRange(qRound(lo * scale), qRound(hi * scale));
      slider->setValue(qRound(initVal * scale));

      /* Slider → spinbox (spinbox.valueChanged then fires emit lambda) */
      QObject::connect(slider, &QSlider::valueChanged,
                       [spin, scale](int v) {
        spin->setValue(double(v) / scale);
      });

      /* Spinbox → slider (blocked to prevent re-entry) */
      QObject::connect(spin,
                       QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                       [slider, scale](double v) {
        slider->blockSignals(true);
        slider->setValue(qRound(v * scale));
        slider->blockSignals(false);
      });

      auto *w    = new QWidget();
      auto *hbox = new QHBoxLayout(w);
      hbox->setContentsMargins(0, 0, 0, 0);
      hbox->addWidget(slider, 1);
      hbox->addWidget(spin);
      return {w, spin};
    };

    /* ── Accelerometer ──────────────────────────────────────── */
    {
      auto *group = new QGroupBox(tr("Accelerometer (m/s²)"));
      auto *form  = new QFormLayout(group);
      form->setVerticalSpacing(8);

      auto *fakeCheck = new QCheckBox(tr("Fake values"));
      fakeCheck->setChecked(m_FakeAccelEnabled);

      auto [wX, spinX] = makeAxisWidget(m_FakeAccel[0], -20.0, 20.0, 100);
      auto [wY, spinY] = makeAxisWidget(m_FakeAccel[1], -20.0, 20.0, 100);
      auto [wZ, spinZ] = makeAxisWidget(m_FakeAccel[2], -20.0, 20.0, 100);
      m_AccelAxisWidget[0] = wX; wX->setEnabled(false);
      m_AccelAxisWidget[1] = wY; wY->setEnabled(false);
      m_AccelAxisWidget[2] = wZ; wZ->setEnabled(false);

      auto emitAccel = [this, fakeCheck, spinX, spinY, spinZ]() {
        m_FakeAccelEnabled = fakeCheck->isChecked();
        m_FakeAccel[0]     = static_cast<float>(spinX->value());
        m_FakeAccel[1]     = static_cast<float>(spinY->value());
        m_FakeAccel[2]     = static_cast<float>(spinZ->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setFakeAccelEnabled(m_FakeAccelEnabled);
        m_Owner->sensors()->setFakeAccel(m_FakeAccel[0], m_FakeAccel[1], m_FakeAccel[2]);
        emit fakeAccelChanged(m_FakeAccelEnabled, m_FakeAccel[0],
                              m_FakeAccel[1], m_FakeAccel[2]);
      };

      connect(fakeCheck, &QCheckBox::stateChanged,
              [emitAccel](int) { emitAccel(); });
      connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitAccel](double) { emitAccel(); });
      connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitAccel](double) { emitAccel(); });
      connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitAccel](double) { emitAccel(); });

      form->addRow(tr("Fake"), fakeCheck);
      form->addRow(tr("X"),    wX);
      form->addRow(tr("Y"),    wY);
      form->addRow(tr("Z"),    wZ);

      pageLayout->addWidget(group);
    }

    /* ── Gyroscope ──────────────────────────────────────────── */
    {
      auto *group = new QGroupBox(tr("Gyroscope (rad/s)"));
      auto *form  = new QFormLayout(group);
      form->setVerticalSpacing(8);

      auto *fakeCheck = new QCheckBox(tr("Fake values"));
      fakeCheck->setChecked(m_FakeGyroEnabled);

      auto [wX, spinX] = makeAxisWidget(m_FakeGyro[0], -1.0, 1.0, 100);
      auto [wY, spinY] = makeAxisWidget(m_FakeGyro[1], -1.0, 1.0, 100);
      auto [wZ, spinZ] = makeAxisWidget(m_FakeGyro[2], -1.0, 1.0, 100);
      m_GyroAxisWidget[0] = wX; wX->setEnabled(false);
      m_GyroAxisWidget[1] = wY; wY->setEnabled(false);
      m_GyroAxisWidget[2] = wZ; wZ->setEnabled(false);

      auto emitGyro = [this, fakeCheck, spinX, spinY, spinZ]() {
        m_FakeGyroEnabled = fakeCheck->isChecked();
        m_FakeGyro[0]     = static_cast<float>(spinX->value());
        m_FakeGyro[1]     = static_cast<float>(spinY->value());
        m_FakeGyro[2]     = static_cast<float>(spinZ->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setFakeGyroEnabled(m_FakeGyroEnabled);
        m_Owner->sensors()->setFakeGyro(m_FakeGyro[0], m_FakeGyro[1], m_FakeGyro[2]);
        emit fakeGyroChanged(m_FakeGyroEnabled, m_FakeGyro[0],
                             m_FakeGyro[1], m_FakeGyro[2]);
      };

      connect(fakeCheck, &QCheckBox::stateChanged,
              [emitGyro](int) { emitGyro(); });
      connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitGyro](double) { emitGyro(); });
      connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitGyro](double) { emitGyro(); });
      connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitGyro](double) { emitGyro(); });

      form->addRow(tr("Fake"), fakeCheck);
      form->addRow(tr("X"),    wX);
      form->addRow(tr("Y"),    wY);
      form->addRow(tr("Z"),    wZ);

      pageLayout->addWidget(group);
    }

    /* ── Illuminance ────────────────────────────────────────── */
    {
      auto *group = new QGroupBox(tr("Illuminance (lux)"));
      auto *form  = new QFormLayout(group);
      form->setVerticalSpacing(8);

      auto *fakeCheck = new QCheckBox(tr("Fake values"));
      fakeCheck->setChecked(m_FakeIllumEnabled);

      /* Spinbox: unconstrained so the user can type any value */
      auto *spin = new QDoubleSpinBox();
      spin->setRange(-999999.0, 999999.0);
      spin->setSingleStep(1.0);
      spin->setDecimals(0);
      spin->setValue(m_FakeIllum);
      spin->setFixedWidth(90);

      /* Slider: capped at 100 klux; grays out when spinbox is out of range */
      auto *slider = new QSlider(Qt::Horizontal);
      slider->setRange(0, 100000);
      slider->setValue(qBound(0, qRound(double(m_FakeIllum)), 100000));
      slider->setEnabled(m_FakeIllum >= 0 && m_FakeIllum <= 100000);

      /* Slider → spinbox */
      connect(slider, &QSlider::valueChanged, [spin](int v) {
        spin->setValue(double(v));
      });

      /* Spinbox → slider (disable slider when out of range) */
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [slider](double v) {
        bool inRange = (v >= slider->minimum() && v <= slider->maximum());
        slider->setEnabled(inRange);
        if (inRange) {
          slider->blockSignals(true);
          slider->setValue(qRound(v));
          slider->blockSignals(false);
        }
      });

      auto emitIllum = [this, fakeCheck, spin]() {
        m_FakeIllumEnabled = fakeCheck->isChecked();
        m_FakeIllum        = static_cast<float>(spin->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setFakeIllumEnabled(m_FakeIllumEnabled);
        m_Owner->sensors()->setFakeIllum(m_FakeIllum);
        emit fakeIllumChanged(m_FakeIllumEnabled, m_FakeIllum);
      };

      connect(fakeCheck, &QCheckBox::stateChanged,
              [emitIllum](int) { emitIllum(); });
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitIllum](double) { emitIllum(); });

      auto *valueWidget = new QWidget();
      auto *hbox = new QHBoxLayout(valueWidget);
      hbox->setContentsMargins(0, 0, 0, 0);
      hbox->addWidget(slider, 1);
      hbox->addWidget(spin);
      m_IllumValueWidget = valueWidget;
      valueWidget->setEnabled(false);

      form->addRow(tr("Fake"),  fakeCheck);
      form->addRow(tr("Value"), valueWidget);

      pageLayout->addWidget(group);
    }

    pageLayout->addStretch();
  }

  /* ── Sidebar ────────────────────────────────────────────────── */
  auto *sidebar = new QListWidget();
  sidebar->setFrameShape(QFrame::NoFrame);
  sidebar->setStyleSheet("QListWidget::item { padding: 6px 10px; }");
  sidebar->addItem(tr("Video"));
  sidebar->addItem(tr("Audio"));
  sidebar->addItem(tr("Environment"));
  sidebar->addItem(tr("Sensors"));
  sidebar->setCurrentRow(0);

  /* ── Stacked settings area ──────────────────────────────────── */
  auto *stack = new QStackedWidget();
  stack->addWidget(makeScrollPage(videoPage));
  stack->addWidget(makeScrollPage(audioPage));
  stack->addWidget(makeScrollPage(envPage));
  stack->addWidget(makeScrollPage(sensorsPage));

  connect(sidebar, &QListWidget::currentRowChanged,
          stack,   &QStackedWidget::setCurrentIndex);

  /* ── Splitter (25 % / 75 %) ─────────────────────────────────── */
  auto *splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(sidebar);
  splitter->addWidget(stack);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  splitter->setSizes({160, 480});

  /* ── Description strip ──────────────────────────────────────── */
  auto *sep = new QFrame();
  sep->setFrameShape(QFrame::HLine);
  sep->setFrameShadow(QFrame::Sunken);

  m_DescLabel = new QLabel();
  m_DescLabel->setWordWrap(true);
  m_DescLabel->setMinimumHeight(36);
  m_DescLabel->setContentsMargins(6, 4, 6, 4);
  m_DescLabel->setForegroundRole(QPalette::Dark);

  /* ── Outer layout ───────────────────────────────────────────── */
  auto *outer = new QVBoxLayout();
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(0);
  outer->addWidget(splitter, 1);
  outer->addWidget(sep);
  outer->addWidget(m_DescLabel);

  setLayout(outer);
  resize(640, 400);
}
