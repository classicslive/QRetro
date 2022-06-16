#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

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
  m_Title[Language::Default] = us->desc;
  m_Description[Language::Default] = us->info;

  if (local)
  {
    m_Title[Language::Local] = local->desc;
    m_Description[Language::Local] = local->info;
  }
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
  m_Title[Default] = QString(us->desc).toStdString();
  if (local)
    m_Title[Local] = QString(local->desc).toStdString();

  auto *choice = us->values;
  while (choice->label && choice->value)
  {
    m_PossibleValues[Default].append(choice->value);
    choice++;
  }

  m_DefaultValue = std::string(us->default_value);

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
  m_Title[Language::Default] = QString(us->desc).toStdString();
  m_TitleCategorized[Language::Default] = QString(us->desc_categorized).toStdString();
  if (local)
  {
    m_Title[Language::Local] = QString(local->desc).toStdString();
    m_TitleCategorized[Language::Local] = QString(local->desc_categorized).toStdString();
  }

  auto *choice = us->values;
  while (choice->label && choice->value)
  {
    m_PossibleValues[Default].append(choice->value);
    choice++;
  }
  if (local)
  {
    choice = local->values;
    while (choice->label && choice->value)
    {
      m_PossibleValues[Local].append(choice->value);
      choice++;
    }
  }

  m_DefaultValue = std::string(us->default_value);

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
  /* If it only allows two choices (enabled and disabled), it is boolean. */
  else if (values.contains("enabled") &&
           values.contains("disabled") &&
           values.count() == 2)
  {
    m_Type = QRetroOption::Bool;
    return true;
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

void QRetroOptions::setOptions(retro_core_options_v2* vars)
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  auto var = vars->definitions;

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
  m_Version = v2;
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

void QRetroOptions::onOptionIntChanged(int state)
{
  auto option = getOption(sender()->objectName().toStdString().c_str());
  int value = state;

  if (!option->possibleValues().contains(QString(state)))
  {
    for (auto i = option->possibleValues().begin(); i != option->possibleValues().end(); i++)
    {
      if (i->toInt() > state)
      {
        i--;
        value = i->toInt();
      }
    }
  }
  option->setValue(QString(state).toStdString());
}

void QRetroOptions::update()
{
  QGridLayout *layout = new QGridLayout();
  int i = 0;

  /* Delete everything if the layout has already been initialized before */
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

  for (auto iter = m_Variables.begin(); iter != m_Variables.end(); iter++, i++)
  {
    auto var = iter->second;

    if (!var || !var->getVisibility())
      continue;
    else switch (var->type())
    {

    case QRetroOption::Bool:
    {
      auto elem = new QCheckBox(this);
      elem->setChecked(strcmp(var->getValue(), "disabled"));
      elem->setObjectName(QString::fromStdString(iter->first));
      connect(elem, SIGNAL(stateChanged(int)),
              this, SLOT(onOptionBoolChanged(int)));

      auto label = new QLabel(var->title(), this);
      label->setBuddy(elem);

      layout->addWidget(label, i, 0);
      layout->addWidget(elem, i, 1);

      break;
    }

    case QRetroOption::Int:
    {
      auto elem = new QSlider(this);
      var->possibleValues().sort();
      elem->setObjectName(QString::fromStdString(iter->first));
      elem->setMinimum(var->possibleValues().front().toInt());
      elem->setMaximum(var->possibleValues().back().toInt());
      elem->setValue(QString(var->getValue()).toInt());
      connect(elem, SIGNAL(valueChanged()),
              this, SLOT(onOptionIntChanged(int)));

      auto label = new QLabel(var->title(), this);
      label->setBuddy(elem);

      layout->addWidget(label, i, 0);
      layout->addWidget(elem, i, 1);

      break;
    }

    default:
    {
      auto elem = new QComboBox(this);
      elem->addItems(var->possibleValues());
      elem->setCurrentText(var->getValue());
      elem->setObjectName(QString::fromStdString(iter->first));
      connect(elem, SIGNAL(currentTextChanged(const QString&)),
              this, SLOT(onOptionChoiceChanged(const QString&)));

      auto label = new QLabel(var->title(), this);
      label->setBuddy(elem);

      layout->addWidget(label, i, 0);
      layout->addWidget(elem, i, 1);
    }
    }
  }
  auto label = new QLabel(QString("Using core options v%1.").arg(QString::number(m_Version)), this);
  layout->addWidget(label, i, 1);

  setWindowTitle(QString("%1 Options").arg(m_CoreName));
  setLayout(layout);
}

bool QRetroOptions::variablesUpdated(bool quiet)
{
  bool updated = m_VariablesUpdated;

  if (!quiet)
    m_VariablesUpdated = false;

  return updated;
}
