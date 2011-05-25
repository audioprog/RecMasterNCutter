#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "marks.h"
#include "markimages.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void NewLength(int newLen, int windowlength);
    void PosChanged(int diff, bool absolut = false);
    void ShowContextMenu(QPoint pos, int x, int mark);
    void MainPosChanged(int newPos);
    void OverviewLength(int newLen, int windowlength);
    void OverviewPosChanged(int diff, bool absolut = false);
    void OverviewMarkChanged(int newPos);

private:
    Ui::MainWindow *ui;

private slots:
    void MarksChanged();

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

private:
    MarkImages mimages;
    int contextmenuNr;
    int contextmenuX;
    Marks *marks;
    QMenu *contextmenu;
};

#endif // MAINWINDOW_H
