#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include <vector>

#include "QRetroOptions.h"

static const char* type_name(uint8_t type)
{
  switch (type)
  {
  case QRetroOption::Choice:
    return "Choice";
  case QRetroOption::Bool:
    return "Bool";
  case QRetroOption::Int:
    return "Int";
  case QRetroOption::Float:
    return "Float";
  default:
    return "Unknown";
  }
}

QRetroOptionCategory::QRetroOptionCategory(retro_core_option_v2_category* us,
  retro_core_option_v2_category* local)
{
  m_Title[Language::Default]       = us->desc ? us->desc : "";
  m_Description[Language::Default] = us->info ? us->info : "";

  if (local)
  {
    m_Title[Language::Local]       = local->desc ? local->desc : "";
    m_Description[Language::Local] = local->info ? local->info : "";
  }
}

const char* QRetroOptionCategory::title(Language lang)
{
  if (lang == Language::Local && !m_Title[lang].empty())
    return m_Title[Language::Local].c_str();
  return m_Title[Language::Default].c_str();
}

const char* QRetroOptionCategory::description(Language lang)
{
  if (lang == Language::Local && !m_Description[lang].empty())
    return m_Description[Language::Local].c_str();
  return m_Description[Language::Default].c_str();
}

QRetroOption::QRetroOption(retro_variable *var)
{
  /*
    The friendly name of the option comes before the first ';' in the string
      var->value.
  */
  m_Title[Default] =
    QString(var->value).section(';', 0, 0).toStdString();

  /*
    The possible values specified by the core come after the first ';' and
      are separated by '|'. There is usually a space after the ';'.
    TODO: Would we ever want duplicates or blank strings as options?
  */
  m_PossibleValues[Default] = QString(var->value).section(";", 1).trimmed().split("|");
  m_PossibleValues[Default].removeDuplicates();
  m_PossibleValues[Default].removeAll(QString(""));

  /* The value the option is initialized to is the first one listed. */
  m_DefaultValue = m_PossibleValues[Default][0].toStdString();

  /*
    By reading the possible values, we can try to provide a hint about what
      kind of data the option represents, which can then be used to properly
      edit it in a UI.
    ie. boolean values can be represented by checkboxes, ints by sliders, etc.
  */
  determineType();

  qDebug("%s (%s):\n"
         "    Possible (%u): [%s]\n"
         "    Default: %s\n"
         "    Type = %s",
         m_Title[Default].c_str(),
         var->key,
         m_PossibleValues[Default].count(),
         m_PossibleValues[Default].join(", ").toStdString().c_str(),
         m_DefaultValue.c_str(),
         type_name(m_Type));
}

/* TODO: Support option titles and multilanguage choices */
QRetroOption::QRetroOption(retro_core_option_definition* us,
  retro_core_option_definition* local)
{
  m_Title[Default]       = us->desc ? us->desc : "";
  m_Description[Default] = us->info ? us->info : "";
  if (local)
  {
    m_Title[Local]       = local->desc ? local->desc : "";
    m_Description[Local] = local->info ? local->info : "";
  }

  auto choice = us->values;

  /* If label is not set, use the actual value */
  while (choice->value)
  {
    m_PossibleValues[Default].append(choice->value);
    choice++;
  }

  /* If default value is not set, default to the first value */
  m_DefaultValue = us->default_value ? std::string(us->default_value) :
                                       std::string(us->values[0].value);

  determineType();

  qDebug("%s (%s):\n"
         "    Possible (%u): [%s]\n"
         "    Default: %s\n"
         "    Type = %s",
         m_Title[local ? Language::Local : Language::Default].c_str(),
         us->key,
         m_PossibleValues[Default].count(),
         m_PossibleValues[Default].join(", ").toStdString().c_str(),
         m_DefaultValue.c_str(),
         type_name(m_Type));
}

QRetroOption::QRetroOption(retro_core_option_v2_definition* us,
  retro_core_option_v2_definition* local)
{
  m_Title[Language::Default]           = us->desc              ? us->desc              : "";
  m_TitleCategorized[Language::Default] = us->desc_categorized  ? us->desc_categorized  : "";
  m_Description[Language::Default]      = us->info              ? us->info              : "";
  m_CategoryKey                         = us->category_key      ? us->category_key      : "";
  if (local)
  {
    m_Title[Language::Local]           = local->desc             ? local->desc             : "";
    m_TitleCategorized[Language::Local] = local->desc_categorized ? local->desc_categorized : "";
    m_Description[Language::Local]      = local->info             ? local->info             : "";
  }

  auto *choice = us->values;
  while (choice->value)
  {
    m_PossibleValues[Default].append(choice->value);
    choice++;
  }
  if (local)
  {
    choice = local->values;
    while (choice->value)
    {
      m_PossibleValues[Local].append(choice->value);
      choice++;
    }
  }

  m_DefaultValue = us->default_value ? std::string(us->default_value) : "";

  determineType();

  qDebug("%s (%s):\n"
         "    Possible (%u): [%s]\n"
         "    Default: %s\n"
         "    Type = %s",
         m_Title[local ? Language::Local : Language::Default].c_str(),
         us->key,
         m_PossibleValues[Default].count(),
         m_PossibleValues[Default].join(", ").toStdString().c_str(),
         m_DefaultValue.c_str(),
         type_name(m_Type));
}

bool QRetroOption::determineType()
{
  auto values = m_PossibleValues[Default];

  if (values.isEmpty())
  {
    m_Type = QRetroOption::None;
    return false;
  }
  /* If it has exactly two values matching a known boolean pair (any
   * capitalisation), treat it as boolean and remember the exact strings. */
  else if (values.count() == 2)
  {
    static const char* const pairs[][2] = {
      {"enabled", "disabled"},
      {"on",      "off"},
      {"true",    "false"},
    };
    for (auto &pair : pairs)
    {
      QString t(pair[0]), f(pair[1]);
      QString foundTrue, foundFalse;
      for (auto &v : values)
      {
        if (!v.compare(t, Qt::CaseInsensitive)) foundTrue  = v;
        if (!v.compare(f, Qt::CaseInsensitive)) foundFalse = v;
      }
      if (!foundTrue.isEmpty() && !foundFalse.isEmpty())
      {
        m_BoolTrueValue  = foundTrue.toStdString();
        m_BoolFalseValue = foundFalse.toStdString();
        m_Type = QRetroOption::Bool;
        return true;
      }
    }
  }
  /* Loop through the options to see if they are all a type of number. */
  else
  {
    bool ok_float = true;
    bool ok_int = true;
    bool ok;

    for (int i = 0; i < values.count(); i++)
    {
      if (ok_int)
      {
        values[i].toInt(&ok, 10);
        ok_int &= ok;
      }
      if (ok_float)
      {
        values[i].toFloat(&ok);
        ok_float &= ok;
      }
    }

    /* TODO: Does, say, 2.0 -> 2 and return success on toInt? */
    if (ok_float && !ok_int)
    {
      m_Type = QRetroOption::Float;
      return true;
    }
    else if (ok_int)
    {
      m_Type = QRetroOption::Int;
      return true;
    }
  }

  /* None of the checks succeeded, but there are options to choose from. */
  m_Type = QRetroOption::Choice;
  return false;
}

const char* QRetroOption::title(Language lang)
{
  if (lang == Language::Local && !m_Title[lang].empty())
    return m_Title[Language::Local].c_str();
  else
    return m_Title[Language::Default].c_str();
}

const char* QRetroOption::description(Language lang)
{
  if (lang == Language::Local && !m_Description[lang].empty())
    return m_Description[Language::Local].c_str();
  else
    return m_Description[Language::Default].c_str();
}

bool QRetroOption::setToDefaultValue()
{
  if (m_PossibleValues[Default].isEmpty())
    return false;
  else
  {
    setValue(m_DefaultValue);
    return true;
  }
}

QRetroOptions::QRetroOptions()
{
  m_Filename = QDir::currentPath() + "/config.ini";
}

QRetroOptions::~QRetroOptions()
{
  QSettings settings(m_Filename, QSettings::IniFormat);

  settings.beginGroup(m_CoreName);
  for (auto iter = m_Variables.begin(); iter != m_Variables.end(); iter++)
  {
    if (!iter->first.empty() && iter->second)
    {
      settings.setValue(iter->first.c_str(), iter->second->getValue());
      delete iter->second;
    }
  }
  settings.sync();
}

QRetroOption* QRetroOptions::getOption(const char *key)
{
  return m_Variables[key];
}

const char* QRetroOptions::getOptionValue(const char *key)
{
  auto var = getOption(key);
  return var ? var->getValue() : nullptr;
}

void QRetroOptions::setOptionValue(const char* key, const char* value)
{
  auto var = getOption(key);

  if (var)
  {
    var->setValue(value);
    m_VariablesUpdated = true;

    /* Let the core refresh option visibility now that a value changed */
    if (m_UpdateDisplayCallback)
      m_UpdateDisplayCallback();
  }
}

void QRetroOptions::setOptions(retro_variable *vars)
{
  QSettings settings(m_Filename, QSettings::IniFormat);

  settings.beginGroup(m_CoreName);
  while (vars && vars->key && vars->value)
  {
    std::string value;
    auto entry = new QRetroOption(vars);

    value = settings.value(vars->key, "").toString().toStdString();
    if (value.empty())
    {
      entry->setToDefaultValue();
      settings.setValue(vars->key, entry->getValue());
    }
    else
      entry->setValue(value);

    m_Variables[vars->key] = entry;
    vars++;
  }
  settings.sync();
  m_Version = v0;
}

void QRetroOptions::setOptions(retro_core_option_definition** vars)
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  auto var = *vars;

  settings.beginGroup(m_CoreName);
  while (var && var->key)
  {
    std::string value;
    auto entry = new QRetroOption(var);

    value = settings.value(var->key, "").toString().toStdString();
    if (value.empty())
    {
      entry->setToDefaultValue();
      settings.setValue(var->key, entry->getValue());
    }
    else
      entry->setValue(value);

    m_Variables[var->key] = entry;
    var++;
  }
  settings.sync();
  m_Version = v1;
}

void QRetroOptions::setOptions(retro_core_options_intl* vars)
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  unsigned i = 0;

  if (!vars->local || vars->us == vars->local)
  {
    setOptions(&vars->us);
    return;
  }

  settings.beginGroup(m_CoreName);
  while (vars->us[i].key)
  {
    std::string value;

    /*
     * Find the local option definition with a matching key to the us one.
     * In practice, these should always align, so this may not be necessary.
     */
    retro_core_option_definition *local = nullptr;
    unsigned j = 0;
    do
    {
      local = &vars->local[j];
      if (!strcmp(local->key, vars->us[i].key))
        break;
      j++;
    } while (local->key);
    if (!local->key)
      local = nullptr;

    auto entry = new QRetroOption(&vars->us[i], &vars->local[i]);

    value = settings.value(vars->us[i].key, "").toString().toStdString();
    if (value.empty())
    {
      entry->setToDefaultValue();
      settings.setValue(vars->us[i].key, entry->getValue());
    }
    else
      entry->setValue(value);

    m_Variables[vars->us[i].key] = entry;
    i++;
  }
  settings.sync();
  m_Version = v1;
}

void QRetroOptions::setOptions(retro_core_options_v2* vars,
                               retro_core_options_v2* local)
{
  QSettings settings(m_Filename, QSettings::IniFormat);

  /* Rebuild category map */
  for (auto &pair : m_Categories)
    delete pair.second;
  m_Categories.clear();

  if (vars->categories)
  {
    auto cat = vars->categories;
    retro_core_option_v2_category *lcat = local ? local->categories : nullptr;
    while (cat->key)
    {
      /* Find matching local category by key */
      retro_core_option_v2_category *match = nullptr;
      if (lcat)
      {
        for (auto *lc = lcat; lc->key; lc++)
          if (!strcmp(lc->key, cat->key)) { match = lc; break; }
      }
      m_Categories.push_back({cat->key, new QRetroOptionCategory(cat, match)});
      cat++;
    }
  }

  /* Build option map */
  auto *var  = vars->definitions;
  auto *lvar = local ? local->definitions : nullptr;

  settings.beginGroup(m_CoreName);
  while (var && var->key)
  {
    /* Find matching local definition by key */
    retro_core_option_v2_definition *lmatch = nullptr;
    if (lvar)
    {
      for (auto *lv = lvar; lv->key; lv++)
        if (!strcmp(lv->key, var->key)) { lmatch = lv; break; }
    }

    auto entry = new QRetroOption(var, lmatch);
    std::string value = settings.value(var->key, "").toString().toStdString();
    if (value.empty())
    {
      entry->setToDefaultValue();
      settings.setValue(var->key, entry->getValue());
    }
    else
      entry->setValue(value);

    m_Variables[var->key] = entry;
    var++;
  }
  settings.sync();
  m_Version = v2;
}

void QRetroOptions::setOptions(retro_core_options_v2_intl* vars)
{
  if (!vars || !vars->us)
    return;

  /* Use localized definitions when available, fall back to US otherwise */
  retro_core_options_v2 *local = (vars->local && vars->local != vars->us)
                                 ? vars->local : nullptr;
  setOptions(vars->us, local);
}

void QRetroOptions::setVisibility(const char *key, bool enabled)
{
  auto var = getOption(key);

  if (var)
    var->setVisibility(enabled);
}

void QRetroOptions::onOptionBoolChanged(int state)
{
  setOptionValue(sender()->objectName().toStdString().c_str(),
                 state == Qt::Unchecked ? "disabled" : "enabled");
}

void QRetroOptions::onOptionChoiceChanged(const QString& choice)
{
  setOptionValue(sender()->objectName().toStdString().c_str(),
                 choice.toStdString().c_str());
}

void QRetroOptions::update()
{
  /* Let the core push any pending SET_CORE_OPTIONS_DISPLAY calls before
   * we build the widget list, so visibility is correct from the start. */
  if (m_UpdateDisplayCallback)
    m_UpdateDisplayCallback();

  /* Delete everything if the layout has already been initialized before. */
  if (this->layout())
  {
    QLayoutItem* item;
    while ((item = this->layout()->takeAt(0)) != nullptr)
    {
      delete item->widget();
      delete item;
    }
    delete this->layout();
  }

  /* Content widget lives inside the scroll area */
  auto *content = new QWidget();
  auto *vbox    = new QVBoxLayout(content);

  using OptionList = std::vector<std::pair<std::string, QRetroOption*>>;
  std::vector<QGroupBox*> allSections;

  /* Adds a single option row (label + control, plus optional description).
   * Returns the number of grid rows consumed (1 or 2). */
  auto addRow = [this](QWidget *parent, QGridLayout *grid, int row,
                        const std::string &key, QRetroOption *var) -> int
  {
    switch (var->type())
    {
    case QRetroOption::Bool:
    {
      std::string trueVal  = var->boolTrueValue();
      std::string falseVal = var->boolFalseValue();
      auto *elem = new QCheckBox(parent);
      elem->setChecked(strcmp(var->getValue(), falseVal.c_str()) != 0);
      connect(elem, &QCheckBox::stateChanged,
              [this, key, trueVal, falseVal](int state) {
        setOptionValue(key.c_str(),
                       state == Qt::Unchecked ? falseVal.c_str() : trueVal.c_str());
      });
      auto *label = new QLabel(var->title(), parent);
      label->setEnabled(var->getVisibility());
      label->setBuddy(elem);
      grid->addWidget(label, row, 0);
      grid->addWidget(elem,  row, 1);
      break;
    }
    default:
    {
      auto *elem = new QComboBox(parent);
      elem->addItems(var->possibleValues());
      elem->setCurrentText(var->getValue());
      elem->setObjectName(QString::fromStdString(key));
      connect(elem, SIGNAL(currentTextChanged(const QString&)),
              this, SLOT(onOptionChoiceChanged(const QString&)));
      auto *label = new QLabel(var->title(), parent);
      label->setEnabled(var->getVisibility());
      label->setBuddy(elem);
      grid->addWidget(label, row, 0);
      grid->addWidget(elem,  row, 1);
      break;
    }
    }

    const char *desc = var->description();
    if (desc && *desc)
    {
      auto *descLabel = new QLabel(desc, parent);
      QFont f = descLabel->font();
      f.setPointSize(qMax(f.pointSize() - 2, 7));
      descLabel->setFont(f);
      descLabel->setEnabled(var->getVisibility());
      descLabel->setWordWrap(true);
      descLabel->setForegroundRole(QPalette::Dark);
      grid->addWidget(descLabel, row + 1, 0, 1, 1);
      return 2;
    }
    return 1;
  };

  /* Returns true if m_Categories contains a category with the given key. */
  auto hasCategory = [&](const std::string &key) {
    for (auto &p : m_Categories)
      if (p.first == key) return true;
    return false;
  };

  /* Builds a collapsible section from a list of options. */
  auto makeSection = [&](const QString &title, const OptionList &opts)
  {
    if (opts.empty())
      return;

    /* Gray the header when every option in the section is invisible. */
    bool anyVisible = false;
    for (auto &p : opts)
      if (p.second && p.second->getVisibility()) { anyVisible = true; break; }

    auto *btn = new QPushButton("\342\226\266 " + title, content);
    btn->setCheckable(true);
    btn->setFlat(true);
    btn->setStyleSheet(
      "QPushButton { text-align: left; font-weight: bold; padding: 4px; border: none; }"
      "QPushButton:hover { background: palette(midlight); }");

    if (!anyVisible)
    {
      QPalette pal = btn->palette();
      pal.setColor(QPalette::ButtonText,
        QApplication::palette().color(QPalette::Disabled, QPalette::ButtonText));
      btn->setPalette(pal);
    }

    auto *section = new QGroupBox(content);
    allSections.push_back(section);
    section->setVisible(false);
    auto *grid    = new QGridLayout(section);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);
    int row = 0;
    for (auto &p : opts)
      if (p.second)
        row += addRow(section, grid, row, p.first, p.second);

    connect(btn, &QPushButton::toggled, [btn, section, title](bool open) {
      section->setVisible(open);
      btn->setText((open ? "\342\226\274 " : "\342\226\266 ") + title);
    });

    vbox->addWidget(btn);
    vbox->addWidget(section);
  };

  if (!m_Categories.empty())
  {
    /* Partition variables into per-category lists and an uncategorised list. */
    std::map<std::string, OptionList> byCategory;
    OptionList uncategorized;

    for (auto &p : m_Variables)
    {
      if (!p.second)
        continue;
      const std::string &catKey = p.second->categoryKey();
      if (!catKey.empty() && hasCategory(catKey))
        byCategory[catKey].push_back(p);
      else
        uncategorized.push_back(p);
    }

    /* Emit categories in the order they appear in m_Categories. */
    for (auto &catPair : m_Categories)
      makeSection(QString(catPair.second->title(Language::Local)),
                  byCategory[catPair.first]);

    makeSection(tr("Other"), uncategorized);
  }
  else
  {
    /* No categories — flat list. */
    auto *section = new QWidget(content);
    auto *grid    = new QGridLayout(section);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);
    int row = 0;
    for (auto &p : m_Variables)
      if (p.second)
        row += addRow(section, grid, row, p.first, p.second);
    vbox->addWidget(section);
  }

  vbox->addStretch();

  auto *scrollArea = new QScrollArea();
  scrollArea->setWidget(content);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto *outer = new QVBoxLayout();
  outer->setContentsMargins(0, 0, 0, 0);
  outer->addWidget(scrollArea);

  setWindowTitle(QString("%1 Options (v%2)").arg(m_CoreName).arg(m_Version));
  setLayout(outer);

  /* Temporarily expand all sections so the layout can compute the natural
   * content width (sections start collapsed, so their width would otherwise
   * be excluded from the size hint). */
  for (auto *s : allSections) s->setVisible(true);
  content->layout()->activate();
  int fitWidth = content->sizeHint().width()
               + style()->pixelMetric(QStyle::PM_ScrollBarExtent);
  for (auto *s : allSections) s->setVisible(false);

  resize(fitWidth, 480);
}

bool QRetroOptions::variablesUpdated(bool quiet)
{
  bool updated = m_VariablesUpdated;

  if (!quiet)
    m_VariablesUpdated = false;

  return updated;
}
