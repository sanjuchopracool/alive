#include "user_ui_setting.h"
#include "config_constants.h"
#include <QDir>

namespace {
constexpr char k_main_window_group[] = "mainwindow";
constexpr char k_geometry[] = "geometry";
constexpr char k_main_window_splitter[] = "main_splitter_state";
// constexpr char k_top_horizontal_splitter_state[] = "top_horizontal_splitter_state";
// constexpr char k_right_panel_splitter_state[] = "right_panel_splitter_state";

constexpr char k_timeline_group[] = "timeline";
constexpr char k_timeline_main_splitter_state[] = "main_splitter_state";
constexpr char k_timeline_header_splitter_state[] = "header_splitter_state";
constexpr char k_top_horizontal_splitter_state[] = "top_horizontal_splitter_state";
constexpr char k_m_timeline_header_show_matte_info[] = "header_show_matte_info";
constexpr char k_right_toolbar_state[] = "right_toolbar_state";

constexpr char k_project_manager_group[] = "project.manager";
constexpr char k_last_opened_file_url[] = "last_opened_file_url";
constexpr char k_last_exported_file_url[] = "last_exported_file_url";

constexpr char k_key_value_group[] = "key_value_group";

const QVariant k_empty_variant;
} // namespace

namespace alive {
namespace {
static UserSettings *the_instance = nullptr;
} // namespace

void UserSettings::init()
{
    if (!the_instance) {
        the_instance = new UserSettings();
    }
}

void UserSettings::deinit()
{
    if (the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

UserSettings *UserSettings::instance()
{
    return the_instance;
}

struct UserSettingsPrivateData
{
    QVariant main_window_splitter_state;
    QVariant timeline_main_splitter_state;
    QVariant timeline_header_splitter_state;
    QVariant top_horizontal_splitter_state;
    QVariant right_toolbar_state;
    QUrl last_opened_file_url;
    QUrl last_exported_path_url;
    bool show_matte_info = false;

    QMap<QString, QVariant> key_value_data;
};

QVariant UserSettings::main_window_geometry()
{
    QVariant result;
    beginGroup(k_main_window_group);
    result = value(k_geometry);
    endGroup();
    return result;
}

void UserSettings::save_main_window_geometry(const QVariant &data)
{
    beginGroup(k_main_window_group);
    setValue(k_geometry, data);
    endGroup();
}

QVariant UserSettings::main_window_splitter_state()
{
    return m_d->main_window_splitter_state;
}

void UserSettings::save_main_window_splitter_state(const QVariant &data)
{
    m_d->main_window_splitter_state = data;
}

QVariant UserSettings::timeline_main_splitter_state()
{
    return m_d->timeline_main_splitter_state;
}

void UserSettings::save_timeline_main_splitter_state(const QVariant &data)
{
    m_d->timeline_main_splitter_state = data;
}

QVariant UserSettings::timeline_header_splitter_state()
{
    return m_d->timeline_header_splitter_state;
}

void UserSettings::save_timeline_header_splitter_state(const QVariant &data)
{
    m_d->timeline_header_splitter_state = data;
}

QVariant UserSettings::top_horizontal_splitter_state()
{
    return m_d->top_horizontal_splitter_state;
}

void UserSettings::save_top_horizontal_splitter_state(
    const QVariant &data)
{
    m_d->top_horizontal_splitter_state = data;
}

bool UserSettings::show_matte_info() const
{
    return m_d->show_matte_info;
}
void UserSettings::set_show_matte_info(bool show)
{
    m_d->show_matte_info = show;
}

void UserSettings::set_last_opened_file_url(const QUrl &url)
{
    if (url != m_d->last_opened_file_url) {
        m_d->last_opened_file_url = url;
    }
}

const QUrl &UserSettings::last_opened_file_url() const
{
    return m_d->last_opened_file_url;
}

QUrl UserSettings::last_opened_file_dir_url() const
{
    if (!m_d->last_opened_file_url.isEmpty()) {
        return QUrl::fromLocalFile(
            QFileInfo(m_d->last_opened_file_url.toLocalFile()).absoluteDir().path());
    } else {
        return QUrl::fromLocalFile(QDir::homePath());
    }
}

QVariant UserSettings::right_toolbar_state()
{
    return m_d->right_toolbar_state;
}

void UserSettings::save_right_toolbar_state(const QVariant &data)
{
    m_d->right_toolbar_state = data;
}

const QVariant &UserSettings::get_data(const QString &key)
{
    auto it = m_d->key_value_data.find(key);
    if (it != m_d->key_value_data.end()) {
        return it.value();
    }
    return k_empty_variant;
}

void UserSettings::set_data(const QString &key, const QVariant &data)
{
    m_d->key_value_data.insert(key, data);
}

UserSettings::UserSettings(QObject *parent)
    : QSettings(QSettings::UserScope, ORGANIZATION_NAME, APP_NAME, parent)
    , m_d(std::make_unique<UserSettingsPrivateData>())
{
    // // right panel splitter state
    beginGroup(k_main_window_group);
    m_d->main_window_splitter_state = value(k_main_window_splitter);
    m_d->top_horizontal_splitter_state = value(k_top_horizontal_splitter_state);
    m_d->right_toolbar_state = value(k_right_toolbar_state);
    endGroup();

    beginGroup(k_timeline_group);
    m_d->timeline_main_splitter_state = value(k_timeline_main_splitter_state);
    m_d->timeline_header_splitter_state = value(k_timeline_header_splitter_state);
    m_d->show_matte_info = value(k_m_timeline_header_show_matte_info).toBool();
    endGroup();

    beginGroup(k_key_value_group);
    for (const auto &key : childKeys()) {
        m_d->key_value_data.insert(key, value(key));
    }
    endGroup();

    beginGroup(k_project_manager_group);
    m_d->last_opened_file_url = value(k_last_opened_file_url).toUrl();
    m_d->last_exported_path_url = value(k_last_exported_file_url).toUrl();
    endGroup();

    if (m_d->last_exported_path_url.isEmpty()) {
        m_d->last_exported_path_url = QUrl::fromLocalFile(QDir::homePath());
    }
}

UserSettings::~UserSettings()
{
    // // right panel splitter state save
    beginGroup(k_main_window_group);
    setValue(k_main_window_splitter, m_d->main_window_splitter_state);
    setValue(k_top_horizontal_splitter_state, m_d->top_horizontal_splitter_state);
    setValue(k_right_toolbar_state, m_d->right_toolbar_state);
    endGroup();

    beginGroup(k_key_value_group);
    for (const auto &[key, value] : m_d->key_value_data.asKeyValueRange()) {
        setValue(key, value);
    }
    endGroup();

    beginGroup(k_timeline_group);
    setValue(k_timeline_main_splitter_state, m_d->timeline_main_splitter_state);
    setValue(k_timeline_header_splitter_state, m_d->timeline_header_splitter_state);
    setValue(k_m_timeline_header_show_matte_info, m_d->show_matte_info);
    endGroup();

    beginGroup(k_project_manager_group);
    setValue(k_last_opened_file_url, m_d->last_opened_file_url);
    setValue(k_last_exported_file_url, m_d->last_exported_path_url);
    endGroup();
}
} // namespace alive
