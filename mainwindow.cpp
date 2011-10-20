#include <QFile>
#include <QFileDialog>
#include <QTime>
#include <QCloseEvent>
#include <QProcess>
#include <QClipboard>
#include <QMessageBox>
//#include <QtDebug>

#include "icondelegate.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "optionsdialog.h"
#include "buttonstateitemdelegate.h"
#include "buttonstate.h"
#include "wavfile.h"
#include "ziprw.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    indexstart(2), indextext(4), indexmp3(5), DoNotNotify(false)
{
    ui->setupUi(this);

    QObject::connect(this, SIGNAL(mDebug(QString)), ui->pteDebug, SLOT(appendPlainText(QString)));

    marks = new Marks();
    QObject::connect(marks, SIGNAL(Debug(QString)), this, SLOT(Debug(QString)));

    ButtonStateItemDelegate *bsiDelegate = new ButtonStateItemDelegate();

    ui->tableTracks->setItemDelegate(bsiDelegate);

    QObject::connect(ui->widget, SIGNAL(LenghtChanged(int,int)), this, SLOT(NewLength(int,int)));
    QObject::connect(ui->PosScrollBar, SIGNAL(valueChanged(int)), ui->widget, SLOT(NewPos(int)));
    QObject::connect(ui->PosScrollBar, SIGNAL(valueChanged(int)), this, SLOT(MainPosChanged(int)));
    QObject::connect(ui->widget, SIGNAL(PosChanged(int,bool)), this, SLOT(PosChanged(int,bool)));
    //QObject::connect(ui->actionSaveMarks, SIGNAL(triggered()), ui->widget, SLOT(SaveMarks()));
    QObject::connect(ui->widget, SIGNAL(ContextMenuWanted(QPoint,int,int)), this, SLOT(ShowContextMenu(QPoint,int,int)));
    QObject::connect(ui->Overview, SIGNAL(LenghtChanged(int,int)), this, SLOT(OverviewLength(int,int)));
    QObject::connect(ui->Overview, SIGNAL(PosChanged(int,bool)), this, SLOT(OverviewPosChanged(int,bool)));
    QObject::connect(ui->OverPos, SIGNAL(valueChanged(int)), ui->Overview, SLOT(NewPos(int)));
    QObject::connect(ui->Overview, SIGNAL(OverviewMarkPosChanged(int)), this, SLOT(OverviewMarkChanged(int)));
    QObject::connect(marks, SIGNAL(MarksChanged()), ui->widget, SLOT(actualize()));
    QObject::connect(marks, SIGNAL(MarksChanged()), this, SLOT(MarksChanged()));

    //QObject::connect(bsiDelegate, SIGNAL(fieldClicked(int,int)), this, SLOT(onTitleFieldClicked(int,int)));

    audio = new AudioOutput();

    QObject::connect(ui->widget, SIGNAL(Play(qint64)), this, SLOT(PlayStart(qint64)));
    QObject::connect(audio, SIGNAL(PosChanged(qint64)), this, SLOT(PlayNotify(qint64)));
    QObject::connect(audio, SIGNAL(PosChanged(qint64)), ui->widget, SLOT(PlayPos(qint64)));
    QObject::connect(ui->vslVol, SIGNAL(valueChanged(int)), audio, SLOT(VolChange(int)));
    QObject::connect(audio, SIGNAL(Debug(QString)), this, SLOT(Debug(QString)));
    ui->cbxOutput->addItems(audio->DeviceList());

    ui->actionStandard->setIcon(mimages.icon(0));
    ui->actionStart_Track->setIcon(mimages.icon(1));
    ui->actionEnd_Track->setIcon(mimages.icon(2));
    ui->actionStart_Silence->setIcon(mimages.icon(3));
    ui->actionEnd_Silence->setIcon(mimages.icon(4));
    ui->actionStandard_1->setIcon(mimages.nricon(0));
    ui->actionStandard_2->setIcon(mimages.nricon(1));
    ui->actionStandard_3->setIcon(mimages.nricon(2));
    ui->actionStandard_4->setIcon(mimages.nricon(3));
    ui->actionLastMarkStandard->setIcon(mimages.icon(0));
    ui->actionLastMarkStart_Track->setIcon(mimages.icon(1));
    ui->actionLastMarkEnd_Track->setIcon(mimages.icon(2));
    ui->actionLastMarkStart_Silence->setIcon(mimages.icon(3));
    ui->actionLastMarkEnd_Silence->setIcon(mimages.icon(4));
    ui->FollowWaveEnd->SetFollowEnd(true);
    ui->FollowWaveEnd->AddInsertPos(0.0);
    ui->FollowWaveEnd->AddInsertPos(0.6);
    ui->FollowWaveEnd->AddInsertPos(0.9);
    ui->FollowWaveEnd->AddInsertPos(1.0);
    ui->Overview->SetRulerHeight(0);
    ui->Overview->SetDotWidthSecs(1);
    this->setCorner(Qt::TopLeftCorner, Qt:: LeftDockWidgetArea);
    this->setCorner(Qt::BottomLeftCorner, Qt:: LeftDockWidgetArea);
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    ui->markTable->setColumnWidth(0, 20);
    ui->markTable->setItemDelegateForColumn(0, new IconDelegate());

    contextmenuNr = -1;
    contextmenuX = -1;
    contextmenu = new QMenu(this);
    contextmenu->addActions(QList<QAction*>() << ui->actionDelete << ui->actionStart_Track << ui->actionEnd_Track << ui->actionStart_Silence << ui->actionEnd_Silence << ui->actionFadeIn << ui->actionFadeOut);

    ui->FollowWaveEnd->SetDebugNr(0);
    ui->widget->SetDebugNr(1);
    ui->Overview->SetDebugNr(2);

    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    ui->markTable->horizontalHeader()->restoreState(settings.value("MarksHeader").toByteArray());
    ui->tableTracks->horizontalHeader()->restoreState(settings.value("TracksHeader").toByteArray());

    ui->cboxPath->insertItems(0, settings.value("PathList", QStringList()).toStringList());
    ui->cboxPath->setCurrentIndex(settings.value("PathIndex", -1).toInt());
    ui->dateEdit->setDate(QDate::currentDate());
    if (QTime::currentTime().hour() < 12)
        ui->comboDayTime->setCurrentIndex(0);
    else if (QTime::currentTime().hour() < 17)
        ui->comboDayTime->setCurrentIndex(1);
    else
        ui->comboDayTime->setCurrentIndex(2);
    ui->vslVol->setValue(settings.value("Volume", 0).toInt());

    this->setWindowTitle(QCoreApplication::organizationName() + " " + QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());

    tracks = new SaveTracks();
    QObject::connect(tracks, SIGNAL(Finished(int)), this, SLOT(SaveTrackFinished(int)));
    tracks->SetMarks(marks);
#ifdef Q_OS_LINUX
    ui->comboDevices->setEditable(true);
#endif
    QObject::connect(tracks, SIGNAL(Debug(QString)), ui->pteDebug, SLOT(appendPlainText(QString)));

    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(on_btnListDevices_clicked()));
    timer.singleShot(1000, this, SLOT(on_btnListDevices_clicked()));

    ui->cboxSampleSize->setCurrentIndex(settings.value("SampleSize", 1).toInt());
#ifdef Q_OS_WIN32
    waveprog = settings.value("WaveProc", "C:/Program Files/Acon Digital Media/Acoustica 4/Acoustica.exe").toString();
    lameprog = settings.value("Lame", "C:/Program Files/lame/lame.exe").toString();
    mp3path = settings.value("MP3Path", "D:/MP3/Gottesdienste 2011 MP3/").toString();
#else
    waveprog = settings.value("WaveProc", "audacity").toString();
    lameprog = settings.value("Lame", "lame").toString();
    mp3path = settings.value("MP3Path", QDir::homePath()).toString();
#endif
    lameparams = settings.value("Lame Parameters", QStringList() << "-b" << "192" << "--cbr" << "-h" << "--tg" << "12").toStringList();

    readCDsources();

    initializing = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
 {
     QSettings settings;
     settings.setValue("geometry", saveGeometry());
     QByteArray ba = saveState();
     settings.setValue("windowState", ba);
     settings.setValue("MarksHeader", ui->markTable->horizontalHeader()->saveState());
     settings.setValue("TracksHeader", ui->tableTracks->horizontalHeader()->saveState());
     QStringList pathlist;
     for (int i = 0; i < ui->cboxPath->count(); i++) {
         if (ui->cboxPath->itemText(i).simplified() != "")
            pathlist << ui->cboxPath->itemText(i);
     }
     if (pathlist.contains(ui->cboxPath->currentText())) {
         int idx = pathlist.indexOf(ui->cboxPath->currentText());
         settings.setValue("PathIndex", idx);
     }
     else if (ui->cboxPath->currentText().simplified() != ""){
         settings.setValue("PathIndex", pathlist.count());
         pathlist << ui->cboxPath->currentText();
     }
     settings.setValue("PathList", pathlist);
     settings.setValue("Volume", ui->vslVol->value());
#ifdef Q_OS_LINUX
     settings.setValue("SampleSize", ui->cboxSampleSize->currentIndex());
#endif
     event->accept();
}

void MainWindow::NewLength(int newLen, int windowlength)
{
    ui->PosScrollBar->setMaximum(newLen > windowlength ? newLen - windowlength : 0);
    ui->PosScrollBar->setPageStep(windowlength);
}

void MainWindow::PosChanged(int diff, bool absolut)
{
    //qDebug() << diff << absolut;
    int nv = absolut ? diff : ui->PosScrollBar->value() + diff;
    bool update = false;
    if (nv < 0) {
        nv = 0;
        update = true;
    }
    if (nv > ui->PosScrollBar->maximum()) {
        nv = ui->PosScrollBar->maximum();
        update = true;
    }
    ui->PosScrollBar->setValue(nv);
    ui->widget->update();
}

void MainWindow::ShowContextMenu(QPoint pos, int x, int mark)
{
    contextmenuNr = mark;
    contextmenuX = x;
    ui->actionDelete->setEnabled(mark > -1);
    contextmenu->exec(pos);
}

void MainWindow::MainPosChanged(int newPos)
{
    int pos = (int)( (qint64)newPos * (qint64)ui->widget->DotWidth() / (qint64)ui->Overview->DotWidth() );
    ui->Overview->OverviewMarkChanged((int)((qint64)ui->PosScrollBar->pageStep() * (qint64)ui->widget->DotWidth() / (qint64)ui->Overview->DotWidth()), pos);
}

void MainWindow::OverviewLength(int newLen, int windowlength)
{
    ui->OverPos->setMaximum(newLen > windowlength ? newLen - windowlength : 0);
    ui->OverPos->setPageStep(windowlength);
}

void MainWindow::OverviewPosChanged(int diff, bool absolut)
{
    int nv = absolut ? diff : ui->OverPos->value() + diff;
    if (nv < 0)
        nv = 0;
    if (nv > ui->OverPos->maximum())
        nv = ui->OverPos->maximum();
    ui->OverPos->setValue(nv);
}

void MainWindow::OverviewMarkChanged(int newPos)
{
    int pos = (int) ((qint64)newPos * (qint64)ui->Overview->DotWidth() / (qint64)ui->widget->DotWidth());
    ui->PosScrollBar->setValue(pos);
}

void MainWindow::SaveTrackFinished(int startmark)
{
    qint64 fullsize = 0;
    QString path = getPath();
    for (int i = 0; i < ui->tableTracks->rowCount(); i++) {
        int row = qVariantValue<int>(ui->tableTracks->item(i, indexstart)->data(Qt::UserRole));
        if (row == startmark) {
            ButtonState bst = qVariantValue<ButtonState>(ui->tableTracks->item(i, 0)->data(0));
            bst.setIconMode(QIcon::Selected);
            ui->tableTracks->item(i, 0)->setData(0, qVariantFromValue(bst));
        }
        QString wavefile = path + QString::number(marks->Pos(row)) + ".wav";
        if (QFile(wavefile).exists())
            fullsize += WavFile(wavefile).SampleCount();
    }
    fullsize = (fullsize + 44099) / 44100;
    fullsize += (ui->tableTracks->rowCount() - 1) * 2;
    int min = fullsize / 60;
    QString secs = QString::number(fullsize - (min * 60));
    if (secs.count() < 2)
        secs = "0" + secs;
    ui->lblFullSize->setText(QString::number(min) + ":" + secs);
}

void MainWindow::MarksChanged()
{
    if (DoNotNotify)
        return;
    initializing = true;
    QString path = getPath();
    int fullsize = 0;

    ui->markTable->setRowCount(marks->Count());
    ui->tableTracks->setRowCount(marks->Count(Marks::StartTrack));
    int idxStartTrack = -1, idxEndTrack = -1;
    QList<int> inProcList = tracks->allMarks();
    for (int i = 0; i < marks->Count(); ++i) {
        Marks::MarkTypes typ;
        if (ui->markTable->item(i, 0) != NULL) {
            typ = static_cast<Marks::MarkTypes>(qVariantValue<int>(ui->markTable->item(i, 0)->data(Qt::DisplayRole)));
            if (typ != marks->Type(i)) {
                typ = marks->Type(i);
                QTableWidgetItem *wi = new QTableWidgetItem(mimages.icon(typ), "");
                wi->setData(Qt::DisplayRole, qVariantFromValue(static_cast<int>(typ)));
                ui->markTable->setItem(i, 0, wi);
            }
        }
        else {
            typ = marks->Type(i);
            QTableWidgetItem *wi = new QTableWidgetItem(mimages.icon(typ), "");
            wi->setData(Qt::DisplayRole, qVariantFromValue(static_cast<int>(typ)));
            ui->markTable->setItem(i, 0, wi);
        }
        int secs = marks->Pos(i) / 44100;
        int msecs = (int)((float)(marks->Pos(i) - (secs * 44100)) / 44.100);
        QTime time(0,0,0);
        time = time.addSecs(secs);
        time = time.addMSecs(msecs);
        QString txt = time.hour() > 0 ? time.toString("HH:mm:ss.zzz") : time.toString("mm:ss.zzz");
        if (ui->markTable->item(i, 1) == NULL) {
            QTableWidgetItem *di = new QTableWidgetItem(txt);
            di->setFlags(di->flags() & ~Qt::ItemIsEditable);
            ui->markTable->setItem(i, 1, di);
        }
        else {
            ui->markTable->item(i, 1)->setText(txt);
        }

        if (typ == Marks::StartTrack) {
            idxStartTrack++;
            QString wavefile = waveFile(idxStartTrack, i);
            if (ui->tableTracks->item(idxStartTrack, 0) == NULL) {
                QTableWidgetItem *ti = new QTableWidgetItem();
                if (QFile(wavefile).exists()) {
                    ButtonState state("Save");
                    if (inProcList.contains(i))
                        state.setIconMode(QIcon::Selected);
                    else {
                        state.setIconMode(QIcon::Active);
                        fullsize += WavFile(wavefile).SampleCount();
                    }
                    ti->setData(0, qVariantFromValue(state));
                }
                else {
                    if (inProcList.contains(i)) {
                        ButtonState state("Save");
                        state.setIconMode(QIcon::Active);
                        ti->setData(0, qVariantFromValue(state));
                    }
                    else
                        ti->setData(0, qVariantFromValue(ButtonState("Save")));
                }
                ui->tableTracks->setItem(idxStartTrack, 0, ti);
            }
            else {
                if (QFile(wavefile).exists()) {
                    if (inProcList.contains(i)) {
                        if (((ButtonState)qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0))).IconMode() != QIcon::Active) {
                            ButtonState bst = qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0));
                            bst.setIconMode(QIcon::Active);
                            ui->tableTracks->item(idxStartTrack, 0)->setData(0, qVariantFromValue(bst));
                        }
                        else
                            fullsize += WavFile(wavefile).SampleCount();
                    }
                    else if (((ButtonState)qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0))).IconMode() != QIcon::Selected) {
                        ButtonState bst = qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0));
                        bst.setIconMode(QIcon::Selected);
                        ui->tableTracks->item(idxStartTrack, 0)->setData(0, qVariantFromValue(bst));
                    }
                }
                else if (inProcList.contains(i)) {
                    if (((ButtonState)qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0))).IconMode() != QIcon::Active) {
                        ButtonState bst = qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0));
                        bst.setIconMode(QIcon::Active);
                        ui->tableTracks->item(idxStartTrack, 0)->setData(0, qVariantFromValue(bst));
                    }
                }
                else if (((ButtonState)qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0))).IconMode() != QIcon::Normal) {
                    ButtonState bst = qVariantValue<ButtonState>(ui->tableTracks->item(idxStartTrack, 0)->data(0));
                    bst.setIconMode(QIcon::Normal);
                    ui->tableTracks->item(idxStartTrack, 0)->setData(0, qVariantFromValue(bst));
                }
            }
            if (ui->tableTracks->item(idxStartTrack, 1) == NULL) {
                QTableWidgetItem *di = new QTableWidgetItem();
                di->setData(0, qVariantFromValue(ButtonState("Open", "OpenShaddow")));
                ui->tableTracks->setItem(idxStartTrack, 1, di);
            }
            if (ui->tableTracks->item(idxStartTrack, indexstart) != NULL) {
                ui->tableTracks->item(idxStartTrack, indexstart)->setText(txt);
                ui->tableTracks->item(idxStartTrack, indexstart)->setData(Qt::UserRole, qVariantFromValue(i));
            }
            else {
                QTableWidgetItem *di = new QTableWidgetItem(txt);
                di->setFlags(di->flags() & ~Qt::ItemIsEditable);
                di->setData(Qt::UserRole, qVariantFromValue(i));
                ui->tableTracks->setItem(idxStartTrack, indexstart, di);
            }
            if (ui->tableTracks->item(idxStartTrack, indextext) == NULL) {
                ui->tableTracks->setItem(idxStartTrack, indextext, new QTableWidgetItem(QString(marks->Text(i))));
            }
            if (ui->tableTracks->item(idxStartTrack, indexmp3) == NULL) {
                ButtonState mp3state("MP3");
                if (QFile(MP3File(idxStartTrack)).exists())
                    mp3state.setIconMode(QIcon::Selected);
                QTableWidgetItem *ti = new QTableWidgetItem();
                ti->setData(0, qVariantFromValue(mp3state));
                ui->tableTracks->setItem(idxStartTrack, indexmp3, ti);
            }
        }
        else if (typ == Marks::EndTrack && idxEndTrack < ui->tableTracks->rowCount() - 1) {
            idxEndTrack++;
            if (ui->tableTracks->item(idxEndTrack, 3) != NULL) {
                ui->tableTracks->item(idxEndTrack, 3)->setText(txt);
                ui->tableTracks->item(idxEndTrack, 3)->setData(Qt::UserRole, qVariantFromValue(i));
            }
            else {
                QTableWidgetItem *di = new QTableWidgetItem(txt);
                di->setFlags(di->flags() & ~Qt::ItemIsEditable);
                di->setData(Qt::UserRole, qVariantFromValue(i));
                ui->tableTracks->setItem(idxEndTrack, 3, di);
            }
        }
    }
    fullsize = (fullsize + 44099) / 44100;
    fullsize += (idxStartTrack - 1) * 2;
    int min = fullsize / 60;
    QString secs = QString::number(fullsize - (min * 60));
    if (secs.count() < 2)
        secs = "0" + secs;
    ui->lblFullSize->setText(QString::number(min) + ":" + secs);
    initializing = false;
}

void MainWindow::PlayNotify(qint64 pos)
{
    ui->statusBar->showMessage(QString::number(pos));
}

void MainWindow::PlayStart(qint64 pos)
{
    //qbbug() << "PlayStart" << pos;
    audio->setHardware(ui->cbxOutput->currentIndex());
    audio->VolChange(ui->vslVol->value());
    if (audio->isPlaying())
        audio->stop();
    else {
        audio->setFilePos(pos);
        audio->startPlaying(pos);
    }
}

void MainWindow::OptionsUpdate()
{
    for (int i = ui->cboxPath->count(); i >= 0; i--)
        ui->cboxPath->removeItem(0);
    QSettings settings;
    ui->cboxPath->insertItems(0, settings.value("PathList", QStringList()).toStringList());
    tracks->ReadSettings();
    readCDsources();
}

void MainWindow::on_action_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Audio"), getPath(), tr("Audio Files (*.raw *.wav)"));
    open(fileName);
}

void MainWindow::on_actionDelete_triggered()
{
    ui->widget->getMarks()->Remove(contextmenuNr);
}

void MainWindow::on_actionStandard_1_triggered()
{
    ui->FollowWaveEnd->AddMarkAtInsertPos(0, Marks::Standard);
}

void MainWindow::on_actionStandard_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::Standard);
    else {
        ui->widget->AddMark(contextmenuX, Marks::Standard);
    }
}

void MainWindow::on_actionStart_Track_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::StartTrack);
    else
        ui->widget->AddMark(contextmenuX, Marks::StartTrack);
}

void MainWindow::on_actionEnd_Track_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::EndTrack);
    else
        ui->widget->AddMark(contextmenuX, Marks::EndTrack);
}

void MainWindow::on_actionStart_Silence_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::StartSilence);
    else
        ui->widget->AddMark(contextmenuX, Marks::StartSilence);
}

void MainWindow::on_actionEnd_Silence_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::EndSilence);
    else
        ui->widget->AddMark(contextmenuX, Marks::EndSilence);
}

void MainWindow::on_actionStandard_2_triggered()
{
    ui->FollowWaveEnd->AddMarkAtInsertPos(1, Marks::Standard);
}

void MainWindow::on_actionStandard_3_triggered()
{
    ui->FollowWaveEnd->AddMarkAtInsertPos(2, Marks::Standard);
}

/*void MainWindow::on_actionAktuell_triggered()
{
    ui->FollowWaveEnd->AddMarkAtInsertPos(3, Marks::Standard);
}*/

void MainWindow::on_actionLastMarkStandard_triggered()
{
    ui->widget->getMarks()->setMark(Marks::Standard);
}

void MainWindow::on_actionLastMarkStart_Track_triggered()
{
    ui->widget->getMarks()->setMark(Marks::StartTrack);
}

void MainWindow::on_actionLastMarkEnd_Track_triggered()
{
    ui->widget->getMarks()->setMark(Marks::EndTrack);
}

void MainWindow::on_actionLastMarkStart_Silence_triggered()
{
    ui->widget->getMarks()->setMark(Marks::StartSilence);
}

void MainWindow::on_actionLastMarkEnd_Silence_triggered()
{
    ui->widget->getMarks()->setMark(Marks::EndSilence);
}

void MainWindow::on_markTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->widget->Select(currentRow);
}

void MainWindow::on_actionPrevious_triggered()
{
    ui->widget->SelectPrevious();
}

void MainWindow::on_actionNext_triggered()
{
    ui->widget->SelectNext();
}

void MainWindow::on_actionFirst_triggered()
{
    ui->widget->Select(0);
}

void MainWindow::on_actionLast_triggered()
{
    ui->widget->Select(marks->Count()-1);
}

void MainWindow::on_actionEditDelete_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->Remove(ui->widget->Selected());
}

void MainWindow::on_actionEditStandard_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::Standard);
}

void MainWindow::on_actionEditStart_Track_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::StartTrack);
}

void MainWindow::on_actionEditEnd_Track_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::EndTrack);
}

void MainWindow::on_actionEditStart_Silence_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::StartSilence);
}

void MainWindow::on_actionEditEnd_Silence_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::EndSilence);
}

void MainWindow::on_actionPos1_triggered()
{
    ui->PosScrollBar->setValue(0);
}

void MainWindow::on_actionEnd_triggered()
{
    ui->PosScrollBar->setValue(ui->PosScrollBar->maximum());
}

void MainWindow::on_actionStop_triggered()
{
    audio->stop();
}

void MainWindow::on_actionPlay_triggered()
{
    ui->widget->Play();
}

void MainWindow::on_actionZoomOut_triggered()
{
    ui->FollowWaveEnd->SetDotWidth(ui->FollowWaveEnd->DotWidth() * 2);
}

void MainWindow::on_actionZoomIn_triggered()
{
    ui->FollowWaveEnd->SetDotWidth(ui->FollowWaveEnd->DotWidth() / 2);
}

void MainWindow::on_actionRefresh_triggered()
{
    ui->widget->Clear();
    ui->Overview->Clear();
}

void MainWindow::on_actionExpand_WaveForm_toggled(bool arg1)
{
    ui->widget->setExpand(arg1);
    ui->Overview->setExpand(arg1);
}

void MainWindow::on_btnListDevices_clicked()
{
    QProcess devlist;
    devlist.setProcessChannelMode(QProcess::SeparateChannels);
#ifdef Q_OS_WIN32
    devlist.start("parec -s");
#else
    devlist.start("arecord -l");
#endif
    if (devlist.waitForFinished()) {
        ui->comboDevices->clear();
        QStringList lines = ((QString)devlist.readAll()).split('\n');
        foreach (QString line, lines) {
#ifdef Q_OS_WIN32
            if (line.contains('=')) {
                bool ok;
                int nr = line.section('=', 0,0).toInt(&ok);
                Q_UNUSED(nr);
                if (ok)
                    ui->comboDevices->addItem(line);
            }
#else
            if (line.contains(':') && !line.startsWith(" ")) {
                QString txt = line.section(' ', 1, -1).replace(QRegExp(" Ger..?t"), "").replace(QRegExp("[:] [^:\\[]+ \\["), ": ").replace("]", "").replace(" Analog", "").replace(" Audio", "");
                txt = txt.section(':', 0, 0).simplified() + "," + txt.section(':', 1, 1).section(',', -1, -1).simplified()
                        + "=" + txt.section(':', 1, 1).section(',', 0, -2).simplified() + " " + txt.section(':', -1, -1);
                ui->comboDevices->addItem(txt);
                emit mDebug(line);
            }
#endif
        }
#ifdef Q_OS_LINUX
        devlist.start("arecord -L");
        if (devlist.waitForFinished()) {
            lines = ((QString)devlist.readAll()).split('\n');
            foreach (QString line, lines) {
                if (!line.startsWith(" ") && line != "null")
                    ui->comboDevices->addItem(line);
            }
        }
#endif

        QSettings settings;
        int lastnr = settings.value("LastRecChannel", qVariantFromValue((int)0)).toInt();
        for (int i = 0; i < ui->comboDevices->count(); i++) {
            bool ok;
            int nr = ui->comboDevices->itemText(i).section('=', 0,0).toInt(&ok);
            if (ok)
                if (nr == lastnr) {
                    ui->comboDevices->setCurrentIndex(i);
                    break;
                }
        }

        //ui->pteDebug->setPlainText(devlist.readAll());
    }
}

void MainWindow::on_btnStartRec_clicked()
{
    marks->setSampleSize(SampleSize());
#ifdef Q_OS_WIN32
    if (ui->comboDevices->currentText().contains('=')) {
        bool ok;
        int nr = ui->comboDevices->currentText().section('=', 0, 0).toInt(&ok);
        if (ok) {
            QSettings settings;
            settings.setValue("LastRecChannel", qVariantFromValue(nr));
            QString path = getPath();
            QDir(path).mkpath(path);
            if (QFile(path + "full.raw").exists()) {
                int i;
                for (i = 1; QFile(path + "full" + QString::number(i) + ".raw").exists(); ++i) {}
                path += "full" + QString::number(i) + ".raw";
            }
            else
                path += "full.raw";
            if (QProcess::startDetached("parec " + QString::number(nr) + " \"" + path + "\""))
                on_btnOpen_clicked();
        }
    }
#else
    bool ok;
    QString hwid = ui->comboDevices->currentText();
    if (hwid.contains("=") && !hwid.contains(":")) {
        hwid = hwid.section('=', 0, 0);
        hwid.section(',', 0, 0).toInt(&ok);
        if (ok) {
            hwid.section(',', -1, -1).toInt(&ok);
            if (ok) {
                hwid = "hw:" + hwid;
            }
        }
    }
    else if (ui->comboDevices->currentText() != "") {
        ok = true;
    }
    if (ok) {
        QSettings settings;
        settings.setValue("LastRecChannel", qVariantFromValue(hwid));
        QString path = getPath();
        QDir(path).mkpath(path);
        QString bitformat;
        mDebug(QString::number(marks->SampleSize()));
        if (marks->SampleSize() < 0)
            bitformat = "FLOAT_LE";
        else
            bitformat = "S" + QString::number(marks->SampleSize() * 8) + "_LE";
        QString cmd = "konsole -e arecord -f " + bitformat + " -r 44100 -c 2 -D " + hwid + " \"" + path + "full.raw\"";
        mDebug(cmd);
        if (QProcess::startDetached(cmd))
            on_btnOpen_clicked();
    }
#endif
}

QString MainWindow::getPath()
{
    QString path = ui->cboxPath->currentText().replace('\\', "/");
    if (!path.endsWith("/"))
        path += "/";
    path += ui->dateEdit->date().toString("yyyy-MM-dd") + "/";
    if (ui->comboDayTime->currentIndex() < 3)
        path += ui->comboDayTime->currentText().left(1);
    else
        path += ui->comboDayTime->currentText();
    path += "/";
    return path;
}

void MainWindow::open(QString fileName)
{
    if (fileName.count() > 0) {
        QFile *file = new QFile(fileName);
        marks->setSampleSize(SampleSize());
        DoNotNotify = true;
        marks->Read(new QFile(fileName + ".rmrk"));
        DoNotNotify = false;
        if (fileName.section('.', -1, -1).toLower() == "wav") {
            WavFile wf(fileName);
            int ssiz = wf.fileFormat().sampleSize() / 8;
            marks->setSampleSize(ssiz);
            mDebug(QString::number(ssiz) + "\n");
            if (ssiz == 4 && wf.fileFormat().sampleType() == QAudioFormat::Float)
                ui->cboxSampleSize->setCurrentIndex(3);
            else
                ui->cboxSampleSize->setCurrentIndex(ssiz - 2);
        }
        else
        {
            if (marks->SampleSize() < 0)
                ui->cboxSampleSize->setCurrentIndex(3);
            else
                ui->cboxSampleSize->setCurrentIndex(marks->SampleSize() - 2);
        }
        ui->sbxStartNr->setValue(marks->StartNr());
        ui->widget->SetFile(file); //
        ui->widget->setMarks(marks);
        tracks->SetMarks(marks);
        ui->FollowWaveEnd->SetFile(file);
        ui->FollowWaveEnd->setMarks(marks);
        ui->FollowWaveEnd->SetDotWidth(ui->widget->DotWidth() * 3);
        ui->Overview->SetFile(file);
        ui->Overview->OverviewMarkChanged((int)((qint64)ui->PosScrollBar->pageStep() * (qint64)ui->widget->DotWidth() / (qint64)ui->Overview->DotWidth()), 0);
        tracks->SetFile(file);
        audio->setFile(fileName);
        audio->setMarks(marks);
        MarksChanged();
    }
}

int MainWindow::SampleSize()
{
    if (ui->cboxSampleSize->currentIndex() == -1)
        return 3;
    if (ui->cboxSampleSize->currentIndex() == 3)
        return -4;
    else
        return ui->cboxSampleSize->currentIndex() + 2;
}

QString MainWindow::MP3File(int title)
{
    QString pmp3path = mp3path;
    if (!pmp3path.endsWith("/"))
        pmp3path += "/";
    pmp3path += ui->dateEdit->date().toString("yyyy-MM-dd");
    if (ui->comboDayTime->currentIndex() < 3)
        pmp3path += ui->comboDayTime->currentText().left(1);
    else
        pmp3path += ui->comboDayTime->currentText();
    pmp3path += "/";
    pmp3path += ui->dateEdit->date().toString("dd.MM.yyyy") + " ";
    pmp3path += ui->comboDayTime->currentText();
    pmp3path += "/";

    QString newname = pmp3path.replace('/', '\\');
    int realtitle = title + ui->sbxStartNr->value();
    if (realtitle < 10)
        newname += "0";
    newname += QString::number(realtitle);
    if (ui->tableTracks->item(title, indextext)->text().simplified() != "")
        newname += " " + ui->tableTracks->item(title, indextext)->text().simplified();
    newname += ".mp3";
    return newname;
}

void MainWindow::on_btnOpen_clicked()
{
    QString path = getPath();
    while (!QFile::exists(path + "full.raw") && ui->comboDayTime->currentIndex() > 0) {
        ui->comboDayTime->setCurrentIndex(ui->comboDayTime->currentIndex()-1);
        path = getPath();
    }

    if (QFile::exists(path + "full.raw")) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Audio"), getPath() + "full.raw", tr("Audio Files (*.raw *.wav)"));
        open(fileName);
    }
}

void MainWindow::on_btnChangePath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (path != "") {
        if (!path.endsWith('\\') && !path.endsWith('/'))
            path += "/";
        ui->cboxPath->setEditText(path.replace('\\', '/'));
    }
}

void MainWindow::on_actionOptions_triggered()
{
    optins = new OptionsDialog(this);
    QObject::connect(optins, SIGNAL(OptionsUpdate()), this, SLOT(OptionsUpdate()));
    optins->show();
}

void MainWindow::on_actionFadeIn_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::FadeIn);
    else
        ui->widget->AddMark(contextmenuX, Marks::FadeIn);
}

void MainWindow::on_actionFadeOut_triggered()
{
    if (contextmenuNr > -1)
        ui->widget->getMarks()->setMark(contextmenuNr, Marks::FadeOut);
    else
        ui->widget->AddMark(contextmenuX, Marks::FadeOut);
}

void MainWindow::on_actionEditFadeIn_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::FadeIn);
}

void MainWindow::on_actionEditFadeOut_triggered()
{
    if (ui->widget->Selected() > -1)
        marks->setMark(ui->widget->Selected(), Marks::FadeOut);
}

void MainWindow::on_actionLastFadeIn_triggered()
{
    ui->widget->getMarks()->setMark(Marks::FadeIn);
}

void MainWindow::on_actionLastFadeOut_triggered()
{
    ui->widget->getMarks()->setMark(Marks::FadeOut);
}

void MainWindow::on_tableTracks_cellDoubleClicked(int row, int column)
{
    if (initializing)
        return;
    if (ui->tableTracks->item(row, column) != NULL) {
        if (column == 0) {
            if (ui->tableTracks->item(row, indexstart + 1)->text() != "") {
                int start = qVariantValue<int>(ui->tableTracks->item(row, indexstart)->data(Qt::UserRole));
                if (marks->Count(Marks::EndTrack, start) > 0) {
                    QString path = getPath();
                    tracks->SetPath(path);

                    tracks->SaveTrack(start);

                    ButtonState st = qVariantValue<ButtonState>(ui->tableTracks->item(row, 0)->data(0));
                    st.setIconMode(QIcon::Disabled);
                    ui->tableTracks->item(row, 0)->setData(0, qVariantFromValue(st));
                }
            }
        }
        else if (column == 1) {
            //int idx = qVariantValue<int>(ui->tableTracks->item(row, indexstart)->data(Qt::UserRole));
            //qint64 pos = marks->Pos(idx);
            QString path = getPath();
            QString filename = waveFile(row);

            if (QFile(path + filename).exists()) {
                QProcess::startDetached(waveprog, QStringList(path + filename));
                ButtonState st = qVariantValue<ButtonState>(ui->tableTracks->item(row, 1)->data(0));
                st.setIconMode(QIcon::Disabled);
                ui->tableTracks->item(row, 1)->setData(0, qVariantFromValue(st));
            }
        }
        else if (column == indexstart) {
            int idx = qVariantValue<int>(ui->tableTracks->item(row, indexstart)->data(Qt::UserRole));
            ui->markTable->selectRow(idx);
        }
        else if (column == indexmp3) {
            //if (ui->tableTracks->item(row, 0)->data(Qt::CheckStateRole) != 0) {
                QString path = getPath();
                //int pnam = marks->Pos(qVariantValue<int>(ui->tableTracks->item(row, indexstart)->data(Qt::UserRole)));
                QString name = path.replace('/', '\\') + waveFile(row);
                if (QFile(name).exists()) {
                    QString pmp3path = mp3path;
                    if (!pmp3path.endsWith("/"))
                        pmp3path += "/";
                    pmp3path += ui->dateEdit->date().toString("yyyy-MM-dd");
                    if (ui->comboDayTime->currentIndex() < 3)
                        pmp3path += ui->comboDayTime->currentText().left(1);
                    else
                        pmp3path += ui->comboDayTime->currentText();
                    pmp3path += "/";
                    pmp3path += ui->dateEdit->date().toString("dd.MM.yyyy") + " ";
                    pmp3path += ui->comboDayTime->currentText();
                    pmp3path += "/";

                    QString newname = pmp3path.replace('/', '\\');
                    int outnr = row + ui->sbxStartNr->value();
                    if (outnr < 10)
                        newname += "0";
                    newname += QString::number(outnr);
                    if (ui->tableTracks->item(row, indextext)->text().simplified() != "")
                        newname += " " + ui->tableTracks->item(row, indextext)->text().simplified();
                    newname += ".mp3";
                    QStringList params(lameparams);
                    params << name << newname;

                    QDir(pmp3path).mkpath(pmp3path);
                    emit mDebug(lameprog.replace('/', '\\') + " " + params.join(" ").replace('/', '\\'));

                    QProcess::startDetached(lameprog, params);
                    ButtonState st = qVariantValue<ButtonState>(ui->tableTracks->item(row, indexmp3)->data(0));
                    st.setIconMode(QIcon::Selected);
                    ui->tableTracks->item(row, indexmp3)->setData(0, qVariantFromValue(st));
                }
            //}
        }
    }
}

void MainWindow::on_tableTracks_cellChanged(int row, int column)
{
    if (column == indextext) {
        int idx = qVariantValue<int>(ui->tableTracks->item(row, indexstart)->data(Qt::UserRole));
        marks->setText(idx, ui->tableTracks->item(row, column)->text());
    }
}

void MainWindow::on_actionStandard_4_triggered()
{
    ui->FollowWaveEnd->AddMarkAtInsertPos(3, Marks::Standard);
}

void MainWindow::on_btbRereadOutput_clicked()
{
    ui->cbxOutput->clear();
    ui->cbxOutput->addItems(audio->DeviceList());
}

void MainWindow::on_actionSaveMarks_triggered()
{
    ui->widget->SaveMarks(ui->sbxStartNr->value());
}

void MainWindow::on_actionPlay_From_Mark_triggered()
{
    if (ui->widget->Selected() > -1) {
        qint64 newpos = marks->Pos(ui->widget->Selected());
        audio->startPlaying(newpos * marks->SampleSize() * 2);
    }
}

void MainWindow::on_tbtCopy_clicked()
{
    QString txt = ui->dateEdit->text() + ui->comboDayTime->currentText();
    for (int i = 0; i < ui->tableTracks->rowCount(); i++) {
        txt += "\n" + QString::number(i + 1) + " " + ui->tableTracks->item(i, indextext)->text();
    }
    QApplication::clipboard()->setText(txt, QClipboard::Clipboard);
}

void MainWindow::on_tbtCDsource_clicked()
{
    QSettings settings;

    QString path = settings.value("CDdest", "").toString();
    if (path == "") {
        QString npath = QFileDialog::getExistingDirectory(this, tr("CD dest"), QDir::homePath());
        if (npath != "") {
            path = npath;
            settings.setValue("CDdest", path);
        }
    }
    if (path.count() > 0 && ui->cbxCDsource->currentText() != "") {
        QString src = settings.value("CDsource", QDir::homePath()).toString();
        QString txt = ZipRW::FileText(src + "/" + ui->cbxCDsource->currentText(), "content.xml");

        txt = txt.replace(">title<", ">" + ui->ledCDTitle->text() + "<");
        txt = txt.replace(">option 1<", ">" + ui->ledCDOpt1->text() + "<");
        txt = txt.replace(">option 2<", ">" + ui->ledCDOpt2->text() + "<");
        txt = txt.replace(">date<", ">" + ui->dateEdit->date().toString("dd.MM.yyyy") + " " + ui->comboDayTime->currentText() + "<");

        QString dst = path + "/" + ui->dateEdit->date().toString("yyyy-MM-dd") + " " + ui->comboDayTime->currentText();
        ZipRW::CpFileText(src + "/" + ui->cbxCDsource->currentText(), dst, "content.xml", txt);
    }
}

void MainWindow::readCDsources()
{
    QSettings settings;
    QDir dir(settings.value("CDsource", QDir::homePath()).toString());
    QStringList lst = dir.entryList(QStringList("*.odg"));
    ui->cbxCDsource->clear();
    foreach (QString itm, lst) {
        ui->cbxCDsource->addItem(itm);
    }
}

QString MainWindow::waveFile(int Nr, int MarkNr)
{
    int outnr = Nr + ui->sbxStartNr->value();
    QString path = getPath();
    if (!path.endsWith('/'))
        path += "/";
    int idx = MarkNr;
    if (idx == -1)
        idx = qVariantValue<int>(ui->tableTracks->item(Nr, indexstart)->data(Qt::UserRole));
    qint64 pos = marks->Pos(idx);
    QString filename = QString::number(pos) + ".wav";
    if (MarkNr == -1 && QFile(path + filename).exists()) {
        QString newname = QString::number(outnr);
        if (newname.count() == 1)
            newname = "0" + newname;
        if (ui->tableTracks->item(Nr, indextext)->text().simplified() != "")
            newname += " " + ui->tableTracks->item(Nr, indextext)->text().simplified();
        QFile(path + filename).rename(path + newname + ".wav");
        return path + newname + ".wav";
    }
    if (!QFile(path + filename).exists()) {
        QString newname = QString::number(outnr);
        if (newname.count() == 1)
            newname = "0" + newname;
        QStringList lst = QDir(path).entryList(QStringList(newname + "*.wav"));
        if (lst.count() > 0)
            filename = lst.at(0);
    }
    return filename;
}

void MainWindow::on_actionCreateCDLabel_triggered()
{
    QString txt = ui->dateEdit->text() + ui->comboDayTime->currentText();
    if (ui->ledCDTitle->text() != "")
        txt += "\n" + ui->ledCDTitle->text();
    if (ui->ledCDOpt1->text() != "")
        txt += "\n" + ui->ledCDOpt1->text();
    if (ui->ledCDOpt2->text() != "")
        txt += "\n" + ui->ledCDOpt2->text();
    for (int i = 0; i < ui->tableTracks->rowCount(); i++) {
        txt += "\n" + QString::number(i + 1) + " " + ui->tableTracks->item(i, indextext)->text();
    }
    QSettings settings;
    QString path = settings.value("Textfilespath", QDir::homePath()).toString();
    QString filename = path + "/" + ui->dateEdit->date().toString("yyyy-MM-dd") + " " + ui->comboDayTime->currentText() + ".txt";
    emit Debug(filename);
    QFile fil(filename);
    fil.open(QFile::WriteOnly);
    QByteArray ba1(3, (char)0xEF);
    ba1[1] = (char)0xBB;
    ba1[2] = (char)0xBF;
    fil.write(ba1);
    QByteArray ba = txt.toUtf8();
    fil.write(ba);
    fil.flush();
    fil.close();
}

void MainWindow::on_actionRec_triggered()
{
    on_btnStartRec_clicked();
}

void MainWindow::on_actionOpenRec_triggered()
{
    QString path = getPath();
    while (!QFile::exists(path + "full.raw") && ui->comboDayTime->currentIndex() > 0) {
        ui->comboDayTime->setCurrentIndex(ui->comboDayTime->currentIndex()-1);
        path = getPath();
    }

    if (QFile::exists(path + "full.raw")) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Audio"),
                                                        getPath() + "full.raw", tr("Audio Files (*.raw *.wav)"));
        open(fileName);
    }
}
