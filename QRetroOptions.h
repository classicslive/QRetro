#ifndef QRETRO_SETTINGS_H
#define QRETRO_SETTINGS_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <map>

#include "libretro.h"

enum Language
{
  /// The index of a string with default language, defined as American English.
  /// Used as a fallback if a localized string does not exist.
  Default = 0,

  /// The index of a localized string, determined via the frontend's reported
  /// language. If this index leads to a blank string, the default language
  /// string should be used instead.
  Local,

  Language_Size
};

class QRetroOptionCategory
{
public:
  QRetroOptionCategory(retro_core_option_v2_category* us,
    retro_core_option_v2_category* local = nullptr);

  const char* title(Language lang = Default);
  const char* description(Language lang = Default);

private:
  std::string m_Title[Language_Size];
  std::string m_Description[Language_Size];
};

class QRetroOption
{
public:
  enum Type
  {
    /**
     * The key exists, but has no defined values.
     **/
    None = 0,

    /**
     * The key is representitive of a choice between multiple defined string
     * values.
     **/
    Choice,

    /**
     * The key is representitive of a bool. There should be two values:
     * "enabled" and "disabled".
     **/
    Bool,

    /**
     * The key is representitive of an integer. All values should be strings
     * that represent integers.
     **/
    Int,

    /**
     * The key is representitive of a float or double. All values should be
     * strings that represent floating-point values.
     **/
    Float,

    /**
     * The key is representitive of an unknown data type.
     **/
    Type_Size
  };

  QRetroOption(retro_variable* var);
  QRetroOption(retro_core_option_definition* us,
    retro_core_option_definition* local = nullptr);
  QRetroOption(retro_core_option_v2_definition* us,
    retro_core_option_v2_definition* local = nullptr);

  const char* title(Language lang = Local);

  const char* getValue() { return m_CurrentValue.c_str(); }
  void setValue(std::string val) { m_CurrentValue = val; }

  bool getVisibility() { return m_Visible; }
  void setVisibility(bool enabled) { m_Visible = enabled; }

  bool setToDefaultValue();

  QStringList possibleValues() { return m_PossibleValues[Default]; }
  Type type() { return m_Type; }

private:
  bool determineType();

  const QRetroOptionCategory* m_Category;
  std::string m_Title[Language_Size];
  std::string m_TitleCategorized[Language_Size];
  Type m_Type = None;
  bool m_Visible = true;

  std::string m_CurrentValue;
  std::string m_DefaultValue;
  QStringList m_PossibleValues[Language_Size];
};

class QRetroOptions : public QWidget
{
Q_OBJECT

public:
  enum Version
  {
    /// Legacy format used by RETRO_ENVIRONMENT_SET_VARIABLES.
    v0 = 0,

    /// Format used by RETRO_ENVIRONMENT_SET_CORE_OPTIONS.
    v1,

    /// Format used by RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2.
    v2
  };

  QRetroOptions();
  ~QRetroOptions();

  QRetroOption* getOption(const char* key);
  const char* getOptionValue(const char* key);

  /**
   * Sets the name of the core these options belong to. When writing options
   * to a file, this will be used as a group header to separate the options
   * of different cores.
   */
  void setCoreName(const char *name) { m_CoreName = QString(name); }

  /**
   * Sets the filename that core options will be written to.
   */
  void setFilename(const QString& filename) { m_Filename = filename; }

  /**
   * Sets the maximum core options API version the frontend reports to support.
   * Used for RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION.
   */
  void setMaxVersion(Version version) { m_MaxVersion = version; }

  /**
   * Sets the current value of an option with a given key.
   */
  void setOptionValue(const char* key, const char* value);

  /**
   * Initializes core options using the legacy API format 0.
   * Used for RETRO_ENVIRONMENT_SET_VARIABLES.
   */
  void setOptions(retro_variable* vars);

  /**
   * Initializes core options using the API format 1.
   * Used for RETRO_ENVIRONMENT_SET_CORE_OPTIONS.
   */
  void setOptions(retro_core_option_definition** vars);

  /**
   * Initializes core options using the API format 1, with localization.
   * Used for RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL.
   */
  void setOptions(retro_core_options_intl* vars);

  /**
   * Initializes core options using the API format 2.
   * Used for RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2.
   */
  void setOptions(retro_core_options_v2* vars);

  /**
   * Initializes core options using the API format 2, with localization.
   * Used for RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL.
   */
  void setOptions(retro_core_options_v2_intl* vars);

  /**
   * Toggles the visibility of an option with a given key.
   * Used for RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY.
   */
  void setVisibility(const char* key, bool enabled);

  /**
   * Returns whether or not option values have been updated. Will automatically
   * reset to false if "quiet" is not true.
   * Used for RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE.
   */
  bool variablesUpdated(bool quiet = false);

  /**
   * Returns the greatest supported core options API version.
   * Used for RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION.
   */
  Version maxVersion() { return m_MaxVersion; }

  void update();

public slots:
  void onOptionBoolChanged(int state);
  void onOptionChoiceChanged(const QString&);

private:
  std::map<std::string, QRetroOptionCategory*> m_Categories;
  QString m_CoreName;
  QString m_Filename;
  Version m_MaxVersion = Version::v1;
  std::map<std::string, QRetroOption*> m_Variables;
  Version m_Version;
  bool m_VariablesUpdated;
};

#endif
