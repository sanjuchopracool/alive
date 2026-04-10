#include "user_settings.h"
#include "config_constants.h"
#include <QDir>

namespace {
constexpr char k_x[] = "x";
constexpr char k_y[] = "y";
constexpr char k_width[] = "width";
constexpr char k_height[] = "height";

constexpr char k_window_group[] = "window";
constexpr char k_main_window_splitter[] = "main_splitter_state";
constexpr char k_top_horizontal_splitter_state[] = "top_horizontal_splitter_state";
constexpr char k_right_panel_splitter_state[] = "right_panel_splitter_state";

constexpr char k_timeline_group[] = "timeline";
constexpr char k_timeline_main_splitter_state[] = "main_splitter_state";
constexpr char k_timeline_header_splitter_state[] = "header_splitter_state";
constexpr char k_m_timeline_header_show_layer_controls[] = "header_show_layer_control";

constexpr char k_project_manager_group[] = "project.manager";
constexpr char k_last_opened_file_url[] = "last_opened_file_url";
constexpr char k_last_exported_file_url[] = "last_exported_file_url";

} // namespace

namespace alive::settings {
int UserSettings::window_x()
{
    int result = -1;
    beginGroup(k_window_group);
    result = value(k_x).toInt();
    endGroup();
    return result;
}

void UserSettings::set_window_x(int x)
{
    beginGroup(k_window_group);
    setValue(k_x, x);
    endGroup();
}

int UserSettings::window_y()
{
    int result = -1;
    beginGroup(k_window_group);
    result = value(k_y).toInt();
    endGroup();
    return result;
}

void UserSettings::set_window_y(int y)
{
    beginGroup(k_window_group);
    setValue(k_y, y);
    endGroup();
}

int UserSettings::window_width()
{
    int result;
    beginGroup(k_window_group);
    bool ok = false;
    result = value(k_width).toInt(&ok);
    endGroup();
    if (!ok)
        result = -1;
    return result;
}

void UserSettings::set_window_width(int width)
{
    beginGroup(k_window_group);
    setValue(k_width, width);
    endGroup();
}

int UserSettings::window_height()
{
    int result = -1;
    beginGroup(k_window_group);
    result = value(k_height).toInt();
    endGroup();
    return result;
}

void UserSettings::set_window_height(int height)
{
    beginGroup(k_window_group);
    setValue(k_height, height);
    endGroup();
}

void UserSettings::set_last_opened_file_url(const QUrl &url)
{
    if (url != m_last_opened_file_url) {
        m_last_opened_file_url = url;
    }
}

QUrl UserSettings::last_opened_file_dir_url() const
{
    if (!m_last_opened_file_url.isEmpty()) {
        return QUrl::fromLocalFile(
            QFileInfo(m_last_opened_file_url.toLocalFile()).absoluteDir().path());
    } else {
        return QUrl::fromLocalFile(QDir::homePath());
    }
}

void UserSettings::save_last_exported_path(const QUrl &last_path)
{
    m_last_exported_path_url = last_path;
}

UserSettings::UserSettings(QObject *parent)
    : QSettings(QSettings::UserScope, ORGANIZATION_NAME, APP_NAME, parent)
{
    // right panel splitter state
    beginGroup(k_window_group);
    m_main_window_splitter_state = value(k_main_window_splitter);
    m_top_horizontal_splitter_state = value(k_top_horizontal_splitter_state);
    m_right_panel_splitter_state = value(k_right_panel_splitter_state);
    endGroup();

    beginGroup(k_timeline_group);
    m_timeline_main_splitter_state = value(k_timeline_main_splitter_state);
    m_timeline_header_splitter_state = value(k_timeline_header_splitter_state);
    m_timeline_header_show_layer_controls = value(k_m_timeline_header_show_layer_controls).toBool();
    endGroup();

    beginGroup(k_project_manager_group);
    m_last_opened_file_url = value(k_last_opened_file_url).toUrl();
    m_last_exported_path_url = value(k_last_exported_file_url).toUrl();
    endGroup();

    if (m_last_exported_path_url.isEmpty()) {
        m_last_exported_path_url = QUrl::fromLocalFile(QDir::homePath());
    }
}

UserSettings::~UserSettings()
{
    // right panel splitter state save
    beginGroup(k_window_group);
    setValue(k_main_window_splitter, m_main_window_splitter_state);
    setValue(k_top_horizontal_splitter_state, m_top_horizontal_splitter_state);
    setValue(k_right_panel_splitter_state, m_right_panel_splitter_state);
    endGroup();

    beginGroup(k_timeline_group);
    setValue(k_timeline_main_splitter_state, m_timeline_main_splitter_state);
    setValue(k_timeline_header_splitter_state, m_timeline_header_splitter_state);
    setValue(k_m_timeline_header_show_layer_controls, m_timeline_header_show_layer_controls);
    endGroup();

    beginGroup(k_project_manager_group);
    setValue(k_last_opened_file_url, m_last_opened_file_url);
    setValue(k_last_exported_file_url, m_last_exported_path_url);
    endGroup();
}
} // namespace alive::settings
