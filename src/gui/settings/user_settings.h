#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#include <gui/theme/canvas_color_setting.h>
#include <QSettings>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::settings {
struct UserSettingsData;
class UserSettings : public QSettings
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(UserSettings)
    Q_PROPERTY(int windowX READ window_x WRITE set_window_x NOTIFY updated FINAL)
    Q_PROPERTY(int windowY READ window_y WRITE set_window_y NOTIFY updated FINAL)
    Q_PROPERTY(int windowWidth READ window_width WRITE set_window_width NOTIFY updated FINAL)
    Q_PROPERTY(int windowHeight READ window_height WRITE set_window_height NOTIFY updated FINAL)

    Q_PROPERTY(QVariant mainWindowSplitterState MEMBER m_main_window_splitter_state)
    Q_PROPERTY(QVariant topHorizontalSplitterState MEMBER m_top_horizontal_splitter_state)
    Q_PROPERTY(QVariant rightPanelSplitterState MEMBER m_right_panel_splitter_state)

    Q_PROPERTY(QVariant timelineMainSplitterState MEMBER m_timeline_main_splitter_state)
    Q_PROPERTY(QVariant timelineHeaderSplitterState MEMBER m_timeline_header_splitter_state)

    Q_PROPERTY(const QUrl &lastOpenedFileUrl READ last_opened_file_url WRITE
                   set_last_opened_file_url NOTIFY updated FINAL)

    Q_PROPERTY(QUrl lastOpenedFileDirUrl READ last_opened_file_dir_url CONSTANT)
    Q_PROPERTY(QUrl lastExportedPathUrl READ last_exported_path CONSTANT)
    Q_PROPERTY(bool timelineHeaderShowControl MEMBER m_timeline_header_show_layer_controls)
public:
    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static UserSettings *instance()
    {
        static UserSettings *the_instance = nullptr;
        if (!the_instance) {
            the_instance = new UserSettings();
        }
        return the_instance;
    }

    int window_x();
    void set_window_x(int x);
    int window_y();
    void set_window_y(int y);

    int window_width();
    void set_window_width(int width);
    int window_height();
    void set_window_height(int height);

    void set_last_opened_file_url(const QUrl &url);
    const QUrl &last_opened_file_url() const { return m_last_opened_file_url; };
    QUrl last_opened_file_dir_url() const;

    void save_last_exported_path(const QUrl &last_path);
    QUrl last_exported_path() const { return m_last_exported_path_url; };

signals:
    void updated();
    void last_opened_file_path_changed();

private:
    explicit UserSettings(QObject *parent = nullptr);
    ~UserSettings();
    QVariant m_main_window_splitter_state;
    QVariant m_top_horizontal_splitter_state;
    QVariant m_right_panel_splitter_state;

    QVariant m_timeline_main_splitter_state;
    QVariant m_timeline_header_splitter_state;
    bool m_timeline_header_show_layer_controls = false;

    QUrl m_last_opened_file_url;
    QUrl m_last_exported_path_url;
};
} // namespace alive::settings
#endif // USER_SETTINGS_H
