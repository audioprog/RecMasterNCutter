#include <QFile>
#include <QFileDialog>
#include <QTime>
#include <QCloseEvent>
#include <QtDebug>

#include "icondelegate.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    marks = new Marks();

    QObject::connect(ui->widget, SIGNAL(LenghtChanged(int,int)), this, SLOT(NewLength(int,int)));
    QObject::connect(ui->PosScrollBar, SIGNAL(valueChanged(int)), ui->widget, SLOT(NewPos(int)));
    QObject::connect(ui->PosScrollBar, SIGNAL(valueChanged(int)), this, SLOT(MainPosChanged(int)));
    QObject::connect(ui->widget, SIGNAL(PosChanged(int,bool)), this, SLOT(PosChanged(int,bool)));
    QObject::connect(ui->actionSaveMarks, SIGNAL(triggered()), ui->widget, SLOT(SaveMarks()));
    QObject::connect(ui->widget, SIGNAL(ContextMenuWanted(QPoint,int,int)), this, SLOT(ShowContextMenu(QPoint,int,int)));
    QObject::connect(ui->Overview, SIGNAL(LenghtChanged(int,int)), this, SLOT(OverviewLength(int,int)));
    QObject::connect(ui->Overview, SIGNAL(PosChanged(int,bool)), this, SLOT(OverviewPosChanged(int,bool)));
    QObject::connect(ui->OverPos, SIGNAL(valueChanged(int)), ui->Overview, SLOT(NewPos(int)));
    QObject::connect(ui->Overview, SIGNAL(OverviewMarkPosChanged(int)), this, SLOT(OverviewMarkChanged(int)));
    QObject::connect(marks, SIGNAL(MarksChanged()), ui->widget, SLOT(actualize()));
    QObject::connect(marks, SIGNAL(MarksChanged()), this, SLOT(MarksChanged()));

    audio = new AudioOutput();

    QObject::connect(ui->widget, SIGNAL(Play(qint64)), this, SLOT(PlayStart(qint64)));
    QObject::connect(audio, SIGNAL(PosChanged(qint64)), this, SLOT(PlayNotify(qint64)));

    ui->actionStandard->setIcon(mimages.icon(0));
    ui->actionStart_Track->setIcon(mimages.icon(1));
    ui->actionEnd_Track->setIcon(mimages.icon(2));
    ui->actionStart_Silence->setIcon(mimages.icon(3));
    ui->actionEnd_Silence->setIcon(mimages.icon(4));
    ui->actionStandard_1->setIcon(mimages.nricon(0));
    ui->actionStandard_2->setIcon(mimages.nricon(1));
    ui->actionStandard_3->setIcon(mimages.nricon(2));
    ui->actionAktuell->setIcon(mimages.nricon(3));
    ui->actionLastMarkStandard->setIcon(mimages.icon(0));
    ui->actionLastMarkStart_Track->setIcon(mimages.icon(1));
    ui->actionLastMarkEnd_Track->setIcon(mimages.icon(2));
    ui->actionLastMarkStart_Silence->setIcon(mimages.icon(3));
    ui->actionLastMarkEnd_Silence->setIcon(mimages.icon(4));
    ui->FollowWaveEnd->SetFollowEnd(true);
    ui->FollowWaveEnd->AddInsertPos(0.0);
    ui->FollowWaveEnd->AddInsertPos(0.4);
    ui->FollowWaveEnd->AddInsertPos(0.7);
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
    contextmenu->addActions(QList<QAction*>() << ui->actionDelete << ui->actionStart_Track << ui->actionEnd_Track << ui->actionStart_Silence << ui->actionEnd_Silence);

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

    this->setWindowTitle(QCoreApplication::organizationName() + " " + QCoreApplication::applicationName());

    tracks = new SaveTracks();
    tracks->SetMarks(marks);
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
         pathlist << ui->cboxPath->itemText(i);
     }
     if (pathlist.contains(ui->cboxPath->currentText())) {
         settings.setValue("PathIndex", pathlist.indexOf(QRegExp(ui->cboxPath->currentText())));
     }
     else {
         settings.setValue("PathIndex", pathlist.count());
         pathlist << ui->cboxPath->currentText();
     }
     settings.setValue("PathList", pathlist);
     event->accept();
}

void MainWindow::NewLength(int newLen, int windowlength)
{
    ui->PosScrollBar->setMaximum(newLen > windowlength ? newLen - windowlength : 0);
    ui->PosScrollBar->setPageStep(windowlength);
}

void MainWindow::PosChanged(int diff, bool absolut)
{
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

void MainWindow::MarksChanged()
{
    ui->markTable->setRowCount(marks->Count());
    ui->tableTracks->setRowCount(marks->Count(Marks::StartTrack));
    int idxStartTrack = -1, idxEndTrack = -1;
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
        int msecs = (int)((float)(marks->Pos(i) - secs) / 44.100);
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
            if (ui->tableTracks->item(idxStartTrack, 0) == NULL) {
                QTableWidgetItem *ti = new QTableWidgetItem();
                ti->setData(Qt::DisplayRole, qVariantFromValue(false));
                ti->setData(Qt::CheckStateRole, qVariantFromValue(0));
                ui->tableTracks->setItem(idxStartTrack, 0, ti);
            }
            if (ui->tableTracks->item(idxStartTrack, 1) != NULL) {
                ui->tableTracks->item(idxStartTrack, 1)->setText(txt);
                ui->tableTracks->item(idxStartTrack, 1)->setData(Qt::UserRole, qVariantFromValue(i));
            }
            else {
                QTableWidgetItem *di = new QTableWidgetItem(txt);
                di->setFlags(di->flags() & ~Qt::ItemIsEditable);
                di->setData(Qt::UserRole, qVariantFromValue(i));
                ui->tableTracks->setItem(idxStartTrack, 1, di);
            }
            if (ui->tableTracks->item(idxStartTrack, 3) == NULL) {
                ui->tableTracks->setItem(idxStartTrack, 3, new QTableWidgetItem(QString("")));
            }
        }
        else if (typ == Marks::EndTrack && idxEndTrack < ui->tableTracks->rowCount() - 1) {
            idxEndTrack++;
            if (ui->tableTracks->item(idxEndTrack, 2) != NULL) {
                ui->tableTracks->item(idxEndTrack, 2)->setText(txt);
                ui->tableTracks->item(idxEndTrack, 2)->setData(Qt::UserRole, qVariantFromValue(i));
            }
            else {
                QTableWidgetItem *di = new QTableWidgetItem(txt);
                di->setFlags(di->flags() & ~Qt::ItemIsEditable);
                di->setData(Qt::UserRole, qVariantFromValue(i));
                ui->tableTracks->setItem(idxEndTrack, 2, di);
            }
        }
    }
}

void MainWindow::PlayNotify(qint64 pos)
{
    ui->statusBar->showMessage(QString::number(pos));
}

void MainWindow::PlayStart(qint64 pos)
{
    audio->setFilePos(pos);
    audio->startPlaying();
}

void MainWindow::on_action_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Audio"), "", tr("Audio Files (*.raw *.wav)"));
    if (fileName.count() > 0) {
        QFile *file = new QFile(fileName);
        marks->Read(new QFile(fileName + ".rmrk"));
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
    }
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

void MainWindow::on_actionAktuell_triggered()
{
    ui->FollowWaveEnd->AddMarkAtInsertPos(3, Marks::Standard);
}

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

void MainWindow::on_tableTracks_cellChanged(int row, int column)
{
    if (ui->tableTracks->item(row, column) != NULL) {
        if (column == 0) {
            if (ui->tableTracks->item(row, column)->data(Qt::CheckStateRole) != 0) {
                QString path = ui->cboxPath->currentText().replace('\\', "/");
                if (!path.endsWith("/"))
                    path += "/";
                path += ui->dateEdit->date().toString("yyyy-MM-dd") + "/";
                if (ui->comboDayTime->currentIndex() < 3)
                    path += ui->comboDayTime->currentText().left(1);
                else
                    path += ui->comboDayTime->currentText();
                path += "/";
                tracks->SetPath(path);

                tracks->SaveTrack(qVariantValue<int>(ui->tableTracks->item(row, 1)->data(Qt::UserRole)));
                //Save title
                //sox -r 44100 -e signed -b 24 -c 2 input.raw Track.wav trim [start] [lenght]
                //sox "|sox input1 -p" "|sox -n -p" Track.wav splice ... : fade 300 0 300
                //or
                //and splice input1 input2 Track.wav  [sec].[msec]
                // fade [type] fade-in-length [stop-time [fade-out-length]]
            }
        }
    }
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
