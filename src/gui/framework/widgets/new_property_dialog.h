#ifndef NEW_PROPERTY_DIALOG_H
#define NEW_PROPERTY_DIALOG_H

#include <core/model/property/property_type.h>
#include <tuple>
#include <QDialog>

namespace Ui {
class NewPropertyDialog;
}

namespace alive {
class NewPropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPropertyDialog(QWidget *parent = nullptr);
    ~NewPropertyDialog();

    // type, name, id
    std::tuple<model::PropertyType, std::string, std::string> selected_data();

private:
    Ui::NewPropertyDialog *ui;
    bool m_name_edited = false;
    bool m_id_edited = false;
};
} // namespace alive
#endif // NEW_PROPERTY_DIALOG_H
