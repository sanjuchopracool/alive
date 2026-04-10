#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <QApplication>

namespace alive {
struct ApplicationPrivateData;
class Application : public QApplication
{
public:
    Application(int &argc, char **argv, int flags = ApplicationFlags);
    ~Application();

    int execute();

private:
    void init();
    void create_actions();
    void update_recent_files_actions(bool clear = false);
    void add_to_recent_files(const QString &file_path);

private slots:
    void handle_focus_change(QWidget *old, QWidget *now);

private:
    std::unique_ptr<ApplicationPrivateData> m_d;
};
} // namespace alive

#endif // APPLICATION_H
