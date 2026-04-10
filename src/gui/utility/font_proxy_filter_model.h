#ifndef FONTPROXYFILTERMODEL_H
#define FONTPROXYFILTERMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive {

namespace gui {
class FontModel;
}
class FontProxyFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FontProxyFilterModel)
    Q_PROPERTY(int currentIndex READ current_index WRITE set_current_index NOTIFY
                   current_index_changed FINAL)
    Q_PROPERTY(const QStringList &styles READ styles NOTIFY styles_changed FINAL)
    Q_PROPERTY(
        int style_index READ style_index WRITE set_style_index NOTIFY style_index_changed FINAL)

public:
    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    static FontProxyFilterModel *instance();
    static void init(gui::FontModel *source);

    int current_index() const;
    void set_current_index(int new_index);

    const QStringList &styles() const;

    int style_index() const;
    void set_style_index(int new_style_index);

signals:
    void current_index_changed();
    void styles_changed();
    void style_index_changed();

private:
    explicit FontProxyFilterModel(gui::FontModel *source);
    void update_styles_from_source();
    void update_styles_index_from_source();

    gui::FontModel *m_source = nullptr;

    QStringList m_styles;
    QVector<int> m_parent_styles_index;

    int m_index;
    int m_style_index = 0;

    bool m_show_system_fonts = false;
};
} // namespace alive

#endif // FONTPROXYFILTERMODEL_H
