#include <functional>

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
#include <QThread>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "QRetroConfig.h"
#include "QRetro.h"

static const struct
{
  const char *name;
  retro_language id;
} k_languages[] = {
  { "English", RETRO_LANGUAGE_ENGLISH },
  { "Japanese", RETRO_LANGUAGE_JAPANESE },
  { "French", RETRO_LANGUAGE_FRENCH },
  { "Spanish", RETRO_LANGUAGE_SPANISH },
  { "German", RETRO_LANGUAGE_GERMAN },
  { "Italian", RETRO_LANGUAGE_ITALIAN },
  { "Dutch", RETRO_LANGUAGE_DUTCH },
  { "Portuguese (Brazil)", RETRO_LANGUAGE_PORTUGUESE_BRAZIL },
  { "Portuguese (Portugal)", RETRO_LANGUAGE_PORTUGUESE_PORTUGAL },
  { "Russian", RETRO_LANGUAGE_RUSSIAN },
  { "Korean", RETRO_LANGUAGE_KOREAN },
  { "Chinese (Traditional)", RETRO_LANGUAGE_CHINESE_TRADITIONAL },
  { "Chinese (Simplified)", RETRO_LANGUAGE_CHINESE_SIMPLIFIED },
  { "Esperanto", RETRO_LANGUAGE_ESPERANTO },
  { "Polish", RETRO_LANGUAGE_POLISH },
  { "Vietnamese", RETRO_LANGUAGE_VIETNAMESE },
  { "Arabic", RETRO_LANGUAGE_ARABIC },
  { "Greek", RETRO_LANGUAGE_GREEK },
  { "Turkish", RETRO_LANGUAGE_TURKISH },
  { "Slovak", RETRO_LANGUAGE_SLOVAK },
  { "Persian", RETRO_LANGUAGE_PERSIAN },
  { "Hebrew", RETRO_LANGUAGE_HEBREW },
  { "Asturian", RETRO_LANGUAGE_ASTURIAN },
  { "Finnish", RETRO_LANGUAGE_FINNISH },
  { "Indonesian", RETRO_LANGUAGE_INDONESIAN },
  { "Swedish", RETRO_LANGUAGE_SWEDISH },
  { "Ukrainian", RETRO_LANGUAGE_UKRAINIAN },
  { "Czech", RETRO_LANGUAGE_CZECH },
  { "Catalan (Valencia)", RETRO_LANGUAGE_CATALAN_VALENCIA },
  { "Catalan", RETRO_LANGUAGE_CATALAN },
  { "English (British)", RETRO_LANGUAGE_BRITISH_ENGLISH },
  { "Hungarian", RETRO_LANGUAGE_HUNGARIAN },
  { "Belarusian", RETRO_LANGUAGE_BELARUSIAN },
  { "Galician", RETRO_LANGUAGE_GALICIAN },
  { "Norwegian", RETRO_LANGUAGE_NORWEGIAN },
  { "Irish", RETRO_LANGUAGE_IRISH },
};

static const int k_languageCount = int(sizeof(k_languages) / sizeof(k_languages[0]));

static const struct
{
  const char *name;
  unsigned id;
} k_memTypes[4] = {
  { "RETRO_MEMORY_SAVE_RAM", RETRO_MEMORY_SAVE_RAM },
  { "RETRO_MEMORY_RTC", RETRO_MEMORY_RTC },
  { "RETRO_MEMORY_SYSTEM_RAM", RETRO_MEMORY_SYSTEM_RAM },
  { "RETRO_MEMORY_VIDEO_RAM", RETRO_MEMORY_VIDEO_RAM },
};

static QString fmtMemPtr(void *p)
{
  if (!p)
    return QStringLiteral("—");
  return QString("0x") +
         QStringLiteral("%1")
           .arg(reinterpret_cast<quintptr>(p), int(sizeof(void *)) * 2, 16, QChar('0'))
           .toUpper();
}

static QString fmtMemSize(size_t s)
{
  if (!s)
    return QStringLiteral("—");
  if (s >= 1024 * 1024)
    return QStringLiteral("%1 bytes (%2 MB)").arg(s).arg(s / (1024 * 1024));
  if (s >= 1024)
    return QStringLiteral("%1 bytes (%2 KB)").arg(s).arg(s / 1024);
  return QStringLiteral("%1 bytes").arg(s);
}

static QString fmtHex(uint64_t v)
{
  if (!v)
    return QStringLiteral("0x0");
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
  m_Owner = owner;
  m_Filename = QDir::currentPath() + "/config.ini";

  m_SaveTimer = new QTimer(this);
  m_SaveTimer->setSingleShot(true);
  m_SaveTimer->setInterval(400);
  connect(m_SaveTimer, &QTimer::timeout, [this]() {
    if (m_Owner && m_Owner->isActive())
      save();
  });

  load();

  /* Apply persisted values to the owning QRetro's modules. */
  m_Owner->m_AudioEnabled = m_AudioEnabled;
  if (m_Owner->m_Audio)
    m_Owner->m_Audio->setVolume(m_AudioVolume);
  m_Owner->m_UseAspectRatio = m_UseAspectRatio;
  m_Owner->m_BilinearFilter = m_BilinearFilter;
  m_Owner->m_IntegerScaling = m_IntegerScaling;

  /* Poll sensor read-tracking flags and enable/disable per-axis UI widgets. */
  auto *sensor_read_timer = new QTimer(this);
  sensor_read_timer->setInterval(200);
  connect(sensor_read_timer, &QTimer::timeout, [this]() {
    if (!m_Owner || !m_Owner->isActive())
      return;
    auto *s = m_Owner->sensors();
    setAccelXHasBeenRead(s->accelXHasBeenRead());
    setAccelYHasBeenRead(s->accelYHasBeenRead());
    setAccelZHasBeenRead(s->accelZHasBeenRead());
    setGyroXHasBeenRead(s->gyroXHasBeenRead());
    setGyroYHasBeenRead(s->gyroYHasBeenRead());
    setGyroZHasBeenRead(s->gyroZHasBeenRead());
    setIllumHasBeenRead(s->illumHasBeenRead());

    auto yes_no = [](bool v) { return v ? QStringLiteral("Yes") : QStringLiteral("No"); };
    auto hz = [](unsigned r) {
      return r ? QString::number(r) + QStringLiteral(" Hz") : QStringLiteral("—");
    };

    if (m_AccelEnabledLabel)
      m_AccelEnabledLabel->setText(yes_no(s->accelEnabled()));
    if (m_AccelRateLabel)
      m_AccelRateLabel->setText(hz(s->accelRate()));
    if (m_GyroEnabledLabel)
      m_GyroEnabledLabel->setText(yes_no(s->gyroEnabled()));
    if (m_GyroRateLabel)
      m_GyroRateLabel->setText(hz(s->gyroRate()));
    if (m_IllumEnabledLabel)
      m_IllumEnabledLabel->setText(yes_no(s->illumEnabled()));
    if (m_IllumRateLabel)
      m_IllumRateLabel->setText(hz(s->illumRate()));

    if (auto *loc = m_Owner->location())
    {
      if (m_LocationStateLabel)
      {
        const char *state_str = "—";
        switch (loc->state())
        {
        case QRetroLocation::Uninitialized:
          state_str = "Uninitialized";
          break;
        case QRetroLocation::Started:
          state_str = "Started";
          break;
        case QRetroLocation::Stopped:
          state_str = "Stopped";
          break;
        }
        m_LocationStateLabel->setText(tr(state_str));
      }
      auto ms = [](unsigned v) {
        return v ? QString::number(v) + QStringLiteral(" ms") : QStringLiteral("—");
      };
      auto dist = [](unsigned v) {
        return v ? QString::number(v) + QStringLiteral(" m") : QStringLiteral("—");
      };
      if (m_LocationIntervalLabel)
        m_LocationIntervalLabel->setText(ms(loc->millisecondInterval()));
      if (m_LocationDistLabel)
        m_LocationDistLabel->setText(dist(loc->distanceInterval()));
    }

    if (m_MicForm)
    {
      int idx = 0;
      for (auto *handle : m_Owner->microphone()->openMics())
      {
        retro_microphone_params_t params{};
        bool active = m_Owner->microphone()->getState(handle);
        unsigned rate = 0;
        m_Owner->microphone()->getParams(handle, &params);
        rate = params.rate;
        QString rate_str = rate ? QString::number(rate) + tr(" Hz") : QStringLiteral("—");

        if (!m_MicStateLabels.contains(handle))
        {
          auto *state_label = new QLabel(active ? tr("Recording") : tr("Stopped"));
          auto *rate_label = new QLabel(rate_str);
          m_MicStateLabels[handle] = state_label;
          m_MicRateLabels[handle] = rate_label;
          m_MicForm->addRow(tr("Instance %1 State").arg(idx + 1), state_label);
          m_MicForm->addRow(tr("Instance %1 Rate").arg(idx + 1), rate_label);
          if (m_MicEmptyLabel)
            m_MicEmptyLabel->setVisible(false);
        }
        else
        {
          m_MicStateLabels[handle]->setText(active ? tr("Recording") : tr("Stopped"));
          if (m_MicRateLabels.contains(handle))
            m_MicRateLabels[handle]->setText(rate_str);
        }
        ++idx;
      }
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

    auto yes_no_str = [this](bool set, bool v) -> QString {
      return set ? (v ? tr("Yes") : tr("No")) : tr("Unset");
    };

    if (m_CoreAchievementsLabel)
      m_CoreAchievementsLabel->setText(
        yes_no_str(m_Owner->supportsAchievementsSet(), m_Owner->supportsAchievements()));

    if (m_CorePerfLevelLabel)
      m_CorePerfLevelLabel->setText(m_Owner->performanceLevelSet()
                                      ? QString::number(m_Owner->performanceLevel())
                                      : tr("Unset"));

    if (m_CorePixelFormatLabel)
    {
      QString s = tr("Unset");
      if (m_Owner->pixelFormatSet())
      {
        switch (m_Owner->retroPixelFormat())
        {
        case RETRO_PIXEL_FORMAT_0RGB1555:
          s = QStringLiteral("0RGB1555");
          break;
        case RETRO_PIXEL_FORMAT_XRGB8888:
          s = QStringLiteral("XRGB8888");
          break;
        case RETRO_PIXEL_FORMAT_RGB565:
          s = QStringLiteral("RGB565");
          break;
        default:
          s = QStringLiteral("Unknown");
          break;
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
        yes_no_str(m_Owner->supportsNoGameSet(), m_Owner->supportsNoGame()));

    if (m_RaSaveStateInBackgroundLabel)
      m_RaSaveStateInBackgroundLabel->setText(
        yes_no_str(m_Owner->raSaveStateInBackgroundSet(), m_Owner->raSaveStateInBackground()));

    if (m_RaPollTypeOverrideLabel)
      m_RaPollTypeOverrideLabel->setText(m_Owner->raPollTypeOverrideSet()
                                           ? QString::number(m_Owner->raPollTypeOverride())
                                           : tr("Unset"));

    if (m_RaSaveStateDisableUndoLabel)
      m_RaSaveStateDisableUndoLabel->setText(
        yes_no_str(m_Owner->raSaveStateDisableUndoSet(), m_Owner->raSaveStateDisableUndo()));

    if (m_RaClearAllThreadWaitsLabel)
      m_RaClearAllThreadWaitsLabel->setText(
        m_Owner->raClearAllThreadWaitsRequested() ? tr("Yes") : tr("No"));

    if (m_MemoryReady && m_Owner->m_Core.retro_get_memory_data)
    {
      for (int i = 0; i < 4; i++)
      {
        if (!m_MemDataPtrLabel[i])
          continue;
        void *ptr = m_Owner->m_Core.retro_get_memory_data(k_memTypes[i].id);
        size_t sz = m_Owner->m_Core.retro_get_memory_size
                      ? m_Owner->m_Core.retro_get_memory_size(k_memTypes[i].id)
                      : 0;
        m_MemDataPtrLabel[i]->setText(fmtMemPtr(ptr));
        m_MemDataSizeLabel[i]->setText(fmtMemSize(sz));
      }
    }

    if (m_MemMapsForm)
    {
      auto *maps = m_Owner->m_Memory.memoryMaps();
      int count = maps ? (int)maps->num_descriptors : 0;
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

            QString body = QStringLiteral("flags=%1  ptr=%2  offset=%3\n"
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
            block->setStyleSheet("QLabel { background: palette(alternate-base); "
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
  sensor_read_timer->start();

  connect(m_Owner, &QRetro::onCoreStart, this, [this]() {
    m_MemoryReady = false;
    QTimer::singleShot(5000, this, [this]() { m_MemoryReady = true; });
  });

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
  m_Language =
    static_cast<retro_language>(settings.value("language", RETRO_LANGUAGE_ENGLISH).toInt());
  m_UseAspectRatio = settings.value("useAspectRatio", true).toBool();
  m_IntegerScaling = settings.value("integerScaling", false).toBool();
  m_BilinearFilter = settings.value("bilinearFilter", true).toBool();
  m_AudioEnabled = settings.value("audioEnabled", true).toBool();
  m_AudioVolume = settings.value("audioVolume", 1.0f).toFloat();

  m_SpoofLocationEnabled = settings.value("spoofLocationEnabled", false).toBool();
  m_SpoofLat = settings.value("spoofLat", 0.0).toDouble();
  m_SpoofLon = settings.value("spoofLon", 0.0).toDouble();
  m_SpoofHAcc = settings.value("spoofHAcc", 0.0).toDouble();
  m_SpoofVAcc = settings.value("spoofVAcc", 0.0).toDouble();

  auto *dirs = m_Owner->directories();
  if (settings.contains("dirSave"))
    dirs->set(QRetroDirectories::Save, settings.value("dirSave").toString(), true);
  if (settings.contains("dirSystem"))
    dirs->set(QRetroDirectories::System, settings.value("dirSystem").toString(), true);
  if (settings.contains("dirCoreAssets"))
    dirs->set(QRetroDirectories::CoreAssets, settings.value("dirCoreAssets").toString(), true);
  if (settings.contains("dirPlaylist"))
    dirs->set(QRetroDirectories::Playlist, settings.value("dirPlaylist").toString(), true);
  if (settings.contains("dirFileBrowserStart"))
    dirs->set(
      QRetroDirectories::FileBrowserStart, settings.value("dirFileBrowserStart").toString(), true);
  if (settings.contains("dirState"))
    dirs->set(QRetroDirectories::State, settings.value("dirState").toString(), true);
}

void QRetroConfig::save()
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  settings.beginGroup("QRetro");
  settings.setValue("language", static_cast<int>(m_Language));
  settings.setValue("useAspectRatio", m_UseAspectRatio);
  settings.setValue("integerScaling", m_IntegerScaling);
  settings.setValue("bilinearFilter", m_BilinearFilter);
  settings.setValue("audioEnabled", m_AudioEnabled);
  settings.setValue("audioVolume", m_AudioVolume);

  settings.setValue("spoofLocationEnabled", m_SpoofLocationEnabled);
  settings.setValue("spoofLat", m_SpoofLat);
  settings.setValue("spoofLon", m_SpoofLon);
  settings.setValue("spoofHAcc", m_SpoofHAcc);
  settings.setValue("spoofVAcc", m_SpoofVAcc);

  auto *dirs = m_Owner->directories();
  settings.setValue("dirSave", dirs->get(QRetroDirectories::Save));
  settings.setValue("dirSystem", dirs->get(QRetroDirectories::System));
  settings.setValue("dirCoreAssets", dirs->get(QRetroDirectories::CoreAssets));
  settings.setValue("dirPlaylist", dirs->get(QRetroDirectories::Playlist));
  settings.setValue("dirFileBrowserStart", dirs->get(QRetroDirectories::FileBrowserStart));
  settings.setValue("dirState", dirs->get(QRetroDirectories::State));

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

  m_LedForm = nullptr;
  m_LedEmptyLabel = nullptr;
  m_LedLabels.clear();
  m_MicForm = nullptr;
  m_MicEmptyLabel = nullptr;
  m_MicStateLabels.clear();
  m_MicRateLabels.clear();
  m_CoreAchievementsLabel = nullptr;
  m_CorePerfLevelLabel = nullptr;
  m_CorePixelFormatLabel = nullptr;
  m_CoreSerializationLabel = nullptr;
  m_CoreSupportsNoGameLabel = nullptr;
  m_RaSaveStateInBackgroundLabel = nullptr;
  m_RaPollTypeOverrideLabel = nullptr;
  m_RaSaveStateDisableUndoLabel = nullptr;
  m_RaClearAllThreadWaitsLabel = nullptr;
  for (int i = 0; i < 4; i++)
    m_MemDataPtrLabel[i] = m_MemDataSizeLabel[i] = nullptr;
  m_MemMapsForm = nullptr;
  m_MemoryReady = false;
  m_MemMapsShownCount = -1;

  /* Wraps a form widget in a borderless scroll area. */
  auto make_scroll_page = [](QWidget *inner) -> QScrollArea * {
    auto *scroll = new QScrollArea();
    scroll->setWidget(inner);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setFocusPolicy(Qt::NoFocus);
    return scroll;
  };

  /* ── Video ─────────────────────────────────────────────────── */
  auto *video_page = new QWidget();
  {
    auto *form = new QFormLayout(video_page);
    form->setContentsMargins(12, 12, 12, 12);
    form->setVerticalSpacing(10);

    auto *aspect_ratio = new QCheckBox();
    aspect_ratio->setChecked(m_UseAspectRatio);
    connect(aspect_ratio, &QCheckBox::stateChanged, [this](int state) {
      m_UseAspectRatio = state != Qt::Unchecked;
      m_SaveTimer->start();
      m_Owner->m_UseAspectRatio = m_UseAspectRatio;
      m_Owner->updateScaling();
      emit aspectRatioChanged(m_UseAspectRatio);
    });
    form->addRow(tr("Aspect Ratio"), aspect_ratio);

    auto *int_scaling = new QCheckBox();
    int_scaling->setChecked(m_IntegerScaling);
    connect(int_scaling, &QCheckBox::stateChanged, [this](int state) {
      m_IntegerScaling = state != Qt::Unchecked;
      m_SaveTimer->start();
      m_Owner->m_IntegerScaling = m_IntegerScaling;
      m_Owner->updateScaling();
      emit integerScalingChanged(m_IntegerScaling);
    });
    form->addRow(tr("Integer Scaling"), int_scaling);

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
  auto *audio_page = new QWidget();
  {
    auto *form = new QFormLayout(audio_page);
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

    auto *vol_slider = new QSlider(Qt::Horizontal);
    vol_slider->setRange(0, 100);
    vol_slider->setValue(qRound(m_AudioVolume * 100.0f));
    vol_slider->setTickPosition(QSlider::TicksBelow);
    vol_slider->setTickInterval(10);

    auto *vol_spin = new QSpinBox();
    vol_spin->setRange(0, 100);
    vol_spin->setSuffix("%");
    vol_spin->setValue(qRound(m_AudioVolume * 100.0f));
    vol_spin->setFixedWidth(70);

    /* Slider → spinbox + apply */
    connect(vol_slider, &QSlider::valueChanged, [this, vol_spin](int v) {
      vol_spin->blockSignals(true);
      vol_spin->setValue(v);
      vol_spin->blockSignals(false);
      m_AudioVolume = v / 100.0f;
      m_SaveTimer->start();
      if (m_Owner->m_Audio)
        m_Owner->m_Audio->setVolume(m_AudioVolume);
    });

    /* Spinbox → slider + apply */
    connect(vol_spin, QOverload<int>::of(&QSpinBox::valueChanged), [this, vol_slider](int v) {
      vol_slider->blockSignals(true);
      vol_slider->setValue(v);
      vol_slider->blockSignals(false);
      m_AudioVolume = v / 100.0f;
      m_SaveTimer->start();
      if (m_Owner->m_Audio)
        m_Owner->m_Audio->setVolume(m_AudioVolume);
    });

    auto *vol_row = new QWidget();
    auto *vol_box = new QHBoxLayout(vol_row);
    vol_box->setContentsMargins(0, 0, 0, 0);
    vol_box->addWidget(vol_slider, 1);
    vol_box->addWidget(vol_spin);
    form->addRow(tr("Volume"), vol_row);
  }

  /* ── Input ──────────────────────────────────────────────────── */
  auto *input_page = new QWidget();
  {
    auto *form = new QFormLayout(input_page);
    form->setContentsMargins(12, 12, 12, 12);
    form->setVerticalSpacing(10);

    const auto &ports = m_Owner->input()->controllerPorts();
    for (unsigned p = 0; p < static_cast<unsigned>(ports.size()); p++)
    {
      const auto &port = ports[p];
      if (port.types.empty())
        continue;

      auto *combo = new QComboBox();
      for (const auto &type : port.types)
        combo->addItem(QString::fromStdString(type.desc), type.id);

      unsigned selected = port.selectedId;
      int idx = combo->findData(selected);
      combo->setCurrentIndex(idx >= 0 ? idx : 0);

      /* Notify the core of the current selection now that the game is loaded. */
      m_Owner->input()->setSelectedControllerType(p, selected);

      connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, combo, p](int) {
        unsigned id = static_cast<unsigned>(combo->currentData().toUInt());
        m_Owner->input()->setSelectedControllerType(p, id);
      });

      form->addRow(tr("Port %1").arg(p + 1), combo);
    }

    if (ports.empty())
    {
      auto *label = new QLabel(tr("No controller info provided by core."));
      label->setForegroundRole(QPalette::Dark);
      form->addRow(label);
    }
  }

  /* ── Environment ────────────────────────────────────────────── */
  auto *env_page = new QWidget();
  {
    auto *form = new QFormLayout(env_page);
    form->setContentsMargins(12, 12, 12, 12);
    form->setVerticalSpacing(10);
    auto *combo = new QComboBox();

    for (int i = 0; i < k_languageCount; i++)
      combo->addItem(k_languages[i].name, static_cast<int>(k_languages[i].id));
    combo->setCurrentIndex(combo->findData(static_cast<int>(m_Language)));

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, combo](int) {
      m_Language = static_cast<retro_language>(combo->currentData().toInt());
      m_SaveTimer->start();
    });
    form->addRow(tr("Language"), combo);
  }

  /* ── Directories ────────────────────────────────────────────── */
  auto *dirs_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(dirs_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *group = new QGroupBox(tr("Directories"));
    auto *form = new QFormLayout(group);
    form->setVerticalSpacing(8);

    struct
    {
      const char *label;
      QRetroDirectories::Type type;
    } rows[] = {
      { "Save", QRetroDirectories::Save },
      { "System", QRetroDirectories::System },
      { "Core Assets", QRetroDirectories::CoreAssets },
      { "Playlist", QRetroDirectories::Playlist },
      { "File Browser Start", QRetroDirectories::FileBrowserStart },
      { "State", QRetroDirectories::State },
    };

    for (auto &row : rows)
    {
      auto *edit = new QLineEdit(m_Owner->directories()->get(row.type));

      QRetroDirectories::Type type = row.type;
      auto *browse_btn = new QPushButton(tr("Browse…"));
      connect(browse_btn, &QPushButton::clicked, [this, edit, type]() {
        QString path =
          QFileDialog::getExistingDirectory(this, tr("Select Directory"), edit->text());
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

      auto *row_widget = new QWidget();
      auto *hbox = new QHBoxLayout(row_widget);
      hbox->setContentsMargins(0, 0, 0, 0);
      hbox->addWidget(edit, 1);
      hbox->addWidget(browse_btn);
      form->addRow(tr(row.label), row_widget);
    }

    page_layout->addWidget(group);
    page_layout->addStretch();
  }

  /* ── Disk Control ───────────────────────────────────────────── */
  auto *disk_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(disk_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *dc = m_Owner->diskControl();
    const bool available = dc->version() != QRetroDiskControl::Invalid;

    /* Status labels — declared here so controls below can update them */
    QLabel *tray_label = nullptr;
    QLabel *active_image_label = nullptr;

    /* Status group */
    {
      auto *group = new QGroupBox(tr("Status"));
      auto *form = new QFormLayout(group);
      form->setVerticalSpacing(8);

      QString version_str;
      switch (dc->version())
      {
      case QRetroDiskControl::v0:
        version_str = QStringLiteral("v0");
        break;
      case QRetroDiskControl::v1:
        version_str = QStringLiteral("v1 (ext)");
        break;
      default:
        version_str = tr("Not available");
        break;
      }
      form->addRow(tr("Interface"), new QLabel(version_str));

      if (available)
      {
        tray_label = new QLabel(dc->getEjectState() ? tr("Ejected") : tr("Inserted"));
        form->addRow(tr("Tray"), tray_label);

        unsigned num_images = dc->getNumImages();
        unsigned cur_index = dc->getImageIndex();
        form->addRow(tr("Images"), new QLabel(QString::number(num_images)));

        active_image_label = new QLabel(QString::number(cur_index));
        form->addRow(tr("Active Image"), active_image_label);
      }

      page_layout->addWidget(group);
    }

    auto update_status = [dc, tray_label, active_image_label]() {
      if (tray_label)
        tray_label->setText(dc->getEjectState() ? tr("Ejected") : tr("Inserted"));
      if (active_image_label)
        active_image_label->setText(QString::number(dc->getImageIndex()));
    };

    if (available)
    {
      /* Drive controls */
      {
        auto *group = new QGroupBox(tr("Drive"));
        auto *form = new QFormLayout(group);
        form->setVerticalSpacing(8);

        /* Tray toggle — eject or insert only, no index change here.
         * A background thread blocks on execOnTimingThread so the core
         * calls land between frames without freezing the UI. */
        auto *eject_btn = new QPushButton(dc->getEjectState() ? tr("Insert") : tr("Eject"));
        connect(eject_btn, &QPushButton::clicked, [this, dc, eject_btn, update_status]() {
          bool now_ejected = !dc->getEjectState();
          auto *t = QThread::create([this, dc, eject_btn, now_ejected, update_status]() {
            m_Owner->execOnTimingThread([dc, now_ejected]() { dc->setEjectState(now_ejected); });
            QMetaObject::invokeMethod(
              eject_btn,
              [eject_btn, now_ejected, update_status]() {
                eject_btn->setText(now_ejected ? tr("Insert") : tr("Eject"));
                update_status();
              },
              Qt::QueuedConnection);
          });
          connect(t, &QThread::finished, t, &QObject::deleteLater);
          t->start();
        });
        form->addRow(tr("Tray"), eject_btn);

        /* Disk swap: eject → set index → insert, executed on the timing
         * thread between frames per the libretro spec. */
        unsigned num_images = dc->getNumImages();
        unsigned cur_index = dc->getImageIndex();
        auto *disk_combo = new QComboBox();
        for (unsigned i = 0; i < num_images; ++i)
        {
          char label[1024] = {};
          char path[4096] = {};
          QString display;
          if (dc->version() == QRetroDiskControl::v1)
          {
            auto basename = [](const char *s) -> QString {
              QFileInfo fi(QString::fromUtf8(s));
              return fi.fileName().isEmpty() ? fi.filePath() : fi.fileName();
            };
            dc->getImageLabel(i, label, sizeof(label));
            if (label[0])
              display = basename(label);
            else
            {
              dc->getImagePath(i, path, sizeof(path));
              display = path[0] ? basename(path) : QStringLiteral("—");
            }
          }
          else
            display = tr("Disk %1").arg(i + 1);
          disk_combo->addItem(display, i);
        }
        disk_combo->setCurrentIndex(static_cast<int>(cur_index));

        auto *swap_btn = new QPushButton(tr("Swap"));
        connect(swap_btn, &QPushButton::clicked, [this, dc, disk_combo, swap_btn, update_status]() {
          unsigned target = static_cast<unsigned>(disk_combo->currentData().toUInt());
          swap_btn->setEnabled(false);
          auto *t = QThread::create([this, dc, target, swap_btn, update_status]() {
            m_Owner->execOnTimingThread([dc, target]() { dc->setImageIndex(target); });
            QMetaObject::invokeMethod(
              swap_btn,
              [swap_btn, update_status]() {
                swap_btn->setEnabled(true);
                update_status();
              },
              Qt::QueuedConnection);
          });
          connect(t, &QThread::finished, t, &QObject::deleteLater);
          t->start();
        });

        auto *row_widget = new QWidget();
        auto *hbox = new QHBoxLayout(row_widget);
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->addWidget(disk_combo, 1);
        hbox->addWidget(swap_btn);
        form->addRow(tr("Swap Disk"), row_widget);

        auto *add_btn = new QPushButton(tr("Add Slot"));
        connect(add_btn, &QPushButton::clicked, [dc, disk_combo]() {
          if (dc->addImageIndex())
            disk_combo->addItem(tr("Disk %1").arg(disk_combo->count() + 1), disk_combo->count());
        });
        form->addRow(tr("Slots"), add_btn);

        page_layout->addWidget(group);
      }

      /* Image list (v1 only) */
      if (dc->version() == QRetroDiskControl::v1)
      {
        auto *group = new QGroupBox(tr("Images"));
        auto *form = new QFormLayout(group);
        form->setVerticalSpacing(8);

        unsigned num_images = dc->getNumImages();
        for (unsigned i = 0; i < num_images; ++i)
        {
          char path[4096] = {};
          char label[1024] = {};
          dc->getImagePath(i, path, sizeof(path));
          dc->getImageLabel(i, label, sizeof(label));

          QString display = QString::fromUtf8(label[0] ? label : path[0] ? path : "—");
          auto *lbl = new QLabel(display);
          lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
          lbl->setToolTip(QString::fromUtf8(path));
          form->addRow(tr("Slot %1").arg(i), lbl);
        }

        if (num_images == 0)
          form->addRow(new QLabel(tr("No images registered.")));

        page_layout->addWidget(group);
      }
    }

    page_layout->addStretch();
  }

  /* ── Sensors ────────────────────────────────────────────────── */
  auto *sensors_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(sensors_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

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
    auto make_axis_widget = [](float initVal, double lo, double hi, int scale,
                              int decimals = 2) -> QPair<QWidget *, QDoubleSpinBox *> {
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
        [spin, scale](int v) { spin->setValue(double(v) / scale); });

      /* Spinbox → slider (blocked to prevent re-entry) */
      QObject::connect(
        spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [slider, scale](double v) {
          slider->blockSignals(true);
          slider->setValue(qRound(v * scale));
          slider->blockSignals(false);
        });

      auto *w = new QWidget();
      auto *hbox = new QHBoxLayout(w);
      hbox->setContentsMargins(0, 0, 0, 0);
      hbox->addWidget(slider, 1);
      hbox->addWidget(spin);
      return { w, spin };
    };

    /* ── Accelerometer ──────────────────────────────────────── */
    {
      auto *group = new QGroupBox(tr("Accelerometer (m/s²)"));
      auto *form = new QFormLayout(group);
      form->setVerticalSpacing(8);

      m_AccelEnabledLabel = new QLabel(tr("—"));
      m_AccelRateLabel = new QLabel(tr("—"));
      form->addRow(tr("Enabled"), m_AccelEnabledLabel);
      form->addRow(tr("Rate"), m_AccelRateLabel);

      auto *spoof_check = new QCheckBox(tr("Spoof values"));
      spoof_check->setChecked(m_SpoofAccelEnabled);

      auto [wX, spinX] = make_axis_widget(m_SpoofAccel[0], -20.0, 20.0, 100);
      auto [wY, spinY] = make_axis_widget(m_SpoofAccel[1], -20.0, 20.0, 100);
      auto [wZ, spinZ] = make_axis_widget(m_SpoofAccel[2], -20.0, 20.0, 100);
      m_AccelAxisWidget[0] = wX;
      wX->setEnabled(false);
      m_AccelAxisWidget[1] = wY;
      wY->setEnabled(false);
      m_AccelAxisWidget[2] = wZ;
      wZ->setEnabled(false);

      auto emit_accel = [this, spoof_check, spinX, spinY, spinZ]() {
        m_SpoofAccelEnabled = spoof_check->isChecked();
        m_SpoofAccel[0] = static_cast<float>(spinX->value());
        m_SpoofAccel[1] = static_cast<float>(spinY->value());
        m_SpoofAccel[2] = static_cast<float>(spinZ->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setSpoofAccelEnabled(m_SpoofAccelEnabled);
        m_Owner->sensors()->setSpoofAccel(m_SpoofAccel[0], m_SpoofAccel[1], m_SpoofAccel[2]);
        emit spoofAccelChanged(
          m_SpoofAccelEnabled, m_SpoofAccel[0], m_SpoofAccel[1], m_SpoofAccel[2]);
      };

      connect(spoof_check, &QCheckBox::stateChanged, [emit_accel](int) { emit_accel(); });
      connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_accel](double) { emit_accel(); });
      connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_accel](double) { emit_accel(); });
      connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_accel](double) { emit_accel(); });

      form->addRow(tr("Spoof"), spoof_check);
      form->addRow(tr("X"), wX);
      form->addRow(tr("Y"), wY);
      form->addRow(tr("Z"), wZ);

      page_layout->addWidget(group);
    }

    /* ── Gyroscope ──────────────────────────────────────────── */
    {
      auto *group = new QGroupBox(tr("Gyroscope (rad/s)"));
      auto *form = new QFormLayout(group);
      form->setVerticalSpacing(8);

      m_GyroEnabledLabel = new QLabel(tr("—"));
      m_GyroRateLabel = new QLabel(tr("—"));
      form->addRow(tr("Enabled"), m_GyroEnabledLabel);
      form->addRow(tr("Rate"), m_GyroRateLabel);

      auto *spoof_check = new QCheckBox(tr("Spoof values"));
      spoof_check->setChecked(m_SpoofGyroEnabled);

      auto [wX, spinX] = make_axis_widget(m_SpoofGyro[0], -1.0, 1.0, 100);
      auto [wY, spinY] = make_axis_widget(m_SpoofGyro[1], -1.0, 1.0, 100);
      auto [wZ, spinZ] = make_axis_widget(m_SpoofGyro[2], -1.0, 1.0, 100);
      m_GyroAxisWidget[0] = wX;
      wX->setEnabled(false);
      m_GyroAxisWidget[1] = wY;
      wY->setEnabled(false);
      m_GyroAxisWidget[2] = wZ;
      wZ->setEnabled(false);

      auto emit_gyro = [this, spoof_check, spinX, spinY, spinZ]() {
        m_SpoofGyroEnabled = spoof_check->isChecked();
        m_SpoofGyro[0] = static_cast<float>(spinX->value());
        m_SpoofGyro[1] = static_cast<float>(spinY->value());
        m_SpoofGyro[2] = static_cast<float>(spinZ->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setSpoofGyroEnabled(m_SpoofGyroEnabled);
        m_Owner->sensors()->setSpoofGyro(m_SpoofGyro[0], m_SpoofGyro[1], m_SpoofGyro[2]);
        emit spoofGyroChanged(m_SpoofGyroEnabled, m_SpoofGyro[0], m_SpoofGyro[1], m_SpoofGyro[2]);
      };

      connect(spoof_check, &QCheckBox::stateChanged, [emit_gyro](int) { emit_gyro(); });
      connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_gyro](double) { emit_gyro(); });
      connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_gyro](double) { emit_gyro(); });
      connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_gyro](double) { emit_gyro(); });

      form->addRow(tr("Spoof"), spoof_check);
      form->addRow(tr("X"), wX);
      form->addRow(tr("Y"), wY);
      form->addRow(tr("Z"), wZ);

      page_layout->addWidget(group);
    }

    /* ── Illuminance ────────────────────────────────────────── */
    {
      auto *group = new QGroupBox(tr("Illuminance (lux)"));
      auto *form = new QFormLayout(group);
      form->setVerticalSpacing(8);

      m_IllumEnabledLabel = new QLabel(tr("—"));
      m_IllumRateLabel = new QLabel(tr("—"));
      form->addRow(tr("Enabled"), m_IllumEnabledLabel);
      form->addRow(tr("Rate"), m_IllumRateLabel);

      auto *spoof_check = new QCheckBox(tr("Spoof values"));
      spoof_check->setChecked(m_SpoofIllumEnabled);

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
      connect(slider, &QSlider::valueChanged, [spin](int v) { spin->setValue(double(v)); });

      /* Spinbox → slider (disable slider when out of range) */
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [slider](double v) {
        bool in_range = (v >= slider->minimum() && v <= slider->maximum());
        slider->setEnabled(in_range);
        if (in_range)
        {
          slider->blockSignals(true);
          slider->setValue(qRound(v));
          slider->blockSignals(false);
        }
      });

      auto emit_illum = [this, spoof_check, spin]() {
        m_SpoofIllumEnabled = spoof_check->isChecked();
        m_SpoofIllum = static_cast<float>(spin->value());
        m_SaveTimer->start();
        m_Owner->sensors()->setSpoofIllumEnabled(m_SpoofIllumEnabled);
        m_Owner->sensors()->setSpoofIllum(m_SpoofIllum);
        emit spoofIllumChanged(m_SpoofIllumEnabled, m_SpoofIllum);
      };

      connect(spoof_check, &QCheckBox::stateChanged, [emit_illum](int) { emit_illum(); });
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [emit_illum](double) { emit_illum(); });

      auto *value_widget = new QWidget();
      auto *hbox = new QHBoxLayout(value_widget);
      hbox->setContentsMargins(0, 0, 0, 0);
      hbox->addWidget(slider, 1);
      hbox->addWidget(spin);
      m_IllumValueWidget = value_widget;
      value_widget->setEnabled(false);

      form->addRow(tr("Spoof"), spoof_check);
      form->addRow(tr("Value"), value_widget);

      page_layout->addWidget(group);
    }

    page_layout->addStretch();
  }

  /* ── Location ───────────────────────────────────────────────── */
  auto *location_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(location_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *group = new QGroupBox(tr("Position"));
    auto *form = new QFormLayout(group);
    form->setVerticalSpacing(8);

    m_LocationStateLabel = new QLabel(tr("—"));
    m_LocationIntervalLabel = new QLabel(tr("—"));
    m_LocationDistLabel = new QLabel(tr("—"));
    form->addRow(tr("State"), m_LocationStateLabel);
    form->addRow(tr("Time interval (ms)"), m_LocationIntervalLabel);
    form->addRow(tr("Distance interval (m)"), m_LocationDistLabel);

    auto *spoof_check = new QCheckBox(tr("Spoof values"));
    spoof_check->setChecked(m_SpoofLocationEnabled);

    auto make_spin = [](double lo, double hi, double step, int decimals,
                       double init) -> QDoubleSpinBox * {
      auto *s = new QDoubleSpinBox();
      s->setRange(lo, hi);
      s->setSingleStep(step);
      s->setDecimals(decimals);
      s->setValue(init);
      return s;
    };

    auto *lat_spin = make_spin(-90.0, 90.0, 0.000001, 6, m_SpoofLat);
    auto *lon_spin = make_spin(-180.0, 180.0, 0.000001, 6, m_SpoofLon);
    auto *h_acc_spin = make_spin(0.0, 100000.0, 1.0, 1, m_SpoofHAcc);
    auto *v_acc_spin = make_spin(0.0, 100000.0, 1.0, 1, m_SpoofVAcc);

    m_LocationSpoofWidgets[0] = lat_spin;
    m_LocationSpoofWidgets[1] = lon_spin;
    m_LocationSpoofWidgets[2] = h_acc_spin;
    m_LocationSpoofWidgets[3] = v_acc_spin;
    for (auto *w : m_LocationSpoofWidgets)
      w->setEnabled(m_SpoofLocationEnabled);

    auto emit_location = [this, spoof_check, lat_spin, lon_spin, h_acc_spin, v_acc_spin]() {
      m_SpoofLocationEnabled = spoof_check->isChecked();
      m_SpoofLat = lat_spin->value();
      m_SpoofLon = lon_spin->value();
      m_SpoofHAcc = h_acc_spin->value();
      m_SpoofVAcc = v_acc_spin->value();
      m_SaveTimer->start();
      auto *loc = m_Owner->location();
      loc->setSpoofEnabled(m_SpoofLocationEnabled);
      loc->setSpoofLat(m_SpoofLat);
      loc->setSpoofLon(m_SpoofLon);
      loc->setSpoofHAcc(m_SpoofHAcc);
      loc->setSpoofVAcc(m_SpoofVAcc);
      for (auto *w : m_LocationSpoofWidgets)
        w->setEnabled(m_SpoofLocationEnabled);
      emit spoofLocationChanged(
        m_SpoofLocationEnabled, m_SpoofLat, m_SpoofLon, m_SpoofHAcc, m_SpoofVAcc);
    };

    connect(spoof_check, &QCheckBox::stateChanged, [emit_location](int) { emit_location(); });
    connect(lat_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
      [emit_location](double) { emit_location(); });
    connect(lon_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
      [emit_location](double) { emit_location(); });
    connect(h_acc_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
      [emit_location](double) { emit_location(); });
    connect(v_acc_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
      [emit_location](double) { emit_location(); });

    form->addRow(tr("Spoof"), spoof_check);
    form->addRow(tr("Latitude"), lat_spin);
    form->addRow(tr("Longitude"), lon_spin);
    form->addRow(tr("H. Accuracy"), h_acc_spin);
    form->addRow(tr("V. Accuracy"), v_acc_spin);

    page_layout->addWidget(group);
    page_layout->addStretch();
  }

  /* ── LED ────────────────────────────────────────────────────── */
  auto *led_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(led_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *group = new QGroupBox(tr("LED States"));
    m_LedForm = new QFormLayout(group);
    m_LedForm->setVerticalSpacing(8);

    m_LedEmptyLabel = new QLabel(tr("LED states will appear here when the core modifies an LED."));
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

    page_layout->addWidget(group);
    page_layout->addStretch();
  }

  /* ── Microphone ─────────────────────────────────────────────── */
  auto *mic_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(mic_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *group = new QGroupBox(tr("Microphone"));
    auto *form = new QFormLayout(group);
    form->setVerticalSpacing(8);

    auto *device_label = new QLabel(m_Owner->microphone()->deviceName());
    device_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Input Device"), device_label);

    m_MicEmptyLabel = new QLabel(tr("No microphone instances are currently open."));
    m_MicEmptyLabel->setWordWrap(true);
    {
      QFont f = m_MicEmptyLabel->font();
      f.setItalic(true);
      m_MicEmptyLabel->setFont(f);
    }
    m_MicEmptyLabel->setForegroundRole(QPalette::Dark);
    form->addRow(m_MicEmptyLabel);

    int idx = 0;
    for (auto *handle : m_Owner->microphone()->openMics())
    {
      retro_microphone_params_t params{};
      bool active = m_Owner->microphone()->getState(handle);
      unsigned rate = 0;
      m_Owner->microphone()->getParams(handle, &params);
      rate = params.rate;

      auto *state_label = new QLabel(active ? tr("Recording") : tr("Stopped"));
      auto *rate_label = new QLabel(rate ? QString::number(rate) + tr(" Hz") : QStringLiteral("—"));
      m_MicStateLabels[handle] = state_label;
      m_MicRateLabels[handle] = rate_label;
      form->addRow(tr("Instance %1 State").arg(idx + 1), state_label);
      form->addRow(tr("Instance %1 Rate").arg(idx + 1), rate_label);
      ++idx;
    }
    m_MicEmptyLabel->setVisible(m_MicStateLabels.isEmpty());
    m_MicForm = form;

    page_layout->addWidget(group);
    page_layout->addStretch();
  }

  /* ── Core Constants ─────────────────────────────────────────── */
  auto *core_const_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(core_const_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *group = new QGroupBox(tr("Core Constants"));
    auto *form = new QFormLayout(group);
    form->setVerticalSpacing(4);

    auto yes_no = [](bool set, bool v) -> QLabel * {
      return new QLabel(set ? (v ? tr("Yes") : tr("No")) : tr("Unset"));
    };

    auto make_desc = [form](const char *text) {
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
      m_Owner->performanceLevelSet() ? QString::number(m_Owner->performanceLevel()) : tr("Unset"));
    form->addRow(tr("Performance Level"), m_CorePerfLevelLabel);
    make_desc("The relative performance level of the core.\n"
              "RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL (8)");

    auto fmt_pixel = [this]() -> QString {
      if (!m_Owner->pixelFormatSet())
        return tr("Unset");
      switch (m_Owner->retroPixelFormat())
      {
      case RETRO_PIXEL_FORMAT_0RGB1555:
        return QStringLiteral("0RGB1555");
      case RETRO_PIXEL_FORMAT_XRGB8888:
        return QStringLiteral("XRGB8888");
      case RETRO_PIXEL_FORMAT_RGB565:
        return QStringLiteral("RGB565");
      default:
        return QStringLiteral("Invalid (0x%1)").arg(m_Owner->retroPixelFormat(), 0, 16).toUpper();
      }
    };
    m_CorePixelFormatLabel = new QLabel(fmt_pixel());
    form->addRow(tr("Pixel Format"), m_CorePixelFormatLabel);
    make_desc("Pixel format used by the core's framebuffer.\n"
              "RETRO_ENVIRONMENT_SET_PIXEL_FORMAT (10)");

    m_CoreSupportsNoGameLabel = yes_no(m_Owner->supportsNoGameSet(), m_Owner->supportsNoGame());
    form->addRow(tr("Supports No Game"), m_CoreSupportsNoGameLabel);
    make_desc("Whether this core reports to support running without any loaded content.\n"
              "RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME (18)");

    m_CoreAchievementsLabel =
      yes_no(m_Owner->supportsAchievementsSet(), m_Owner->supportsAchievements());
    form->addRow(tr("Supports Achievements"), m_CoreAchievementsLabel);
    make_desc("Whether or not this core reports to support achievements.\n"
              "RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS (42)");

    m_CoreSerializationLabel = new QLabel(
      m_Owner->serializationQuirksSet()
        ? QStringLiteral("0x") + QString::number(m_Owner->serializationQuirks(), 16).toUpper()
        : tr("Unset"));
    form->addRow(tr("Serialization Quirks"), m_CoreSerializationLabel);
    make_desc("Bitmask of quirks affecting save state serialization.\n"
              "RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS (44)");

    page_layout->addWidget(group);

    auto *ra_group = new QGroupBox(tr("RetroArch Hints"));
    auto *ra_form = new QFormLayout(ra_group);
    ra_form->setVerticalSpacing(4);

    auto ra_desc = [ra_form](const char *text) {
      auto *label = new QLabel(text);
      label->setWordWrap(true);
      QFont f = label->font();
      f.setPointSize(qMax(f.pointSize() - 1, 7));
      label->setFont(f);
      label->setForegroundRole(QPalette::Dark);
      label->setContentsMargins(0, 0, 0, 6);
      ra_form->addRow(label);
    };

    m_RaSaveStateInBackgroundLabel =
      yes_no(m_Owner->raSaveStateInBackgroundSet(), m_Owner->raSaveStateInBackground());
    ra_form->addRow(tr("Save State in Background"), m_RaSaveStateInBackgroundLabel);
    ra_desc("Hint whether serialization should be run in a background thread.\n"
            "RETRO_ENVIRONMENT_SET_SAVE_STATE_IN_BACKGROUND (RA 2)");

    m_RaClearAllThreadWaitsLabel =
      new QLabel(m_Owner->raClearAllThreadWaitsRequested() ? tr("Yes") : tr("No"));
    ra_form->addRow(tr("Clear All Thread Waits Requested"), m_RaClearAllThreadWaitsLabel);
    ra_desc("Whether the core has requested the clear-all-thread-waits callback.\n"
            "RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB (RA 3)");

    m_RaPollTypeOverrideLabel =
      new QLabel(m_Owner->raPollTypeOverrideSet() ? QString::number(m_Owner->raPollTypeOverride())
                                                  : tr("Unset"));
    ra_form->addRow(tr("Poll Type Override"), m_RaPollTypeOverrideLabel);
    ra_desc("Hint for which input poll type the frontend should prefer.\n"
            "RETRO_ENVIRONMENT_POLL_TYPE_OVERRIDE (RA 4)");

    m_RaSaveStateDisableUndoLabel =
      yes_no(m_Owner->raSaveStateDisableUndoSet(), m_Owner->raSaveStateDisableUndo());
    ra_form->addRow(tr("Disable Save State Undo"), m_RaSaveStateDisableUndoLabel);
    ra_desc("Hint that the frontend should disable its save/load undo feature.\n"
            "RETRO_ENVIRONMENT_SET_SAVE_STATE_DISABLE_UNDO (RA 5)");

    page_layout->addWidget(ra_group);
    page_layout->addStretch();
  }

  /* ── Memory ─────────────────────────────────────────────────── */
  auto *mem_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(mem_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    auto *group = new QGroupBox(tr("Memory Data"));
    auto *form = new QFormLayout(group);
    form->setVerticalSpacing(8);

    for (int i = 0; i < 4; i++)
    {
      auto *header = new QLabel(k_memTypes[i].name);
      if (i > 0)
        header->setContentsMargins(0, 8, 0, 0);
      form->addRow(header);

      m_MemDataPtrLabel[i] = new QLabel(tr("—"));
      m_MemDataSizeLabel[i] = new QLabel(tr("—"));
      makeSmallGray(m_MemDataPtrLabel[i]);
      makeSmallGray(m_MemDataSizeLabel[i]);

      form->addRow(tr("Data"), m_MemDataPtrLabel[i]);
      form->addRow(tr("Size"), m_MemDataSizeLabel[i]);
    }

    page_layout->addWidget(group);

    auto *maps_group = new QGroupBox(tr("Memory Descriptors"));
    m_MemMapsForm = new QFormLayout(maps_group);
    m_MemMapsForm->setVerticalSpacing(8);
    m_MemMapsShownCount = -1; /* populated on first timer tick */
    page_layout->addWidget(maps_group);

    page_layout->addStretch();
  }

  /* ── Proc Address ───────────────────────────────────────────── */
  auto *proc_page = new QWidget();
  {
    auto *page_layout = new QVBoxLayout(proc_page);
    page_layout->setContentsMargins(12, 12, 12, 12);
    page_layout->setSpacing(12);

    /* Symbol query row */
    auto *input_widget = new QWidget();
    auto *input_hbox = new QHBoxLayout(input_widget);
    input_hbox->setContentsMargins(0, 0, 0, 0);
    input_hbox->setSpacing(6);
    auto *symbol_input = new QLineEdit();
    symbol_input->setPlaceholderText(tr("Symbol name…"));
    auto *query_btn = new QPushButton(tr("Query"));
    input_hbox->addWidget(symbol_input, 1);
    input_hbox->addWidget(query_btn);
    page_layout->addWidget(input_widget);

    /* Button area for resolved functions */
    auto *func_group = new QGroupBox(tr("Found Functions"));
    auto *button_layout = new QVBoxLayout(func_group);
    button_layout->setSpacing(6);

    /* Restore buttons for symbols confirmed in previous queries */
    for (const QString &sym : m_ProcSymbols)
    {
      auto *btn = new QPushButton(sym);
      connect(btn, &QPushButton::clicked, this,
        [this, sym]() { m_Owner->procAddress()->call(sym.toLocal8Bit().constData()); });
      button_layout->addWidget(btn);
    }
    button_layout->addStretch();

    page_layout->addWidget(func_group, 1);

    auto *not_found_label = new QLabel();
    not_found_label->setForegroundRole(QPalette::Highlight);
    not_found_label->hide();
    page_layout->addWidget(not_found_label);

    /* Look up the typed symbol; add a call button if it resolves. */
    auto do_query = [this, symbol_input, button_layout, not_found_label]() {
      const QString sym = symbol_input->text().trimmed();
      if (sym.isEmpty() || m_ProcSymbols.contains(sym))
        return;
      if (m_Owner->procAddress()->get(sym.toLocal8Bit().constData()))
      {
        not_found_label->hide();
        m_ProcSymbols.append(sym);
        auto *btn = new QPushButton(sym);
        connect(btn, &QPushButton::clicked, this,
          [this, sym]() { m_Owner->procAddress()->call(sym.toLocal8Bit().constData()); });
        /* Insert before the trailing stretch */
        QLayoutItem *stretch = button_layout->takeAt(button_layout->count() - 1);
        button_layout->addWidget(btn);
        button_layout->addItem(stretch);
        symbol_input->clear();
      }
      else
      {
        not_found_label->setText(tr("Symbol \"%1\" was not recognized by the core.").arg(sym));
        not_found_label->show();
      }
    };

    connect(query_btn, &QPushButton::clicked, this, [do_query]() { do_query(); });
    connect(symbol_input, &QLineEdit::returnPressed, this, [do_query]() { do_query(); });
  }

  /* ── Logging ────────────────────────────────────────────────── */
  auto *log_edit = new QTextEdit();
  log_edit->setReadOnly(true);
  log_edit->setFont(QFont("monospace"));
  log_edit->setFrameShape(QFrame::NoFrame);
  log_edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  {
    auto level_prefix = [](int level) -> QString {
      switch (level)
      {
      case RETRO_LOG_DEBUG:
        return "[D] ";
      case RETRO_LOG_INFO:
        return "[I] ";
      case RETRO_LOG_WARN:
        return "[W] ";
      case RETRO_LOG_ERROR:
        return "[E] ";
      default:
        return "    ";
      }
    };
    auto level_color = [](int level) -> QString {
      switch (level)
      {
      case RETRO_LOG_DEBUG:
        return "#808080";
      case RETRO_LOG_WARN:
        return "#c87800";
      case RETRO_LOG_ERROR:
        return "#b40000";
      default:
        return "";
      }
    };
    auto add_entry = [log_edit, level_prefix, level_color](int level, const QString &msg) {
      QString color = level_color(level);
      QString text = level_prefix(level) + msg.toHtmlEscaped();
      if (!color.isEmpty())
        log_edit->append(QString("<span style=\"color:%1\">%2</span>").arg(color, text));
      else
        log_edit->append(text);
    };

    for (const auto &entry : m_Owner->log()->entries())
      add_entry(static_cast<int>(entry.level), entry.message);

    connect(m_Owner, &QRetro::onCoreLog, log_edit,
      [add_entry](int level, const QString msg) { add_entry(level, msg); });
  }

  /* ── Messages ───────────────────────────────────────────────── */
  auto *msg_page = new QWidget();
  auto *msg_layout = new QVBoxLayout(msg_page);
  msg_layout->setContentsMargins(0, 0, 0, 0);
  msg_layout->setSpacing(0);

  auto *msg_version_combo = new QComboBox();
  msg_version_combo->addItem(tr("0 – SET_MESSAGE only"), QVariant(0u));
  msg_version_combo->addItem(tr("1 – SET_MESSAGE_EXT"), QVariant(1u));
  msg_version_combo->setCurrentIndex(static_cast<int>(m_Owner->message()->interfaceVersion()));
  connect(msg_version_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [this](int index) { m_Owner->message()->setInterfaceVersion(static_cast<unsigned>(index)); });

  auto *msg_edit = new QTextEdit();
  msg_edit->setReadOnly(true);
  msg_edit->setFont(QFont("monospace"));
  msg_edit->setFrameShape(QFrame::NoFrame);
  msg_edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

  msg_layout->addWidget(msg_version_combo);
  msg_layout->addWidget(msg_edit);

  {
    auto level_color = [](retro_log_level level) -> QString {
      switch (level)
      {
      case RETRO_LOG_DEBUG:
        return "#808080";
      case RETRO_LOG_WARN:
        return "#c87800";
      case RETRO_LOG_ERROR:
        return "#b40000";
      default:
        return "";
      }
    };
    auto add_entry = [msg_edit, level_color](const QRetroMessageEntry &entry) {
      QString color = level_color(entry.level);
      QString text = entry.message.toHtmlEscaped();
      if (!color.isEmpty())
        msg_edit->append(QString("<span style=\"color:%1\">%2</span>").arg(color, text));
      else
        msg_edit->append(text);
    };

    for (const auto &entry : m_Owner->message()->entries())
      add_entry(entry);

    connect(m_Owner->message(), &QRetroMessage::onMessage, msg_edit,
      [add_entry](const QRetroMessageEntry entry) { add_entry(entry); });
  }

  /* ── Core Options ───────────────────────────────────────────── */
  auto *options_widget = m_Owner->options();
  options_widget->update();

  /* ── Search index ────────────────────────────────────────────── */
  struct SearchRow
  {
    QString text; // label text
    QString group; // group box title (empty if none)
    QGroupBox *groupBox = nullptr;
    QWidget *lw = nullptr;
    QWidget *fw = nullptr;
  };

  std::function<void(QLayout *, QList<SearchRow> &, QGroupBox *)> collect_form_rows;
  collect_form_rows = [&collect_form_rows](QLayout *layout, QList<SearchRow> &rows, QGroupBox *gb) {
    if (!layout)
      return;
    if (auto *form = qobject_cast<QFormLayout *>(layout))
    {
      for (int i = 0; i < form->rowCount(); ++i)
      {
        auto *li = form->itemAt(i, QFormLayout::LabelRole);
        auto *fi = form->itemAt(i, QFormLayout::FieldRole);
        QWidget *lw = li ? li->widget() : nullptr;
        QWidget *fw = fi ? fi->widget() : nullptr;
        QString text;
        if (auto *lbl = qobject_cast<QLabel *>(lw))
          text = lbl->text();
        rows.append(SearchRow{ text, gb ? gb->title() : QString(), gb, lw, fw });
      }
    }
    else
    {
      for (int i = 0; i < layout->count(); ++i)
      {
        auto *item = layout->itemAt(i);
        if (item->layout())
        {
          collect_form_rows(item->layout(), rows, gb);
        }
        else if (auto *w = item->widget())
        {
          auto *child = qobject_cast<QGroupBox *>(w);
          collect_form_rows(w->layout(), rows, child ? child : gb);
        }
      }
    }
  };

  // Pages in stack order; non-form pages get an empty row list
  QVector<QList<SearchRow>> page_rows;
  page_rows.append(QList<SearchRow>()); // 0: optionsWidget — dynamic, skip
  auto index_page = [&](QWidget *page) {
    QList<SearchRow> rows;
    collect_form_rows(page->layout(), rows, nullptr);
    page_rows.append(rows);
  };
  index_page(video_page); //  1
  index_page(audio_page); //  2
  index_page(input_page); //  3
  index_page(env_page); //  4
  index_page(dirs_page); //  5
  index_page(disk_page); //  6
  index_page(sensors_page); //  7
  index_page(location_page); //  8
  index_page(led_page); //  9
  index_page(core_const_page); // 10
  index_page(mem_page); // 11
  index_page(proc_page); // 12
  page_rows.append(QList<SearchRow>()); // 13: logEdit
  page_rows.append(QList<SearchRow>()); // 14: msgPage

  /* ── Sidebar ────────────────────────────────────────────────── */
  const QString core_name = m_Owner->options()->coreName();
  const QString core_label =
    core_name.isEmpty() ? tr("Core Options") : tr("%1 Options").arg(core_name);

  auto *sidebar = new QListWidget();
  sidebar->setFrameShape(QFrame::NoFrame);
  sidebar->setStyleSheet("QListWidget::item { padding: 6px 10px; }");

  int stack_idx = 0;
  QListWidgetItem *first_item = nullptr;

  auto add_divider = [&](const char *text) {
    auto *item = new QListWidgetItem(tr(text));
    item->setFlags(Qt::ItemIsEnabled);
    QFont f = item->font();
    f.setPointSize(qMax(f.pointSize() - 2, 7));
    item->setFont(f);
    item->setForeground(sidebar->palette().color(QPalette::Mid));
    sidebar->addItem(item);
  };

  auto add_sidebar_item = [&](const QString &text) {
    auto *item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, stack_idx++);
    sidebar->addItem(item);
    if (!first_item)
      first_item = item;
  };

  add_divider("GENERAL");
  add_sidebar_item(core_label);
  add_sidebar_item(tr("Video"));
  add_sidebar_item(tr("Audio"));
  add_sidebar_item(tr("Input"));
  add_sidebar_item(tr("Environment"));
  add_sidebar_item(tr("Directories"));
  add_sidebar_item(tr("Disk Control"));

  add_divider("ADVANCED");
  add_sidebar_item(tr("Sensors"));
  add_sidebar_item(tr("Location"));
  add_sidebar_item(tr("LED"));
  add_sidebar_item(tr("Microphone"));

  add_divider("DEVELOPER");
  add_sidebar_item(tr("Core Constants"));
  add_sidebar_item(tr("Memory"));
  add_sidebar_item(tr("Proc Address"));
  add_sidebar_item(tr("Logging"));
  add_sidebar_item(tr("Messages"));

  sidebar->setCurrentItem(first_item);

  /* ── Stacked settings area ──────────────────────────────────── */
  auto *stack = new QStackedWidget();
  stack->addWidget(options_widget);
  stack->addWidget(make_scroll_page(video_page));
  stack->addWidget(make_scroll_page(audio_page));
  stack->addWidget(make_scroll_page(input_page));
  stack->addWidget(make_scroll_page(env_page));
  stack->addWidget(make_scroll_page(dirs_page));
  stack->addWidget(make_scroll_page(disk_page));
  stack->addWidget(make_scroll_page(sensors_page));
  stack->addWidget(make_scroll_page(location_page));
  stack->addWidget(make_scroll_page(led_page));
  stack->addWidget(make_scroll_page(mic_page));
  stack->addWidget(make_scroll_page(core_const_page));
  stack->addWidget(make_scroll_page(mem_page));
  stack->addWidget(make_scroll_page(proc_page));
  stack->addWidget(log_edit);
  stack->addWidget(msg_page);

  connect(sidebar, &QListWidget::currentItemChanged,
    [stack](QListWidgetItem *current, QListWidgetItem *) {
      if (!current)
        return;
      QVariant v = current->data(Qt::UserRole);
      if (v.isValid())
        stack->setCurrentIndex(v.toInt());
    });

  /* ── Search bar ─────────────────────────────────────────────── */
  auto *search_bar = new QLineEdit();
  search_bar->setPlaceholderText(tr("Search settings..."));
  search_bar->setClearButtonEnabled(true);
  search_bar->setContentsMargins(6, 4, 6, 4);

  connect(
    search_bar, &QLineEdit::textChanged, sidebar, [sidebar, stack, page_rows](const QString &text) {
      const QString q = text.trimmed().toLower();
      const bool empty = q.isEmpty();

      // Show/hide form rows and determine page visibility
      QVector<bool> page_visible(page_rows.size(), false);
      for (int idx = 0; idx < page_rows.size(); ++idx)
      {
        const auto &rows = page_rows[idx];
        if (rows.isEmpty())
        {
          // Non-form page: visible when search is empty
          page_visible[idx] = empty;
        }
        else
        {
          // First pass: determine which group boxes have a title match
          QSet<QGroupBox *> group_matches;
          if (!empty)
          {
            for (const auto &row : rows)
            {
              if (row.groupBox && row.group.toLower().contains(q))
                group_matches.insert(row.groupBox);
            }
          }

          // Second pass: show/hide rows; a row is visible if it, its group
          // title, or (when empty) anything matches
          QMap<QGroupBox *, bool> group_has_visible;
          bool any_visible = false;
          for (const auto &row : rows)
          {
            bool match = empty || row.text.toLower().contains(q) ||
                         (row.groupBox && group_matches.contains(row.groupBox));
            if (row.lw)
              row.lw->setVisible(match);
            if (row.fw)
              row.fw->setVisible(match);
            if (match)
              any_visible = true;
            if (row.groupBox)
              group_has_visible[row.groupBox] =
                group_has_visible.value(row.groupBox, false) || match;
          }

          // Show/hide group box widgets themselves
          for (auto it = group_has_visible.begin(); it != group_has_visible.end(); ++it)
            it.key()->setVisible(it.value());

          page_visible[idx] = any_visible;
        }
      }

      // Update sidebar page item visibility; non-form pages also match on label
      for (int i = 0; i < sidebar->count(); ++i)
      {
        auto *item = sidebar->item(i);
        QVariant v = item->data(Qt::UserRole);
        if (!v.isValid())
          continue;
        int idx = v.toInt();
        bool visible = (idx < page_visible.size()) && page_visible[idx];
        if (!visible && idx < page_rows.size() && page_rows[idx].isEmpty())
          visible = empty || item->text().toLower().contains(q);
        item->setHidden(!visible);
      }

      // Hide dividers whose entire section is hidden
      for (int i = 0; i < sidebar->count(); ++i)
      {
        auto *item = sidebar->item(i);
        if (item->data(Qt::UserRole).isValid())
          continue;
        bool any_visible = false;
        for (int j = i + 1; j < sidebar->count(); ++j)
        {
          auto *next = sidebar->item(j);
          if (!next->data(Qt::UserRole).isValid())
            break;
          if (!next->isHidden())
          {
            any_visible = true;
            break;
          }
        }
        item->setHidden(!any_visible);
      }

      // If the current selection was hidden, move to first visible item
      auto *current = sidebar->currentItem();
      if (current && current->isHidden())
      {
        for (int i = 0; i < sidebar->count(); ++i)
        {
          auto *item = sidebar->item(i);
          if (!item->isHidden() && item->data(Qt::UserRole).isValid())
          {
            sidebar->setCurrentItem(item);
            break;
          }
        }
      }
    });

  /* ── Splitter (25 % / 75 %) ─────────────────────────────────── */
  auto *splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(sidebar);
  splitter->addWidget(stack);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  splitter->setSizes({ 160, 480 });

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
  outer->addWidget(search_bar);
  outer->addWidget(splitter, 1);
  outer->addWidget(sep);
  outer->addWidget(m_DescLabel);

  setLayout(outer);
  resize(820, 400);
}
