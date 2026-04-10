#ifndef USER_UI_SETTINGS_H
#define USER_UI_SETTINGS_H

#include <QSettings>
#include <QUrl>

namespace alive {
struct UserSettingsPrivateData;
class UserSettings : public QSettings
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(UserSettings)
public:
    static void init();
    static void deinit();

    static UserSettings *instance();

    QVariant main_window_geometry();
    void save_main_window_geometry(const QVariant &data);

    QVariant main_window_splitter_state();
    void save_main_window_splitter_state(const QVariant &data);

    QVariant timeline_main_splitter_state();
    void save_timeline_main_splitter_state(const QVariant &data);

    QVariant timeline_header_splitter_state();
    void save_timeline_header_splitter_state(const QVariant &data);

    QVariant top_horizontal_splitter_state();
    void save_top_horizontal_splitter_state(const QVariant &data);

    bool show_matte_info() const;
    void set_show_matte_info(bool show);

    void set_last_opened_file_url(const QUrl &url);
    const QUrl &last_opened_file_url() const;
    QUrl last_opened_file_dir_url() const;

    QVariant right_toolbar_state();
    void save_right_toolbar_state(const QVariant &data);

    const QVariant &get_data(const QString &key);
    void set_data(const QString &key, const QVariant &data);
signals:
    void updated();
    void last_opened_file_path_changed();

private:
    explicit UserSettings(QObject *parent = nullptr);
    ~UserSettings();

private:
    std::unique_ptr<UserSettingsPrivateData> m_d;
};
} // namespace alive
#endif // USER_SETTINGS_H
