#include <QSettings>
#include <QStringList>
#include <QFileDialog>

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

#ifdef Q_OS_WIN32
    QString waveprog = settings.value("WaveProc", "C:/Program Files/Acon Digital Media/Acoustica 4/Acoustica.exe").toString();
    QString lameprog = settings.value("Lame", "C:/Program Files/lame/lame.exe").toString();
    QString mp3path = settings.value("MP3Path", "D:/MP3/Gottesdienste 2011 MP3/").toString();
#else
    QString waveprog = settings.value("WaveProc", "audacity").toString();
    QString lameprog = settings.value("Lame", "lame").toString();
    QString mp3path = settings.value("MP3Path", QDir::homePath()).toString();
#endif
    ui->ledMp3Path->setText(mp3path);
    ui->ledSoxPath->setText(settings.value("soxpath", "").toString());
    ui->ledLamePath->setText(lameprog);
    ui->ledAudioProc->setText(waveprog);
    ui->ledTextfilespath->setText(settings.value("Textfilespath", QDir::homePath()).toString());
    ui->ledCDsrc->setText(settings.value("CDsource", QDir::homePath()).toString());
    ui->ledCDdest->setText(settings.value("CDdest", QDir::homePath()).toString());
    lst = settings.value("TitleList", QStringList()).toStringList();
    lst.sort();
    ui->lstwTitle->addItems(lst);
    lst = settings.value("Option1List", QStringList()).toStringList();
    lst.sort();
    ui->lstwOpts1->addItems(lst);
    lst = settings.value("Option2List", QStringList()).toStringList();
    lst.sort();
    ui->lstwOpts2->addItems(lst);
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
    settings.setValue("MP3Path", ui->ledMp3Path->text());
    settings.setValue("soxpath", ui->ledSoxPath->text());
    settings.setValue("Lame", ui->ledLamePath->text());
    settings.setValue("WaveProc", ui->ledAudioProc->text());
    settings.setValue("Textfilespath", ui->ledTextfilespath->text());
    settings.setValue("CDsource", ui->ledCDsrc->text());
    settings.setValue("CDdest", ui->ledCDdest->text());
    lstw.clear();
    lst.clear();
    lstw = ui->lstwTitle->findItems("", Qt::MatchStartsWith);
    foreach (QListWidgetItem* itm, lstw)
        if (itm->text().simplified() != "")
            lst << itm->text();
    settings.setValue("TitleList", lst);
    lst.clear();
    lstw = ui->lstwOpts1->findItems("", Qt::MatchStartsWith);
    foreach (QListWidgetItem* itm, lstw)
        if (itm->text().simplified() != "")
            lst << itm->text();
    settings.setValue("Option1List", lst);
    lst.clear();
    lstw = ui->lstwOpts2->findItems("", Qt::MatchStartsWith);
    foreach (QListWidgetItem* itm, lstw)
        if (itm->text().simplified() != "")
            lst << itm->text();
    settings.setValue("Option2List", lst);
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

void OptionsDialog::on_tbtMp3Path_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("MP3 Path"), ui->ledMp3Path->text());
    if (path != "") {
        ui->ledMp3Path->setText(path);
    }
}

void OptionsDialog::on_tbtSoxPath_clicked()
{
#ifdef Q_OS_WIN32
    QString file = QFileDialog::getOpenFileName(this, "sox Audio Programm", ui->ledSoxPath->text() + "/sox.exe", "sox.exe");
#else
    QString file = QFileDialog::getOpenFileName(this, "sox Audio Programm", ui->ledSoxPath->text() + "/sox", "sox");
#endif
    if (file != "") {
        file = file.section('/', 0, -2);
        ui->ledSoxPath->setText(file);
    }
}

void OptionsDialog::on_tbtLamePath_clicked()
{
#ifdef Q_OS_WIN32
    QString file = QFileDialog::getOpenFileName(this, "lame MP3 Audio Programm", ui->ledLamePath->text(), "lame.exe");
#else
    QString file = QFileDialog::getOpenFileName(this, "lame MP3 Audio Programm", ui->ledLamePath->text(), "lame");
#endif
    if (file != "") {
        ui->ledLamePath->setText(file);
    }
}

void OptionsDialog::on_tbtAudioProc_clicked()
{
#ifdef Q_OS_WIN32
    QString file = QFileDialog::getOpenFileName(this, "Audio Edit Programm", ui->ledAudioProc->text(), "*.exe");
#else
    QString file = QFileDialog::getOpenFileName(this, "Audio Edit Programm", ui->ledAudioProc->text(), "");
#endif
    if (file != "") {
        ui->ledAudioProc->setText(file);
    }
}

void OptionsDialog::on_btnTextfilepath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Textfiles Directory"), ui->ledTextfilespath->text());
    if (path != "")
        ui->ledTextfilespath->setText(path);
}

void OptionsDialog::on_btnCDsrc_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("CD Label source directory"), ui->ledTextfilespath->text());
    if (path != "")
        ui->ledCDsrc->setText(path);
}

void OptionsDialog::on_btnCDdest_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("CD Label dest directory"), ui->ledTextfilespath->text());
    if (path != "")
        ui->ledCDdest->setText(path);
}

void OptionsDialog::on_tbtnTitleDel_clicked()
{
    if (ui->lstwTitle->selectedItems().count() > 0)
        ui->lstwTitle->takeItem(ui->lstwTitle->currentRow());
}

void OptionsDialog::on_tbtnOpts1Del_clicked()
{
    if (ui->lstwOpts1->selectedItems().count() > 0)
        ui->lstwOpts1->takeItem(ui->lstwOpts1->currentRow());
}

void OptionsDialog::on_tbtnOpts2Del_clicked()
{
    if (ui->lstwOpts2->selectedItems().count() > 0)
        ui->lstwOpts2->takeItem(ui->lstwOpts2->currentRow());
}
