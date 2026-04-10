#include "new_property_dialog.h"
#include "ui_new_property_dialog.h"
#include <core/model/property/property_type.h>
#include <core/model/property/property_type_information.h>
#include <QPushButton>

namespace alive {
NewPropertyDialog::NewPropertyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewPropertyDialog)
{
    ui->setupUi(this);

    ui->comboBox->addItem(tr("Unknown"));
    for (auto start = model::PropertyType::e_StaticPropertyStart;
         start < model::PropertyType::e_StaticPropertyEnd;) {
        ui->comboBox->addItem(QString::fromStdString(
                                  model::info_from_property_type(start).type_name),
                              static_cast<int>(start));
        start = static_cast<model::PropertyType>(static_cast<int>(start) + 1);
    }

    auto check_if_valid = [this]() {
        ui->buttonBox->button(QDialogButtonBox::Ok)
            ->setEnabled(!ui->le_id->text().isEmpty() && !ui->le_id->text().isEmpty()
                         && (ui->comboBox->currentIndex() != 0));
    };
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, [this, check_if_valid](int index) {
        const auto &info = model::info_from_property_type(
            static_cast<model::PropertyType>(ui->comboBox->currentData().toInt()));
        ui->le_hint->setText(QString::fromStdString(info.hint));
        if (!m_name_edited) {
            QSignalBlocker bkl_name(ui->le_name);
            ui->le_name->setText(QString::fromStdString(info.defaut_name));
        }

        if (!m_id_edited) {
            QSignalBlocker bkl_id(ui->le_id);
            ui->le_id->setText(QString::fromStdString(info.defaut_name).toLower());
        }

        check_if_valid();
    });

    check_if_valid();
    connect(ui->le_id, &QLineEdit::textChanged, this, [this, check_if_valid]() {
        m_id_edited = true;
        check_if_valid();
    });
    connect(ui->le_name, &QLineEdit::textChanged, this, [this, check_if_valid]() {
        m_name_edited = true;
        check_if_valid();
    });
    setWindowTitle(tr("Create New Property"));
}

NewPropertyDialog::~NewPropertyDialog()
{
    delete ui;
}

std::tuple<model::PropertyType, std::string, std::string> NewPropertyDialog::selected_data()
{
    return {static_cast<model::PropertyType>(ui->comboBox->currentData().toInt()),
            ui->le_name->text().toStdString(),
            ui->le_id->text().toStdString()};
}
} // namespace alive
