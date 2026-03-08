#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QDir>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
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

static const struct { const char *name; unsigned id; } k_memTypes[4] = {
  { "RETRO_MEMORY_SAVE_RAM",   RETRO_MEMORY_SAVE_RAM   },
  { "RETRO_MEMORY_RTC",        RETRO_MEMORY_RTC        },
  { "RETRO_MEMORY_SYSTEM_RAM", RETRO_MEMORY_SYSTEM_RAM },
  { "RETRO_MEMORY_VIDEO_RAM",  RETRO_MEMORY_VIDEO_RAM  },
};

static QString fmtMemPtr(void *p)
{
  if (!p) return QStringLiteral("—");
  return QString("0x") + QStringLiteral("%1")
    .arg(reinterpret_cast<quintptr>(p), int(sizeof(void*)) * 2, 16, QChar('0'))
    .toUpper();
}

static QString fmtMemSize(size_t s)
{
  if (!s) return QStringLiteral("—");
  if (s >= 1024 * 1024) return QStringLiteral("%1 bytes (%2 MB)").arg(s).arg(s / (1024 * 1024));
  if (s >= 1024)        return QStringLiteral("%1 bytes (%2 KB)").arg(s).arg(s / 1024);
  return QStringLiteral("%1 bytes").arg(s);
}

static QString fmtHex(uint64_t v)
{
  if (!v) return QStringLiteral("0x0");
  return QStringLiteral("0x") + QString::number(v, 16).toUpper();
}

static void makeSmallGray(QLabel *label)
{
  QFont f = label->font();
  f.setPointSize(qMax(f.pointSize() - 1, 7));
  label->setFont(f);
  label->setForegroundRole(QPalette::Dark);
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
}

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
  if (m_Owner->m_Audio)
    m_Owner->m_Audio->setVolume(m_AudioVolume);
  m_Owner->m_BilinearFilter = m_BilinearFilter;
  m_Owner->m_IntegerScaling = m_IntegerScaling;


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

    auto yesNo = [](bool v) { return v ? QStringLiteral("Yes") : QStringLiteral("No"); };
    auto hz    = [](unsigned r) {
      return r ? QString::number(r) + QStringLiteral(" Hz") : QStringLiteral("—");
    };

    if (m_AccelEnabledLabel) m_AccelEnabledLabel->setText(yesNo(s->accelEnabled()));
    if (m_AccelRateLabel)    m_AccelRateLabel->setText(hz(s->accelRate()));
    if (m_GyroEnabledLabel)  m_GyroEnabledLabel->setText(yesNo(s->gyroEnabled()));
    if (m_GyroRateLabel)     m_GyroRateLabel->setText(hz(s->gyroRate()));
    if (m_IllumEnabledLabel) m_IllumEnabledLabel->setText(yesNo(s->illumEnabled()));
    if (m_IllumRateLabel)    m_IllumRateLabel->setText(hz(s->illumRate()));

    if (auto *loc = m_Owner->location())
    {
      if (m_LocationStateLabel)
      {
        const char *stateStr = "—";
        switch (loc->state())
        {
        case QRetroLocation::Uninitialized: stateStr = "Uninitialized"; break;
        case QRetroLocation::Started:       stateStr = "Started";       break;
        case QRetroLocation::Stopped:       stateStr = "Stopped";       break;
        }
        m_LocationStateLabel->setText(tr(stateStr));
      }
      auto ms   = [](unsigned v) {
        return v ? QString::number(v) + QStringLiteral(" ms") : QStringLiteral("—");
      };
      auto dist = [](unsigned v) {
        return v ? QString::number(v) + QStringLiteral(" m") : QStringLiteral("—");
      };
      if (m_LocationIntervalLabel) m_LocationIntervalLabel->setText(ms(loc->millisecondInterval()));
      if (m_LocationDistLabel)     m_LocationDistLabel->setText(dist(loc->distanceInterval()));
    }

    if (m_LedForm)
    {
      for (auto &[idx, val] : m_Owner->led()->leds())
      {
        if (m_LedLabels.contains(idx))
        {
          m_LedLabels[idx]->setText(QString::number(val));
        }
        else
        {
          auto *label = new QLabel(QString::number(val));
          m_LedLabels[idx] = label;
          m_LedForm->addRow(tr("LED %1").arg(idx), label);
          if (m_LedEmptyLabel)
            m_LedEmptyLabel->setVisible(false);
        }
      }
    }

    auto yesNoStr = [this](bool set, bool v) -> QString {
      return set ? (v ? tr("Yes") : tr("No")) : tr("Unset");
    };

    if (m_CoreAchievementsLabel)
      m_CoreAchievementsLabel->setText(
        yesNoStr(m_Owner->supportsAchievementsSet(), m_Owner->supportsAchievements()));

    if (m_CorePerfLevelLabel)
      m_CorePerfLevelLabel->setText(
        m_Owner->performanceLevelSet()
          ? QString::number(m_Owner->performanceLevel())
          : tr("Unset"));

    if (m_CorePixelFormatLabel) {
      QString s = tr("Unset");
      if (m_Owner->pixelFormatSet()) {
        switch (m_Owner->retroPixelFormat()) {
          case RETRO_PIXEL_FORMAT_0RGB1555: s = QStringLiteral("0RGB1555"); break;
          case RETRO_PIXEL_FORMAT_XRGB8888: s = QStringLiteral("XRGB8888"); break;
          case RETRO_PIXEL_FORMAT_RGB565:   s = QStringLiteral("RGB565");   break;
          default:                          s = QStringLiteral("Unknown");   break;
        }
      }
      m_CorePixelFormatLabel->setText(s);
    }

    if (m_CoreSerializationLabel)
      m_CoreSerializationLabel->setText(
        m_Owner->serializationQuirksSet()
          ? QStringLiteral("0x") + QString::number(m_Owner->serializationQuirks(), 16).toUpper()
          : tr("Unset"));

    if (m_CoreSupportsNoGameLabel)
      m_CoreSupportsNoGameLabel->setText(
        yesNoStr(m_Owner->supportsNoGameSet(), m_Owner->supportsNoGame()));

    if (m_Owner->m_Core.retro_get_memory_data)
    {
      for (int i = 0; i < 4; i++)
      {
        if (!m_MemDataPtrLabel[i]) continue;
        void  *ptr = m_Owner->m_Core.retro_get_memory_data(k_memTypes[i].id);
        size_t sz  = m_Owner->m_Core.retro_get_memory_size
                     ? m_Owner->m_Core.retro_get_memory_size(k_memTypes[i].id) : 0;
        m_MemDataPtrLabel[i]->setText(fmtMemPtr(ptr));
        m_MemDataSizeLabel[i]->setText(fmtMemSize(sz));
      }
    }

    if (m_MemMapsForm)
    {
      auto *maps = m_Owner->memoryMaps();
      int count  = maps ? (int)maps->num_descriptors : 0;
      if (count != m_MemMapsShownCount)
      {
        while (m_MemMapsForm->rowCount() > 0)
          m_MemMapsForm->removeRow(0);
        m_MemMapsShownCount = count;

        if (count == 0)
        {
          auto *lbl = new QLabel(tr("No memory descriptors reported by this core."));
          lbl->setWordWrap(true);
          makeSmallGray(lbl);
          m_MemMapsForm->addRow(lbl);
        }
        else
        {
          for (int i = 0; i < count; i++)
          {
            const auto &d = maps->descriptors[i];

            QString header = d.addrspace && *d.addrspace
              ? QStringLiteral("[%1] %2").arg(i).arg(d.addrspace)
              : QStringLiteral("[%1]").arg(i);

            QString body =
              QStringLiteral("flags=%1  ptr=%2  offset=%3\n"
                             "start=%4  select=%5  disconnect=%6\n"
                             "len=%7")
              .arg(fmtHex(d.flags))
              .arg(fmtMemPtr(d.ptr))
              .arg(fmtHex(d.offset))
              .arg(fmtHex(d.start))
              .arg(fmtHex(d.select))
              .arg(fmtHex(d.disconnect))
              .arg(fmtMemSize(d.len));

            auto *block = new QLabel(header + QStringLiteral("\n") + body);
            QFont f("Courier");
            f.setStyleHint(QFont::TypeWriter);
            f.setPointSize(qMax(QApplication::font().pointSize() - 1, 7));
            block->setFont(f);
            block->setWordWrap(true);
            block->setTextInteractionFlags(Qt::TextSelectableByMouse);
            block->setStyleSheet(
              "QLabel { background: palette(alternate-base); "
              "border: 1px solid palette(mid); "
              "padding: 4px 6px; border-radius: 3px; }");
            if (i > 0)
              block->setContentsMargins(0, 4, 0, 0);
            m_MemMapsForm->addRow(block);
          }
        }
      }
    }
  });
  sensorReadTimer->start();

  update();
  setWindowTitle(tr("QRetro Settings"));
}

void QRetroConfig::showEvent(QShowEvent *event)
{
  update();
  QWidget::showEvent(event);
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
  m_AudioVolume    = settings.value("audioVolume",    1.0f).toFloat();

  m_SpoofLocationEnabled = settings.value("spoofLocationEnabled", false).toBool();
  m_SpoofLat             = settings.value("spoofLat",  0.0).toDouble();
  m_SpoofLon             = settings.value("spoofLon",  0.0).toDouble();
  m_SpoofHAcc            = settings.value("spoofHAcc", 0.0).toDouble();
  m_SpoofVAcc            = settings.value("spoofVAcc", 0.0).toDouble();

  auto *dirs = m_Owner->directories();
  if (settings.contains("dirSave"))
    dirs->set(QRetroDirectories::Save,       settings.value("dirSave").toString(),       true);
  if (settings.contains("dirSystem"))
    dirs->set(QRetroDirectories::System,     settings.value("dirSystem").toString(),     true);
  if (settings.contains("dirCoreAssets"))
    dirs->set(QRetroDirectories::CoreAssets, settings.value("dirCoreAssets").toString(), true);
  if (settings.contains("dirPlaylist"))
    dirs->set(QRetroDirectories::Playlist,        settings.value("dirPlaylist").toString(),        true);
  if (settings.contains("dirFileBrowserStart"))
    dirs->set(QRetroDirectories::FileBrowserStart, settings.value("dirFileBrowserStart").toString(), true);
}

void QRetroConfig::save()
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  settings.beginGroup("QRetro");
  settings.setValue("language",       static_cast<int>(m_Language));
  settings.setValue("integerScaling", m_IntegerScaling);
  settings.setValue("bilinearFilter", m_BilinearFilter);
  settings.setValue("audioEnabled",   m_AudioEnabled);
  settings.setValue("audioVolume",    m_AudioVolume);

  settings.setValue("spoofLocationEnabled", m_SpoofLocationEnabled);
  settings.setValue("spoofLat",  m_SpoofLat);
  settings.setValue("spoofLon",  m_SpoofLon);
  settings.setValue("spoofHAcc", m_SpoofHAcc);
  settings.setValue("spoofVAcc", m_SpoofVAcc);

  auto *dirs = m_Owner->directories();
  settings.setValue("dirSave",       dirs->get(QRetroDirectories::Save));
  settings.setValue("dirSystem",     dirs->get(QRetroDirectories::System));
  settings.setValue("dirCoreAssets",      dirs->get(QRetroDirectories::CoreAssets));
  settings.setValue("dirPlaylist",        dirs->get(QRetroDirectories::Playlist));
  settings.setValue("dirFileBrowserStart",dirs->get(QRetroDirectories::FileBrowserStart));

  settings.sync();
}

void QRetroConfig::update()
{
  /* QRetroOptions is a by-value member of QRetro and must never be deleted
   * by Qt's layout teardown. Detach it from its parent before destroying
   * the old layout so the stack won't try to delete it. */
  m_Owner->options()->setParent(nullptr);

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

  m_LedForm       = nullptr;
  m_LedEmptyLabel = nullptr;
  m_LedLabels.clear();
  m_CoreAchievementsLabel   = nullptr;
  m_CorePerfLevelLabel      = nullptr;
  m_CorePixelFormatLabel    = nullptr;
  m_CoreSerializationLabel  = nullptr;
  m_CoreSupportsNoGameLabel = nullptr;
  for (int i = 0; i < 4; i++)
    m_MemDataPtrLabel[i] = m_MemDataSizeLabel[i] = nullptr;
  m_MemMapsForm       = nullptr;
  m_MemMapsShownCount = -1;

  /* Wraps a form widget in a borderless scroll area. */
  auto makeScrollPage = [](QWidget *inner) -> QScrollArea* {
    auto *scroll = new QScrollArea();
    scroll->setWidget(inner);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setFocusPolicy(Qt::NoFocus);
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

    auto *volSlider = new QSlider(Qt::Horizontal);
    volSlider->setRange(0, 100);
    volSlider->setValue(qRound(m_AudioVolume * 100.0f));
    volSlider->setTickPosition(QSlider::TicksBelow);
    volSlider->setTickInterval(10);

    auto *volSpin = new QSpinBox();
    volSpin->setRange(0, 100);
    volSpin->setSuffix("%");
    volSpin->setValue(qRound(m_AudioVolume * 100.0f));
    volSpin->setFixedWidth(70);

    /* Slider → spinbox */
    connect(volSlider, &QSlider::valueChanged, [volSpin](int v) {
      volSpin->blockSignals(true);
      volSpin->setValue(v);
      volSpin->blockSignals(false);
    });

    /* Spinbox → slider + apply */
    connect(volSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this, volSlider](int v) {
      volSlider->blockSignals(true);
      volSlider->setValue(v);
      volSlider->blockSignals(false);
      m_AudioVolume = v / 100.0f;
      m_SaveTimer->start();
      if (m_Owner->m_Audio)
        m_Owner->m_Audio->setVolume(m_AudioVolume);
    });

    auto *volRow = new QWidget();
    auto *volBox = new QHBoxLayout(volRow);
    volBox->setContentsMargins(0, 0, 0, 0);
    volBox->addWidget(volSlider, 1);
    volBox->addWidget(volSpin);
    form->addRow(tr("Volume"), volRow);
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

  /* ── Directories ────────────────────────────────────────────── */
  auto *dirsPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(dirsPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    auto *group = new QGroupBox(tr("Directories"));
    auto *form  = new QFormLayout(group);
    form->setVerticalSpacing(8);

    struct { const char *label; QRetroDirectories::Type type; } rows[] = {
      { "Save",              QRetroDirectories::Save            },
      { "System",            QRetroDirectories::System          },
      { "Core Assets",       QRetroDirectories::CoreAssets      },
      { "Playlist",          QRetroDirectories::Playlist        },
      { "File Browser Start",QRetroDirectories::FileBrowserStart},
    };

    for (auto &row : rows)
    {
      auto *edit = new QLineEdit(m_Owner->directories()->get(row.type));

      QRetroDirectories::Type type = row.type;
      auto *browseBtn = new QPushButton(tr("Browse…"));
      connect(browseBtn, &QPushButton::clicked, [this, edit, type]() {
        QString path = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                         edit->text());
        if (!path.isEmpty())
        {
          edit->setText(path);
          m_Owner->directories()->set(type, path);
          m_SaveTimer->start();
        }
      });

      connect(edit, &QLineEdit::textEdited, [this, type](const QString &path) {
        m_Owner->directories()->set(type, path, true);
        m_SaveTimer->start();
      });

      auto *rowWidget = new QWidget();
      auto *hbox = new QHBoxLayout(rowWidget);
      hbox->setContentsMargins(0, 0, 0, 0);
      hbox->addWidget(edit, 1);
      hbox->addWidget(browseBtn);
      form->addRow(tr(row.label), rowWidget);
    }

    pageLayout->addWidget(group);
    pageLayout->addStretch();
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

      m_AccelEnabledLabel = new QLabel(tr("—"));
      m_AccelRateLabel    = new QLabel(tr("—"));
      form->addRow(tr("Enabled"), m_AccelEnabledLabel);
      form->addRow(tr("Rate"),    m_AccelRateLabel);

      auto *spoofCheck = new QCheckBox(tr("Spoof values"));
      spoofCheck->setChecked(m_SpoofAccelEnabled);

      auto [wX, spinX] = makeAxisWidget(m_SpoofAccel[0], -20.0, 20.0, 100);
      auto [wY, spinY] = makeAxisWidget(m_SpoofAccel[1], -20.0, 20.0, 100);
      auto [wZ, spinZ] = makeAxisWidget(m_SpoofAccel[2], -20.0, 20.0, 100);
      m_AccelAxisWidget[0] = wX; wX->setEnabled(false);
      m_AccelAxisWidget[1] = wY; wY->setEnabled(false);
      m_AccelAxisWidget[2] = wZ; wZ->setEnabled(false);

      auto emitAccel = [this, spoofCheck, spinX, spinY, spinZ]() {
        m_SpoofAccelEnabled = spoofCheck->isChecked();
        m_SpoofAccel[0]     = static_cast<float>(spinX->value());
        m_SpoofAccel[1]     = static_cast<float>(spinY->value());
        m_SpoofAccel[2]     = static_cast<float>(spinZ->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setSpoofAccelEnabled(m_SpoofAccelEnabled);
        m_Owner->sensors()->setSpoofAccel(m_SpoofAccel[0], m_SpoofAccel[1], m_SpoofAccel[2]);
        emit spoofAccelChanged(m_SpoofAccelEnabled, m_SpoofAccel[0],
                              m_SpoofAccel[1], m_SpoofAccel[2]);
      };

      connect(spoofCheck, &QCheckBox::stateChanged,
              [emitAccel](int) { emitAccel(); });
      connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitAccel](double) { emitAccel(); });
      connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitAccel](double) { emitAccel(); });
      connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitAccel](double) { emitAccel(); });

      form->addRow(tr("Spoof"), spoofCheck);
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

      m_GyroEnabledLabel = new QLabel(tr("—"));
      m_GyroRateLabel    = new QLabel(tr("—"));
      form->addRow(tr("Enabled"), m_GyroEnabledLabel);
      form->addRow(tr("Rate"),    m_GyroRateLabel);

      auto *spoofCheck = new QCheckBox(tr("Spoof values"));
      spoofCheck->setChecked(m_SpoofGyroEnabled);

      auto [wX, spinX] = makeAxisWidget(m_SpoofGyro[0], -1.0, 1.0, 100);
      auto [wY, spinY] = makeAxisWidget(m_SpoofGyro[1], -1.0, 1.0, 100);
      auto [wZ, spinZ] = makeAxisWidget(m_SpoofGyro[2], -1.0, 1.0, 100);
      m_GyroAxisWidget[0] = wX; wX->setEnabled(false);
      m_GyroAxisWidget[1] = wY; wY->setEnabled(false);
      m_GyroAxisWidget[2] = wZ; wZ->setEnabled(false);

      auto emitGyro = [this, spoofCheck, spinX, spinY, spinZ]() {
        m_SpoofGyroEnabled = spoofCheck->isChecked();
        m_SpoofGyro[0]     = static_cast<float>(spinX->value());
        m_SpoofGyro[1]     = static_cast<float>(spinY->value());
        m_SpoofGyro[2]     = static_cast<float>(spinZ->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setSpoofGyroEnabled(m_SpoofGyroEnabled);
        m_Owner->sensors()->setSpoofGyro(m_SpoofGyro[0], m_SpoofGyro[1], m_SpoofGyro[2]);
        emit spoofGyroChanged(m_SpoofGyroEnabled, m_SpoofGyro[0],
                             m_SpoofGyro[1], m_SpoofGyro[2]);
      };

      connect(spoofCheck, &QCheckBox::stateChanged,
              [emitGyro](int) { emitGyro(); });
      connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitGyro](double) { emitGyro(); });
      connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitGyro](double) { emitGyro(); });
      connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
              [emitGyro](double) { emitGyro(); });

      form->addRow(tr("Spoof"), spoofCheck);
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

      m_IllumEnabledLabel = new QLabel(tr("—"));
      m_IllumRateLabel    = new QLabel(tr("—"));
      form->addRow(tr("Enabled"), m_IllumEnabledLabel);
      form->addRow(tr("Rate"),    m_IllumRateLabel);

      auto *spoofCheck = new QCheckBox(tr("Spoof values"));
      spoofCheck->setChecked(m_SpoofIllumEnabled);

      /* Spinbox: unconstrained so the user can type any value */
      auto *spin = new QDoubleSpinBox();
      spin->setRange(-999999.0, 999999.0);
      spin->setSingleStep(1.0);
      spin->setDecimals(0);
      spin->setValue(m_SpoofIllum);
      spin->setFixedWidth(90);

      /* Slider: capped at 100 klux; grays out when spinbox is out of range */
      auto *slider = new QSlider(Qt::Horizontal);
      slider->setRange(0, 100000);
      slider->setValue(qBound(0, qRound(double(m_SpoofIllum)), 100000));
      slider->setEnabled(m_SpoofIllum >= 0 && m_SpoofIllum <= 100000);

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

      auto emitIllum = [this, spoofCheck, spin]() {
        m_SpoofIllumEnabled = spoofCheck->isChecked();
        m_SpoofIllum        = static_cast<float>(spin->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setSpoofIllumEnabled(m_SpoofIllumEnabled);
        m_Owner->sensors()->setSpoofIllum(m_SpoofIllum);
        emit spoofIllumChanged(m_SpoofIllumEnabled, m_SpoofIllum);
      };

      connect(spoofCheck, &QCheckBox::stateChanged,
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

      form->addRow(tr("Spoof"),  spoofCheck);
      form->addRow(tr("Value"), valueWidget);

      pageLayout->addWidget(group);
    }

    pageLayout->addStretch();
  }

  /* ── Location ───────────────────────────────────────────────── */
  auto *locationPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(locationPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    auto *group = new QGroupBox(tr("Position"));
    auto *form  = new QFormLayout(group);
    form->setVerticalSpacing(8);

    m_LocationStateLabel    = new QLabel(tr("—"));
    m_LocationIntervalLabel = new QLabel(tr("—"));
    m_LocationDistLabel     = new QLabel(tr("—"));
    form->addRow(tr("State"),                 m_LocationStateLabel);
    form->addRow(tr("Time interval (ms)"),    m_LocationIntervalLabel);
    form->addRow(tr("Distance interval (m)"), m_LocationDistLabel);

    auto *spoofCheck = new QCheckBox(tr("Spoof values"));
    spoofCheck->setChecked(m_SpoofLocationEnabled);

    auto makeSpin = [](double lo, double hi, double step,
                       int decimals, double init) -> QDoubleSpinBox* {
      auto *s = new QDoubleSpinBox();
      s->setRange(lo, hi);
      s->setSingleStep(step);
      s->setDecimals(decimals);
      s->setValue(init);
      return s;
    };

    auto *latSpin  = makeSpin(-90.0,   90.0,    0.000001, 6, m_SpoofLat);
    auto *lonSpin  = makeSpin(-180.0,  180.0,   0.000001, 6, m_SpoofLon);
    auto *hAccSpin = makeSpin(0.0,     100000.0, 1.0,     1, m_SpoofHAcc);
    auto *vAccSpin = makeSpin(0.0,     100000.0, 1.0,     1, m_SpoofVAcc);

    m_LocationSpoofWidgets[0] = latSpin;
    m_LocationSpoofWidgets[1] = lonSpin;
    m_LocationSpoofWidgets[2] = hAccSpin;
    m_LocationSpoofWidgets[3] = vAccSpin;
    for (auto *w : m_LocationSpoofWidgets)
      w->setEnabled(m_SpoofLocationEnabled);

    auto emitLocation = [this, spoofCheck, latSpin, lonSpin, hAccSpin, vAccSpin]() {
      m_SpoofLocationEnabled = spoofCheck->isChecked();
      m_SpoofLat  = latSpin->value();
      m_SpoofLon  = lonSpin->value();
      m_SpoofHAcc = hAccSpin->value();
      m_SpoofVAcc = vAccSpin->value();
      m_SaveTimer->start();
      auto *loc = m_Owner->location();
      loc->setSpoofEnabled(m_SpoofLocationEnabled);
      loc->setSpoofLat(m_SpoofLat);
      loc->setSpoofLon(m_SpoofLon);
      loc->setSpoofHAcc(m_SpoofHAcc);
      loc->setSpoofVAcc(m_SpoofVAcc);
      for (auto *w : m_LocationSpoofWidgets)
        w->setEnabled(m_SpoofLocationEnabled);
      emit spoofLocationChanged(m_SpoofLocationEnabled, m_SpoofLat, m_SpoofLon,
                                m_SpoofHAcc, m_SpoofVAcc);
    };

    connect(spoofCheck, &QCheckBox::stateChanged,
            [emitLocation](int) { emitLocation(); });
    connect(latSpin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [emitLocation](double) { emitLocation(); });
    connect(lonSpin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [emitLocation](double) { emitLocation(); });
    connect(hAccSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [emitLocation](double) { emitLocation(); });
    connect(vAccSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [emitLocation](double) { emitLocation(); });

    form->addRow(tr("Spoof"),        spoofCheck);
    form->addRow(tr("Latitude"),     latSpin);
    form->addRow(tr("Longitude"),    lonSpin);
    form->addRow(tr("H. Accuracy"), hAccSpin);
    form->addRow(tr("V. Accuracy"), vAccSpin);

    pageLayout->addWidget(group);
    pageLayout->addStretch();
  }

  /* ── LED ────────────────────────────────────────────────────── */
  auto *ledPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(ledPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    auto *group = new QGroupBox(tr("LED States"));
    m_LedForm = new QFormLayout(group);
    m_LedForm->setVerticalSpacing(8);

    m_LedEmptyLabel = new QLabel(
      tr("LED states will appear here when the core modifies an LED."));
    m_LedEmptyLabel->setWordWrap(true);
    {
      QFont f = m_LedEmptyLabel->font();
      f.setItalic(true);
      m_LedEmptyLabel->setFont(f);
    }
    m_LedEmptyLabel->setForegroundRole(QPalette::Dark);
    m_LedForm->addRow(m_LedEmptyLabel);

    for (auto &[idx, val] : m_Owner->led()->leds())
    {
      auto *label = new QLabel(QString::number(val));
      m_LedLabels[idx] = label;
      m_LedForm->addRow(tr("LED %1").arg(idx), label);
    }

    m_LedEmptyLabel->setVisible(m_LedLabels.isEmpty());

    pageLayout->addWidget(group);
    pageLayout->addStretch();
  }

  /* ── Core Constants ─────────────────────────────────────────── */
  auto *coreConstPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(coreConstPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    auto *group = new QGroupBox(tr("Core Constants"));
    auto *form  = new QFormLayout(group);
    form->setVerticalSpacing(4);

    auto yesNo = [](bool set, bool v) -> QLabel* {
      return new QLabel(set ? (v ? tr("Yes") : tr("No")) : tr("Unset"));
    };

    auto makeDesc = [form](const char *text) {
      auto *label = new QLabel(text);
      label->setWordWrap(true);
      QFont f = label->font();
      f.setPointSize(qMax(f.pointSize() - 1, 7));
      label->setFont(f);
      label->setForegroundRole(QPalette::Dark);
      label->setContentsMargins(0, 0, 0, 6);
      form->addRow(label);
    };

    m_CorePerfLevelLabel = new QLabel(
      m_Owner->performanceLevelSet()
        ? QString::number(m_Owner->performanceLevel())
        : tr("Unset"));
    form->addRow(tr("Performance Level"), m_CorePerfLevelLabel);
    makeDesc("The relative performance level of the core.\n"
             "RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL (8)");

    auto fmtPixel = [this]() -> QString {
      if (!m_Owner->pixelFormatSet()) return tr("Unset");
      switch (m_Owner->retroPixelFormat()) {
        case RETRO_PIXEL_FORMAT_0RGB1555: return QStringLiteral("0RGB1555");
        case RETRO_PIXEL_FORMAT_XRGB8888: return QStringLiteral("XRGB8888");
        case RETRO_PIXEL_FORMAT_RGB565:   return QStringLiteral("RGB565");
        default:                          return QStringLiteral("Invalid (0x%1)").arg(m_Owner->retroPixelFormat(), 0, 16).toUpper();
      }
    };
    m_CorePixelFormatLabel = new QLabel(fmtPixel());
    form->addRow(tr("Pixel Format"), m_CorePixelFormatLabel);
    makeDesc("Pixel format used by the core's framebuffer.\n"
             "RETRO_ENVIRONMENT_SET_PIXEL_FORMAT (10)");

    m_CoreSupportsNoGameLabel = yesNo(m_Owner->supportsNoGameSet(),
                                      m_Owner->supportsNoGame());
    form->addRow(tr("Supports No Game"), m_CoreSupportsNoGameLabel);
    makeDesc("Whether this core reports to support running without any loaded content.\n"
             "RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME (18)");

    m_CoreAchievementsLabel = yesNo(m_Owner->supportsAchievementsSet(),
                                    m_Owner->supportsAchievements());
    form->addRow(tr("Supports Achievements"), m_CoreAchievementsLabel);
    makeDesc("Whether or not this core reports to support achievements.\n"
             "RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS (42)");

    m_CoreSerializationLabel = new QLabel(
      m_Owner->serializationQuirksSet()
        ? QStringLiteral("0x") + QString::number(m_Owner->serializationQuirks(), 16).toUpper()
        : tr("Unset"));
    form->addRow(tr("Serialization Quirks"), m_CoreSerializationLabel);
    makeDesc("Bitmask of quirks affecting save state serialization.\n"
             "RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS (44)");

    pageLayout->addWidget(group);
    pageLayout->addStretch();
  }

  /* ── Memory ─────────────────────────────────────────────────── */
  auto *memPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(memPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    auto *group = new QGroupBox(tr("Memory Data"));
    auto *form  = new QFormLayout(group);
    form->setVerticalSpacing(8);

    for (int i = 0; i < 4; i++)
    {
      auto *header = new QLabel(k_memTypes[i].name);
      if (i > 0)
        header->setContentsMargins(0, 8, 0, 0);
      form->addRow(header);

      void  *ptr = m_Owner->m_Core.retro_get_memory_data
                   ? m_Owner->m_Core.retro_get_memory_data(k_memTypes[i].id) : nullptr;
      size_t sz  = m_Owner->m_Core.retro_get_memory_size
                   ? m_Owner->m_Core.retro_get_memory_size(k_memTypes[i].id) : 0;

      m_MemDataPtrLabel[i]  = new QLabel(fmtMemPtr(ptr));
      m_MemDataSizeLabel[i] = new QLabel(fmtMemSize(sz));
      makeSmallGray(m_MemDataPtrLabel[i]);
      makeSmallGray(m_MemDataSizeLabel[i]);

      form->addRow(tr("Data"), m_MemDataPtrLabel[i]);
      form->addRow(tr("Size"), m_MemDataSizeLabel[i]);
    }

    pageLayout->addWidget(group);

    auto *mapsGroup = new QGroupBox(tr("Memory Descriptors"));
    m_MemMapsForm = new QFormLayout(mapsGroup);
    m_MemMapsForm->setVerticalSpacing(8);
    m_MemMapsShownCount = -1; /* populated on first timer tick */
    pageLayout->addWidget(mapsGroup);

    pageLayout->addStretch();
  }

  /* ── Proc Address ───────────────────────────────────────────── */
  auto *procPage = new QWidget();
  {
    auto *pageLayout = new QVBoxLayout(procPage);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(12);

    /* Symbol query row */
    auto *inputWidget = new QWidget();
    auto *inputHbox   = new QHBoxLayout(inputWidget);
    inputHbox->setContentsMargins(0, 0, 0, 0);
    inputHbox->setSpacing(6);
    auto *symbolInput = new QLineEdit();
    symbolInput->setPlaceholderText(tr("Symbol name…"));
    auto *queryBtn = new QPushButton(tr("Query"));
    inputHbox->addWidget(symbolInput, 1);
    inputHbox->addWidget(queryBtn);
    pageLayout->addWidget(inputWidget);

    /* Button area for resolved functions */
    auto *funcGroup    = new QGroupBox(tr("Found Functions"));
    auto *buttonLayout = new QVBoxLayout(funcGroup);
    buttonLayout->setSpacing(6);

    /* Restore buttons for symbols confirmed in previous queries */
    for (const QString &sym : m_ProcSymbols)
    {
      auto *btn = new QPushButton(sym);
      connect(btn, &QPushButton::clicked, this, [this, sym]() {
        m_Owner->procAddress()->call(sym.toLocal8Bit().constData());
      });
      buttonLayout->addWidget(btn);
    }
    buttonLayout->addStretch();

    pageLayout->addWidget(funcGroup, 1);

    auto *notFoundLabel = new QLabel();
    notFoundLabel->setForegroundRole(QPalette::Highlight);
    notFoundLabel->hide();
    pageLayout->addWidget(notFoundLabel);

    /* Look up the typed symbol; add a call button if it resolves. */
    auto doQuery = [this, symbolInput, buttonLayout, notFoundLabel]() {
      const QString sym = symbolInput->text().trimmed();
      if (sym.isEmpty() || m_ProcSymbols.contains(sym))
        return;
      if (m_Owner->procAddress()->get(sym.toLocal8Bit().constData()))
      {
        notFoundLabel->hide();
        m_ProcSymbols.append(sym);
        auto *btn = new QPushButton(sym);
        connect(btn, &QPushButton::clicked, this, [this, sym]() {
          m_Owner->procAddress()->call(sym.toLocal8Bit().constData());
        });
        /* Insert before the trailing stretch */
        QLayoutItem *stretch = buttonLayout->takeAt(buttonLayout->count() - 1);
        buttonLayout->addWidget(btn);
        buttonLayout->addItem(stretch);
        symbolInput->clear();
      }
      else
      {
        notFoundLabel->setText(tr("Symbol \"%1\" was not recognized by the core.").arg(sym));
        notFoundLabel->show();
      }
    };

    connect(queryBtn,    &QPushButton::clicked,     this, [doQuery]() { doQuery(); });
    connect(symbolInput, &QLineEdit::returnPressed,  this, [doQuery]() { doQuery(); });
  }

  /* ── Logging ────────────────────────────────────────────────── */
  auto *logEdit = new QTextEdit();
  logEdit->setReadOnly(true);
  logEdit->setFont(QFont("monospace"));
  logEdit->setFrameShape(QFrame::NoFrame);
  logEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  {
    auto levelPrefix = [](int level) -> QString {
      switch (level) {
      case RETRO_LOG_DEBUG: return "[D] ";
      case RETRO_LOG_INFO:  return "[I] ";
      case RETRO_LOG_WARN:  return "[W] ";
      case RETRO_LOG_ERROR: return "[E] ";
      default:              return "    ";
      }
    };
    auto levelColor = [](int level) -> QString {
      switch (level) {
      case RETRO_LOG_DEBUG: return "#808080";
      case RETRO_LOG_WARN:  return "#c87800";
      case RETRO_LOG_ERROR: return "#b40000";
      default:              return "";
      }
    };
    auto addEntry = [logEdit, levelPrefix, levelColor](int level, const QString &msg) {
      QString color = levelColor(level);
      QString text = levelPrefix(level) + msg.toHtmlEscaped();
      if (!color.isEmpty())
        logEdit->append(QString("<span style=\"color:%1\">%2</span>").arg(color, text));
      else
        logEdit->append(text);
    };

    for (const auto &entry : m_Owner->log()->entries())
      addEntry(static_cast<int>(entry.level), entry.message);

    connect(m_Owner, &QRetro::onCoreLog, logEdit,
            [addEntry](int level, const QString msg) {
      addEntry(level, msg);
    });
  }

  /* ── Messages ───────────────────────────────────────────────── */
  auto *msgEdit = new QTextEdit();
  msgEdit->setReadOnly(true);
  msgEdit->setFont(QFont("monospace"));
  msgEdit->setFrameShape(QFrame::NoFrame);
  msgEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  {
    auto levelColor = [](retro_log_level level) -> QString {
      switch (level) {
      case RETRO_LOG_DEBUG: return "#808080";
      case RETRO_LOG_WARN:  return "#c87800";
      case RETRO_LOG_ERROR: return "#b40000";
      default:              return "";
      }
    };
    auto addEntry = [msgEdit, levelColor](const QRetroMessageEntry &entry) {
      QString color = levelColor(entry.level);
      QString text  = entry.message.toHtmlEscaped();
      if (!color.isEmpty())
        msgEdit->append(QString("<span style=\"color:%1\">%2</span>").arg(color, text));
      else
        msgEdit->append(text);
    };

    for (const auto &entry : m_Owner->message()->entries())
      addEntry(entry);

    connect(m_Owner->message(), &QRetroMessage::onMessage, msgEdit,
            [addEntry](const QRetroMessageEntry entry) {
      addEntry(entry);
    });
  }

  /* ── Core Options ───────────────────────────────────────────── */
  auto *optionsWidget = m_Owner->options();
  optionsWidget->update();

  /* ── Sidebar ────────────────────────────────────────────────── */
  const QString coreName  = m_Owner->options()->coreName();
  const QString coreLabel = coreName.isEmpty() ? tr("Core Options")
                                               : tr("%1 Options").arg(coreName);

  auto *sidebar = new QListWidget();
  sidebar->setFrameShape(QFrame::NoFrame);
  sidebar->setStyleSheet("QListWidget::item { padding: 6px 10px; }");

  int stackIdx = 0;
  QListWidgetItem *firstItem = nullptr;

  auto addDivider = [&](const char *text) {
    auto *item = new QListWidgetItem(tr(text));
    item->setFlags(Qt::ItemIsEnabled);
    QFont f = item->font();
    f.setPointSize(qMax(f.pointSize() - 2, 7));
    item->setFont(f);
    item->setForeground(sidebar->palette().color(QPalette::Mid));
    sidebar->addItem(item);
  };

  auto addSidebarItem = [&](const QString &text) {
    auto *item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, stackIdx++);
    sidebar->addItem(item);
    if (!firstItem) firstItem = item;
  };

  addDivider("GENERAL");
  addSidebarItem(coreLabel);
  addSidebarItem(tr("Video"));
  addSidebarItem(tr("Audio"));
  addSidebarItem(tr("Environment"));
  addSidebarItem(tr("Directories"));

  addDivider("ADVANCED");
  addSidebarItem(tr("Sensors"));
  addSidebarItem(tr("Location"));
  addSidebarItem(tr("LED"));

  addDivider("DEVELOPER");
  addSidebarItem(tr("Core Constants"));
  addSidebarItem(tr("Memory"));
  addSidebarItem(tr("Proc Address"));
  addSidebarItem(tr("Logging"));
  addSidebarItem(tr("Messages"));

  sidebar->setCurrentItem(firstItem);

  /* ── Stacked settings area ──────────────────────────────────── */
  auto *stack = new QStackedWidget();
  stack->addWidget(optionsWidget);
  stack->addWidget(makeScrollPage(videoPage));
  stack->addWidget(makeScrollPage(audioPage));
  stack->addWidget(makeScrollPage(envPage));
  stack->addWidget(makeScrollPage(dirsPage));
  stack->addWidget(makeScrollPage(sensorsPage));
  stack->addWidget(makeScrollPage(locationPage));
  stack->addWidget(makeScrollPage(ledPage));
  stack->addWidget(makeScrollPage(coreConstPage));
  stack->addWidget(makeScrollPage(memPage));
  stack->addWidget(makeScrollPage(procPage));
  stack->addWidget(logEdit);
  stack->addWidget(msgEdit);

  connect(sidebar, &QListWidget::currentItemChanged,
          [stack](QListWidgetItem *current, QListWidgetItem *) {
    if (!current) return;
    QVariant v = current->data(Qt::UserRole);
    if (v.isValid())
      stack->setCurrentIndex(v.toInt());
  });

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
  resize(820, 400);
}
