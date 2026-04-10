#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <core/exporter/exporter.h>
#include <memory>
#include <QMainWindow>
#include <editor/settings/layer_setting.h>

namespace alive::model {
    class CompositionDocument;
    class Layer;
}
namespace alive {
class CanvasTabWidget;
class ActionContextItem;
class CompositionSetting;

struct MainWindowPrivateData;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    CanvasTabWidget *canvas_tab_widget();

public slots:
    void slot_export_as(alive::exporter::Exporter::Format format);
    void slot_import();
    void update_recent_files_menu(ActionContextItem *item);

protected:
    void closeEvent(QCloseEvent *ev) override;

private slots:
    int slot_close_modified_doc(alive::model::CompositionDocument* doc);
    int slot_show_comp_setting_dialog(CompositionSetting setting);
    int slot_show_solid_setting_dialog(alive::model::Layer *layer,
                                       const alive::model::LayerSetting &data);
    int slot_show_setting_for_file_based_layer_dialog(model::Layer *layer, const model::LayerSetting &data);
    void slot_show_save_comp_dialog(const QString &name);

private:
    void restore_settings();
    void save_settings();

private:
    std::unique_ptr<MainWindowPrivateData> m_d;
};

} // namespace alive
#endif // MAIN_WINDOW_H
