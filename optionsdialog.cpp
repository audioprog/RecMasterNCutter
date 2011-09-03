#include <QSettings>
#include <QStringList>

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    QSettings settings;
    QStringList lst = settings.value("PathList", QStringList()).toStringList();
    ui->lstwPath->addItems(lst);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_buttonBox_accepted()
{
    QList<QListWidgetItem*> lstw = ui->lstwPath->findItems("", Qt::MatchStartsWith);
    QStringList lst;
    foreach (QListWidgetItem* itm, lstw)
        if (itm->text().simplified() != "")
            lst << itm->text();
    QSettings settings;
    settings.setValue("PathList", lst);
    emit OptionsUpdate();
    this->close();
}

void OptionsDialog::on_buttonBox_rejected()
{
    this->close();
}

void OptionsDialog::on_tbtnPathsDel_clicked()
{
    if (ui->lstwPath->selectedItems().count() > 0) {
        ui->lstwPath->takeItem(ui->lstwPath->currentRow());
//        ui->lstwPath->removeItemWidget(ui->lstwPath->selectedItems().at(0));
    }
}
