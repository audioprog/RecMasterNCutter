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
