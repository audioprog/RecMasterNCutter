#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

signals:
    void OptionsUpdate();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_tbtnPathsDel_clicked();

    void on_tbtMp3Path_clicked();

    void on_tbtSoxPath_clicked();

    void on_tbtLamePath_clicked();

    void on_tbtAudioProc_clicked();

    void on_btnTextfilepath_clicked();

    void on_btnCDsrc_clicked();

    void on_btnCDdest_clicked();

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
