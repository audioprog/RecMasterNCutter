#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QModelIndex>
#include "marks.h"
#include "markimages.h"
#include "savetracks.h"
#include "audiooutput.h"
#include "optionsdialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void mDebug(QString text);

public slots:
    void NewLength(int newLen, int windowlength);
    void PosChanged(int diff, bool absolut = false);
    void ShowContextMenu(QPoint pos, int x, int mark);
    void MainPosChanged(int newPos);
    void OverviewLength(int newLen, int windowlength);
    void OverviewPosChanged(int diff, bool absolut = false);
    void OverviewMarkChanged(int newPos);
    void SaveTrackFinished(int startmark);

    void Debug(QString text) { emit mDebug(text); }

private slots:
    void MarksChanged();
    void PlayNotify(qint64 pos);
    void PlayStart(qint64 pos);
    void OptionsUpdate();

    void on_action_Open_triggered();

    void on_actionDelete_triggered();

    void on_actionStandard_1_triggered();

    void on_actionStandard_triggered();

    void on_actionStart_Track_triggered();

    void on_actionEnd_Track_triggered();

    void on_actionStart_Silence_triggered();

    void on_actionEnd_Silence_triggered();

    void on_actionStandard_2_triggered();

    void on_actionStandard_3_triggered();

    void on_actionAktuell_triggered();

    void on_actionLastMarkStandard_triggered();

    void on_actionLastMarkStart_Track_triggered();

    void on_actionLastMarkEnd_Track_triggered();

    void on_actionLastMarkStart_Silence_triggered();

    void on_actionLastMarkEnd_Silence_triggered();

    void on_markTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_actionPrevious_triggered();

    void on_actionNext_triggered();

    void on_actionFirst_triggered();

    void on_actionLast_triggered();

    void on_actionEditDelete_triggered();

    void on_actionEditStandard_triggered();

    void on_actionEditStart_Track_triggered();

    void on_actionEditEnd_Track_triggered();

    void on_actionEditStart_Silence_triggered();

    void on_actionEditEnd_Silence_triggered();

    void on_actionPos1_triggered();

    void on_actionEnd_triggered();

    //void onTitleFieldClicked(int row, int column);

    void on_actionStop_triggered();

    void on_actionPlay_triggered();

    void on_actionZoomOut_triggered();

    void on_actionZoomIn_triggered();

    void on_actionRefresh_triggered();

    void on_actionExpand_WaveForm_toggled(bool arg1);

    void on_btnListDevices_clicked();

    void on_btnStartRec_clicked();

    void on_btnOpen_clicked();


    void on_btnChangePath_clicked();

    void on_actionOptions_triggered();

    //void on_tableTracks_doubleClicked(const QModelIndex &index);

    void on_actionFadeIn_triggered();

    void on_actionFadeOut_triggered();

    void on_actionEditFadeIn_triggered();

    void on_actionEditFadeOut_triggered();

    void on_actionLastFadeIn_triggered();

    void on_actionLastFadeOut_triggered();

    void on_tableTracks_cellDoubleClicked(int row, int column);

    void on_tableTracks_cellChanged(int row, int column);

    void on_actionStandard_4_triggered();

    void on_btbRereadOutput_clicked();

    void on_actionSaveMarks_triggered();

private:
    Ui::MainWindow *ui;
    AudioOutput* audio;
    OptionsDialog* optins;

    QString getPath();
    void open(QString fileName);
    int SampleSize();
    QString MP3File(int title);

    MarkImages mimages;
    int contextmenuNr;
    int contextmenuX;
    Marks *marks;
    QMenu *contextmenu;
    SaveTracks *tracks;
    QTimer timer;
    QString waveprog;
    QString lameprog;
    QString mp3path;
    QStringList lameparams;
    bool initializing;
    int indexstart;
    int indextext;
    int indexmp3;

    bool DoNotNotify;
};

#endif // MAINWINDOW_H
