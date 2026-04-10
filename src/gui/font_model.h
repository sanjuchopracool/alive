#ifndef FONTMODEL_H
#define FONTMODEL_H

#include <QAbstractListModel>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::gui {
class FontModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FontModel)
    Q_PROPERTY(
        int index READ current_index WRITE set_current_index NOTIFY current_index_changed FINAL)
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

    enum CustomRole { e_SystemFont = Qt::UserRole + 1, e_DBFont };

    static FontModel *instance()
    {
        static FontModel *the_instance = nullptr;
        if (!the_instance) {
            the_instance = new FontModel();
        }
        return the_instance;
    }

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Q_INVOKABLE int font_famliy_index(QString family) const;
    QString family_at_index(int index) const;
    QString style_at_index(int index) const;
    bool is_system_font(int index, int style_index) const;

    int current_index() const;
    void set_current_index(int new_index);
    void set_current_family_and_style(int new_family, int new_style);

    const QStringList &styles() const;

    int style_index() const;
    void set_style_index(int new_style_index);

signals:
    void current_index_changed();
    void styles_changed();
    void style_index_changed();

private:
    explicit FontModel(QObject *parent = nullptr);
    void update_styles();

    QStringList m_styles;

    int m_index = -1;
    int m_style_index = 0;
};
} // namespace alive::gui

#endif // FONTMODEL_H
