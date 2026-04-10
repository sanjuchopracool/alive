#ifndef CENTRAL_WIDGET_H
#define CENTRAL_WIDGET_H

#include <memory>
#include <QWidget>

namespace alive {
class CanvasTabWidget;
class TextPropertyWidget;

struct CentralWidgetPrivateData;

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();

    CanvasTabWidget *canvas_tab_widget();
signals:

public:
    void restore_settings();
    void save_settings() const;

private:
    std::unique_ptr<CentralWidgetPrivateData> m_d;
};

} // namespace alive

#endif // CENTRAL_WIDGET_H
