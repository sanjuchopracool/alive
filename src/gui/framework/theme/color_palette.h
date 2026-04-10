#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <memory>
#include <QAbstractListModel>
#include <QColor>
#include <QObject>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::theme {
struct ColorPaletteData;
class ColorPalette : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ColorPalette)
    Q_PROPERTY(QColor primary_background_color READ primary_background_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor secondary_background_color READ secondary_background_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor button_color READ button_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor link_color READ link_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor stroke_color READ stroke_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor primary_font_color READ primary_font_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor secondary_font_color READ secondary_font_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor bright_color READ bright_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor dark_color READ dark_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor accent_color READ accent_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor transparent_accent_color READ transparent_accent_color NOTIFY updated FINAL)
    Q_PROPERTY(QColor playCursorColor READ play_cursor_color NOTIFY updated FINAL)
public:
    enum Color {
        e_BackgroundPrimary,
        e_BackgroundSecondary,
        e_PopupBackground,
        e_TextField,
        e_Button,
        e_FontPrimary,
        e_FontSecondary,
        e_Bright,
        e_Dark,
        e_Stroke,
        e_Link,
        e_Accent,
        e_PlayCursorColor,
        e_Last
    };

    ~ColorPalette();

    const QColor &get_color(Color index) const;
    const QColor &primary_background_color() const;
    const QColor &secondary_background_color() const;
    const QColor &link_color() const;
    const QColor &stroke_color() const;
    const QColor &button_color() const;
    const QColor &popup_color() const;
    const QColor &play_cursor_color() const;

    const QColor &primary_font_color() const;
    const QColor &secondary_font_color() const;
    const QColor &bright_color() const;
    const QColor &dark_color() const;
    const QColor &accent_color() const;
    const QColor transparent_accent_color() const;

    // QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static void init();
    static void deinit(bool qml = false);
    static ColorPalette *instance();

signals:
    void updated();

private:
    explicit ColorPalette(QObject *parent = nullptr);

private:
    std::unique_ptr<ColorPaletteData> m_d;
};
} // namespace alive::theme

#endif // COLORPALETTE_H
