#include "project_manager.h"
#include <Corrade/Containers/Pointer.h>
#include <core/composition/composition_node.h>
#include <core/exporter/exporter.h>
#include <core/model/composition.h>
#include <core/script/expression_engine_interface.h>
#include <core/serializer/editor_support.h>
#include <core/serializer/lottie_parser.h>
#include <gui/color_utility.h>
#include <gui/composition/timeline.h>
#include <gui/document/composition_document_model.h>
#include <editor/undo_support/document_undo_manager.h>
#include <gui/settings/user_settings.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/theme/layer_color_model.h>
#include <gui/utility/gui_utility.h>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMap>
#include <QUrl>
namespace alive::project {

namespace {
static ProjectManager *the_instance = nullptr;
} // namespace

using EExpressionnginePtr = Corrade::Containers::Pointer<script::ExpressionEngineInterface>;
struct DocumentObjects
{
    alive::gui::TimeLine *timeline;
    alive::gui::TimelineHelper *timeline_helper;
    script::ExpressionEngineInterface *comp_node_engine = nullptr;
    EExpressionnginePtr playground_engine;
    EExpressionnginePtr comp_project_engine;
};

struct ProjectManagerData
{
    model::CompositionDocument *current_document = nullptr;
    bool close_event_triggered = false;
    bool enable_save_action = false;
    std::string window_title;
    void insert_doc(model::CompositionDocument *doc)
    {
        if (!docs.contains(doc)) {
            DocumentObjects objects;
            objects.timeline = new alive::gui::TimeLine(doc, doc);
            objects.timeline_helper = new alive::gui::TimelineHelper(objects.timeline, doc);
            docs.emplace(doc, std::move(objects));

            doc->connect(objects.timeline,
                         &gui::TimeLine::selected_frame_changed,
                         doc,
                         &model::CompositionDocument::slot_current_frame_changed);

            doc->connect(objects.timeline, &gui::TimeLine::range_changed, doc, [this, doc]() {
                doc->set_modified(true);
            });

            doc->connect(objects.timeline, &gui::TimeLine::play_range_changed, doc, [this, doc]() {
                doc->set_modified(true);
            });
        }
    }

    gui::TimeLine *document_timeline(model::CompositionDocument *doc) const
    {
        if (doc) {
            auto it = docs.find(doc);
            if (it != docs.end()) {
                return it->second.timeline;
            }
        }
        return nullptr;
    }
    script::ExpressionEngineInterface *expression_engine(model::CompositionDocument *doc) const
    {
        if (doc) {
            auto it = docs.find(doc);
            if (it != docs.end()) {
                return it->second.comp_node_engine;
            }
        }
        return nullptr;
    }

    script::ExpressionEngineInterface *playground_expression_engine(model::CompositionDocument *doc)
    {
        if (doc) {
            auto it = docs.find(doc);
            if (it != docs.end()) {
                return it->second.playground_engine.get();
            }
        }
        return nullptr;
    }

    script::ExpressionEngineInterface *project_expression_engine(model::CompositionDocument *doc)
    {
        if (doc) {
            auto it = docs.find(doc);
            if (it != docs.end()) {
                return it->second.comp_project_engine.get();
            }
        }
        return nullptr;
    }

    void update_expression_engine(model::CompositionDocument *doc,
                                  const model::CompositionNode *node)
    {
        if (doc && docs.contains(doc) && node) {
            Q_ASSERT_X(docs[doc].comp_node_engine == nullptr,
                       Q_FUNC_INFO,
                       "Setting expression engine again!");
            docs[doc].comp_node_engine = node->property_engine();
            model::CompositionNode &ref = *const_cast<model::CompositionNode *>(node);
            docs[doc].playground_engine.reset(
                script::create_engine(ref, script::ExpressionEngineInterface::e_Both));
            docs[doc].comp_project_engine.reset(
                script::create_engine(ref, script::ExpressionEngineInterface::e_Comp));

            // apply precomp properties
            auto *engine = docs[doc].comp_project_engine.get();
            if (engine) {
                doc->composition()->apply_scripts_on_layers(engine);
            }
        }
    }

    gui::TimelineHelper *document_timeline_helper(model::CompositionDocument *doc) const
    {
        if (doc) {
            auto it = docs.find(doc);
            if (it != docs.end()) {
                return it->second.timeline_helper;
            }
        }
        return nullptr;
    }

    QUuid current_doc_id() const
    {
        if (current_document) {
            return current_document->id();
        }
        return {};
    }

    model::CompositionDocument *find_doc(QUuid uuid)
    {
        for (auto i = docs.cbegin(), end = docs.cend(); i != end; ++i) {
            if (i->first->id() == uuid) {
                return i->first;
            }
        }

        return nullptr;
    }
    model::CompositionDocument *find_doc(const model::Composition *comp)
    {
        for (auto i = docs.cbegin(), end = docs.cend(); i != end; ++i) {
            if (i->first->composition() == comp) {
                return i->first;
            }
        }

        return nullptr;
    }

    void remove_and_delete_document(model::CompositionDocument *doc)
    {
        docs.erase(doc);
        delete doc;
    }

    bool any_document_open() const { return docs.size(); }
    std::map<model::CompositionDocument *, DocumentObjects> docs;

    alive::AliveProjectInformation settings_for_doc(model::CompositionDocument *doc)
    {
        alive::AliveProjectInformation setting;
        if (doc) {
            const auto *timeline = document_timeline(doc);
            if (timeline) {
                setting.m_range_start = timeline->range_start();
                setting.m_range_end = timeline->range_end();
                setting.m_play_range_start = timeline->play_start();
                setting.m_play_range_end = timeline->play_end();
                setting.m_current_frame = timeline->current_frame();
                setting.m_playground_script = doc->playground_script();
            }
        }
        return setting;
    }
};

ProjectManager::ProjectManager(QObject *parent)
    : QObject{parent}
    , m_d(std::make_unique<ProjectManagerData>())
{
}

model::CompositionDocument *ProjectManager::open_doc_from_comp(model::Composition *comp)
{
    auto doc = new model::CompositionDocumentModel(comp, this);
    m_d->insert_doc(doc);
    connect(doc,
            &model::CompositionDocument::modified_changed,
            this,
            [&](model::CompositionDocument *doc) {
                if (doc == m_d->current_document) {
                    update_window_title();
                }
            });

    connect(doc,
            &model::CompositionDocument::show_menu_for_layer,
            this,
            [this](model::Layer *layer) { emit show_menu_for_layer(layer); });
    connect(doc,
            &model::CompositionDocument::show_menu_for_property,
            this,
            [this](model::Property *property) { emit show_menu_for_property(property); });
    connect(doc,
            &model::CompositionDocument::show_menu_for_shape_item,
            this,
            [this](model::ShapeItem *item) {
                QVariant data;
                data.setValue(item);
                emit show_menu_for_shape_item(data);
            });

    connect(doc, &model::CompositionDocument::name_changed, this, [this, doc]() {
        emit document_name_updated(doc->id(), doc->name());
    });

    connect(doc, &model::CompositionDocument::apply_composition_script, this, [this, doc]() {
        auto *engine = m_d->playground_expression_engine(doc);
        engine->is_valid_expression(doc->composition()->apply_script());
    });

    return doc;
}

void ProjectManager::slot_save_document(model::CompositionDocument *doc)
{
    if (doc) {
        if (doc->is_modified()) {
            if (doc->saved_path().empty()) {
                emit save_new_comp(doc->name());
            } else {
                auto data = alive::LottieParser::save_alive(doc->composition(),
                                                          m_d->settings_for_doc(doc));
                QFile file(doc->saved_path().c_str());
                if (!file.open(QIODevice::WriteOnly)) {
                    qWarning() << file.errorString();
                    return;
                }
                file.write(data.c_str());
                doc->set_modified(false);
            }
        }
    }
}

void ProjectManager::slot_close_document(model::CompositionDocument *doc)
{
    if (doc) {
        emit about_to_close_document(doc);
        m_d->remove_and_delete_document(doc);
    }

    if (!m_d->any_document_open()) {
        set_current_document(nullptr);
    }
}

void ProjectManager::set_current_document(model::CompositionDocument *doc)
{
    m_d->current_document = doc;
    update_window_title();
    emit current_doc_changed();
}

void ProjectManager::update_window_title()
{
    if (m_d->current_document) {
        m_d->window_title = m_d->current_document->saved_path();
        if (m_d->current_document->is_modified()) {
            m_d->window_title += "*";
        }
        m_d->window_title += " - Alive";
    } else {
        m_d->window_title = "Alive";
    }
    emit window_title_changed(m_d->window_title);
}

ProjectManager::~ProjectManager() {}

void ProjectManager::init()
{
    if (!the_instance) {
        the_instance = new ProjectManager();
    }
}

void ProjectManager::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

ProjectManager *ProjectManager::instance()
{
    return the_instance;
}

void ProjectManager::open_file(const QUrl &url)
{
    QString file_path = url.toLocalFile();
    if (file_path.isEmpty()) {
        return;
    }

    QFileInfo file_info(file_path);
    if (!file_info.isFile()) {
        return;
    }

    QFile file(file_path);
    if (!file.open(QFile::ReadOnly)) {
        emit open_file_failed(OpenFileFailed, file_path);
        return;
    }

    const QByteArray data = file.readAll();
    if (data.isEmpty()) {
        emit open_file_failed(OpenFileInvalid, file_path);
        return;
    }

    if (file.isOpen()) {
        try {
            alive::AliveProjectInformation setting;
            alive::Messages messages;
            auto comp_ptr = alive::LottieParser::parse_alive(data.constData(), setting, messages);
            for (const auto &message : messages)
                qDebug() << message.message;

            if (!comp_ptr->get_uuid().empty()) {
                QUuid uuid = QUuid::fromString(comp_ptr->get_uuid());
                model::CompositionDocument *doc = m_d->find_doc(uuid);
                if (doc) {
                    emit doc_already_opened(file_path, uuid);
                    return;
                }
            }

            // assign random colors to layers
            auto layer_colors = alive::gui::LayerColorModel::instance();
            for (auto &layer : comp_ptr->layers()) {
                if (layer->editor_color_index() == layer_colors->unset_index()) {
                    layer->set_editor_color_index(layer_colors->get_random_color_index());
                }
            }

            auto doc = open_doc_from_comp(comp_ptr.release());
            doc->set_saved_path(file_path.toStdString());
            settings::UserSettings::instance()->set_last_opened_file_url(url);
            alive::UserSettings::instance()->set_last_opened_file_url(url);
            auto timeline = m_d->document_timeline(doc);
            if (timeline) {
                timeline->set_range(setting.m_range_start, setting.m_range_end);
                timeline->set_play_range(setting.m_play_range_start, setting.m_play_range_end);
                timeline->select_frame(setting.m_current_frame);
                doc->set_playground_script(setting.m_playground_script);
            }

            doc->set_modified(false);

            emit document_opened(doc);
            set_current_document(doc);
        } catch (...) {
            emit open_file_failed(OpenFileInvalid, file_path);
        }
    }
}

void ProjectManager::new_composition()
{
    alive::CompositionSetting settings;
    settings.height = 1080;
    settings.width = 1920;
    settings.name = "Composition";
    settings.fps = 60.0f;
    settings.duration = 2;
    emit show_composition_setting_dialog(settings);
}

void ProjectManager::edit_composition_setting()
{
    if (m_d->current_document) {
        emit show_composition_setting_dialog(m_d->current_document->composition_setting());
    }
}

void ProjectManager::save()
{
    model::CompositionDocument *doc = m_d->current_document;
    slot_save_document(doc);
}

void ProjectManager::save_as(const QUrl &url)
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        QString file_path;
        if (url.isLocalFile()) {
            file_path = url.toLocalFile();
        } else {
            file_path = url.toString();
        }
        QFile file(file_path);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << file.errorString();
            return;
        }

        QFileInfo file_info(file_path);
        if (doc->saved_path() != file_path.toStdString()) {
            doc->set_id(QUuid::createUuid());
        }
        doc->set_saved_path(file_path.toStdString());
        qDebug() << doc->name();
        if (doc->composition()->name().empty()) {
            doc->set_name(file_info.baseName());
        }

        auto data = alive::LottieParser::save_alive(doc->composition(), m_d->settings_for_doc(doc));
        file.write(data.c_str());
        doc->set_modified(false);
    }
    if (m_d->close_event_triggered) {
        slot_close_event();
    } else {
        set_current_document(doc);
    }
}

void ProjectManager::close()
{
    if (m_d->current_document) {
        if (m_d->current_document->is_modified()) {
            emit close_unmodified_doc(m_d->current_document);
        } else {
            slot_close_document(m_d->current_document);
        }
    }
}

void ProjectManager::import(Layers &layers)
{
    if (m_d->current_document) {
        m_d->current_document->import_layers(layers);
    }
}

void ProjectManager::export_as_image_sequence(const QUrl &url)
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        exporter::Exporter image_exporter = exporter::Exporter(doc->composition());
        image_exporter.export_as(url.toLocalFile().toStdString());
        settings::UserSettings::instance()->save_last_exported_path(url);
    }
}

void ProjectManager::export_as_webp(QUrl url)
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        exporter::Exporter image_exporter = exporter::Exporter(doc->composition());
        std::string path;
        if (url.isLocalFile()) {
            path = url.toLocalFile().toStdString();
        } else {
            path = url.toString().toStdString();
        }
        if (path.size()) {
            image_exporter.export_as(path, exporter::Exporter::e_WebM);
        }
    }
}

void ProjectManager::toggle_timeline()
{
    if (m_d->current_document) {
        gui::TimeLine *timeline = m_d->document_timeline(m_d->current_document);
        if (timeline->is_running()) {
            timeline->stop();
        } else {
            timeline->resume();
        }
    }
}

QUrl ProjectManager::export_sequnce_path()
{
    return settings::UserSettings::instance()->last_exported_path();
}

void ProjectManager::slot_close_event()
{
    m_d->close_event_triggered = true;
    QList<model::CompositionDocument *> modified_docs;
    QList<model::CompositionDocument *> unmodified_docs;

    for (auto it = m_d->docs.begin(); it != m_d->docs.end(); ++it) {
        model::CompositionDocument *doc = it->first;
        if (doc->is_modified()) {
            modified_docs.emplaceBack(doc);
        } else {
            unmodified_docs.emplaceBack(doc);
        }
    }

    for (auto *doc : unmodified_docs) {
        emit about_to_close_document(doc);
        m_d->remove_and_delete_document(doc);
    }

    if (modified_docs.size()) {
        emit close_unmodified_doc(modified_docs.first());
    } else {
        qDebug() << "Quiting Application";
        qGuiApp->exit();
    }
}

QList<model::CompositionDocument *> ProjectManager::close_unmodifed_docs()
{
    QList<model::CompositionDocument *> modified_docs;
    QList<model::CompositionDocument *> unmodified_docs;

    for (auto it = m_d->docs.begin(); it != m_d->docs.end(); ++it) {
        model::CompositionDocument *doc = it->first;
        if (doc->is_modified()) {
            modified_docs.emplaceBack(doc);
        } else {
            unmodified_docs.emplaceBack(doc);
        }
    }

    for (auto *doc : unmodified_docs) {
        emit about_to_close_document(doc);
        m_d->remove_and_delete_document(doc);
    }
    return modified_docs;
}

void ProjectManager::undo()
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        doc->undo_manager()->undo();
    }
}

void ProjectManager::redo()
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        doc->undo_manager()->redo();
    }
}

void ProjectManager::set_work_area_start()
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        gui::TimeLine *timeline = m_d->document_timeline(m_d->current_document);
        doc->undo_manager()->set_work_area_start(timeline, timeline->current_frame());
    }
}

void ProjectManager::set_work_area_end()
{
    model::CompositionDocument *doc = m_d->current_document;
    if (doc) {
        gui::TimeLine *timeline = m_d->document_timeline(m_d->current_document);
        doc->undo_manager()->set_work_area_end(timeline, timeline->current_frame());
    }
}

bool ProjectManager::save_action_enabled() const
{
    return m_d->current_document ? m_d->current_document->is_modified() : false;
}

bool ProjectManager::close_action_enabled() const
{
    return m_d->current_document != nullptr;
}

const std::string &ProjectManager::window_title() const
{
    return m_d->window_title;
}

gui::TimeLine *ProjectManager::document_timeline(model::CompositionDocument *doc) const
{
    return m_d->document_timeline(doc);
}

void ProjectManager::update_expression_engine(model::CompositionDocument *doc,
                                              model::CompositionNode *node) const
{
    m_d->update_expression_engine(doc, node);
}

gui::TimelineHelper *ProjectManager::document_timeline_helper(model::CompositionDocument *doc) const
{
    return m_d->document_timeline_helper(doc);
}

model::CompositionDocument *ProjectManager::document_for_composition(
    const model::Composition *comp) const
{
    return m_d->find_doc(comp);
}

void ProjectManager::select_current_doc_with_id(QUuid uuid)
{
    if (current_doc_id() != uuid) {
        model::CompositionDocument *doc = m_d->find_doc(uuid);
        if (doc) {
            set_current_document(doc);
        }
    }
}

void ProjectManager::save_and_close_doc_with_id(QUuid uuid)
{
    model::CompositionDocument *doc = m_d->find_doc(uuid);
    if (doc) {
        if (doc->saved_path().empty()) {
            emit save_new_comp(doc->name());
            return;
        }
        slot_save_document(doc);
        slot_close_document(doc);
    }

    if (m_d->close_event_triggered) {
        slot_close_event();
    }
}

void ProjectManager::close_doc_with_id(QUuid uuid)
{
    model::CompositionDocument *doc = m_d->find_doc(uuid);
    if (doc) {
        qDebug() << "Closing" << doc->saved_path();
        slot_close_document(doc);
    }

    if (!m_d->docs.empty()) {
        set_current_document(m_d->docs.begin()->first);
    } else {
        set_current_document(nullptr);
    }

    if (m_d->close_event_triggered) {
        slot_close_event();
    }
}

void ProjectManager::exit_app()
{
    qGuiApp->exit();
}

void ProjectManager::create_new_composition_from_setting(const CompositionSetting setting)
{
    model::Composition *comp = new model::Composition(setting.name,
                                                      "0.0.1",
                                                      setting.width,
                                                      setting.height,
                                                      0,
                                                      setting.fps * setting.duration,
                                                      setting.fps);
    comp->set_canvas_background_color(setting.bg_color);
    auto doc = open_doc_from_comp(comp);
    emit document_opened(doc);
    set_current_document(doc);
}

void ProjectManager::update_composition_setting(const CompositionSetting setting)
{
    if (m_d->current_document && (m_d->current_document == setting.document)) {
        m_d->current_document->undo_manager()->update_composition_setting(setting);
    }
}

void ProjectManager::create_new_layer(alive::model::LayerType type)
{
    if (m_d->current_document) {
        auto data = m_d->current_document->create_and_push_new_layer(type);
        if (data.first) {
            switch (type) {
            case alive::model::LayerType::e_Solid:
                emit show_setting_for_solid_layer(data.first, data.second);
                break;
            default:
                break;
            }
        }
    }
}

void ProjectManager::delete_shape_item(QVariant layer_data)
{
    if (layer_data.isValid() && m_d->current_document) {
        model::ShapeItem *shape_item = layer_data.value<model::ShapeItem *>();
        m_d->current_document->undo_manager()->remove_shape_item(shape_item);
    }
}

void ProjectManager::apply_layer_settings(model::Layer *layer, const model::LayerSetting &data)
{
    if (m_d->current_document) {
        m_d->current_document->apply_layer_setting(layer, data);
    }
}

void ProjectManager::invoke_setting_for_layer(model::Layer *layer)
{
    if (layer && m_d->current_document) {
        model::LayerType type = layer->layer_type();
        model::LayerSetting data = model::layer_setting(layer);
        switch (type) {
        case model::LayerType::e_Solid:
            emit show_setting_for_solid_layer(layer, data);
            break;
        case model::LayerType::e_Image:
        case model::LayerType::e_Precomp:
            emit show_setting_for_file_based_layer(layer, data);
            break;
        default:
            break;
        }
    }
}

void ProjectManager::invoke_expresson_editor_for_property(model::Property *property)
{
    if (m_d->current_document && property) {
        auto timeline = m_d->document_timeline(m_d->current_document);
        emit show_expresson_editor_for_property(property,
                                                m_d->current_document->id(),
                                                QString::fromStdString(property->get_expression(
                                                    timeline->current_frame())));
    }
}

bool ProjectManager::is_valid_expression(model::Property *property, const QString &expression)
{
    bool result = false;
    script::ExpressionEngineInterface *engine = m_d->expression_engine(m_d->current_document);
    if (engine) {
        result = property->is_valid_expression(engine, expression.toStdString());
    }
    return result;
}

std::pair<bool, std::string> ProjectManager::is_valid_playground_expression(const QString &expression)
{
    script::ExpressionEngineInterface *engine = m_d->project_expression_engine(
        m_d->current_document);
    if (engine) {
        return engine->is_valid_expression(expression.toStdString());
    }
    return {false, {}};
}

std::pair<bool, std::string> ProjectManager::is_valid_project_expression(const QString &expression)
{
    script::ExpressionEngineInterface *engine = m_d->playground_expression_engine(
        m_d->current_document);
    if (engine) {
        return engine->is_valid_expression(expression.toStdString());
    }
    return {false, {}};
}

bool ProjectManager::set_expression_on_property(model::Property *property, const QString &expression)
{
    bool result = false;
    if (m_d->current_document) {
        if (expression.isEmpty()) {
            result = true;
        } else {
            result = is_valid_expression(property, expression);
        }

        if (result) {
            m_d->current_document->undo_manager()->set_expression_on_property(property, expression.toStdString());
        }
    }

    return result;
}

model::CompositionDocument *ProjectManager::current_document() const
{
    return m_d->current_document;
}

gui::TimeLine *ProjectManager::current_timline() const
{
    return m_d->document_timeline(m_d->current_document);
}

void ProjectManager::trigger_save_as()
{
    if (m_d->current_document) {
        emit save_new_comp(m_d->current_document->name());
    }
}

AliveProjectInformation ProjectManager::settings_for_doc(model::CompositionDocument *doc)
{
    return m_d->settings_for_doc(doc);
}

QUuid ProjectManager::current_doc_id() const
{
    return m_d->current_doc_id();
}

QString ProjectManager::current_doc_name() const
{
    if (m_d->current_document)
        return m_d->current_document->name();
    return {};
}
} // namespace alive::project
