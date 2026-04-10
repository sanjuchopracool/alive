#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <memory>
#include <QObject>
#include <QUrl>

#include <core/model/layers/layer.h>
#include <core/serializer/lottie_parser.h>
#include <gui/composition/timeline.h>
#include <gui/quick/range_selector_helper.h>
#include <editor/settings/composition_setting.h>
#include <editor/settings/layer_setting.h>

namespace alive::model {
class CompositionDocument;
class CompositionNode;
class Layer;
}

using LayerPtr = Corrade::Containers::Pointer<alive::model::Layer>;
using Layers = std::vector<LayerPtr>;

namespace alive::script {
class ExpressionEngineInterface;
}

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::project {
struct ProjectManagerData;
class ProjectManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ProjectManager)
public:
    ~ProjectManager();

    // NOLINTBEGIN
    enum DialogCodes { OpenFileFailed, OpenFileInvalid, OpenFileError };
    Q_ENUM(DialogCodes)
    // NOLINTEND

    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static void init();
    static void deinit(bool qml = false);
    static ProjectManager *instance();

    void open_file(const QUrl &url);
    void new_composition();
    void edit_composition_setting();
    void save();
    void save_as(const QUrl &url);
    void close();
    void import(Layers &layers);
    void export_as_image_sequence(const QUrl &url);
    void export_as_webp(QUrl url);
    void toggle_timeline();
    QUrl export_sequnce_path();
    void undo();
    void redo();

    void set_work_area_start();
    void set_work_area_end();

    bool save_action_enabled() const;
    bool close_action_enabled() const;

    const std::string &window_title() const;

    alive::gui::TimeLine *document_timeline(model::CompositionDocument *doc) const;
    void update_expression_engine(model::CompositionDocument *doc,
                                  model::CompositionNode *node) const;
    alive::gui::TimelineHelper *document_timeline_helper(model::CompositionDocument *doc) const;
    model::CompositionDocument *document_for_composition(const model::Composition *comp) const;

    QUuid current_doc_id() const;
    QString current_doc_name() const;

    void select_current_doc_with_id(QUuid uuid);
    void save_and_close_doc_with_id(QUuid uuid);
    void close_doc_with_id(QUuid uuid);
    void exit_app();

    // composition related
    void create_new_composition_from_setting(const alive::CompositionSetting);
    void update_composition_setting(const alive::CompositionSetting);

    // layer related
    void create_new_layer(alive::model::LayerType type);
    void delete_shape_item(QVariant layer_data);

    void apply_layer_settings(model::Layer *layer, const model::LayerSetting &data);
    void invoke_expresson_editor_for_property(model::Property *layer);

    bool is_valid_expression(model::Property *property, const QString &expression);
    std::pair<bool, std::string> is_valid_playground_expression(const QString &expression);
    std::pair<bool, std::string> is_valid_project_expression(const QString &expression);

    bool set_expression_on_property(model::Property *property, const QString &expression);
    model::CompositionDocument *current_document() const;
    gui::TimeLine *current_timline() const;
    void trigger_save_as();

    alive::AliveProjectInformation settings_for_doc(model::CompositionDocument *doc);

public slots:
    void slot_close_event();
    QList<model::CompositionDocument *> close_unmodifed_docs();
    void slot_close_document(model::CompositionDocument *doc);
    void slot_save_document(model::CompositionDocument *doc);
    void invoke_setting_for_layer(model::Layer *layer);
signals:
    void open_file_failed(ProjectManager::DialogCodes, QString message);
    void document_opened(model::CompositionDocument *);
    void window_title_changed(const std::string &title);
    void about_to_close_document(model::CompositionDocument *);
    void current_doc_changed();
    void close_unmodified_doc(model::CompositionDocument *doc);
    void doc_already_opened(QString path, QUuid uuid);
    void show_menu_for_layer(model::Layer *);
    void show_menu_for_property(model::Property *);
    void show_menu_for_shape_item(QVariant);
    void show_setting_for_solid_layer(model::Layer *layer, model::LayerSetting);
    void show_setting_for_file_based_layer(model::Layer *layer, model::LayerSetting);
    void document_name_updated(QUuid, QString);
    void save_new_comp(QString);
    void show_composition_setting_dialog(alive::CompositionSetting);

    void show_expresson_editor_for_property(model::Property *layer, QUuid uuid, QString expression);

private:
    explicit ProjectManager(QObject *parent = nullptr);
    model::CompositionDocument *open_doc_from_comp(model::Composition *comp);

    void set_current_document(model::CompositionDocument *doc);
    void update_window_title();

private:
    std::unique_ptr<ProjectManagerData> m_d;
};
} // namespace alive::project
#endif // PROJECTMANAGER_H
