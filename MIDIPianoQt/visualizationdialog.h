#ifndef VISUALIZATIONDIALOG_H
#define VISUALIZATIONDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QTimer>

#include "midifilereader.h"
#include "midicontroller.h"
#include "midiopenglwidget.h"

namespace Ui {
class VisualizationDialog;
}

class VisualizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VisualizationDialog(QMainWindow *parent = 0);
    VisualizationDialog *loadMidiFile(QString midiFilePath);
    ~VisualizationDialog();
    MidiData midiData;
    MidiController *controller;
private slots:
    void on_pushButton_clicked();

    void on_btnSwitchView_clicked();

    void on_pushButton_2_clicked();

private:
    QMainWindow *_parent;
    Ui::VisualizationDialog *ui;
};

#endif // VISUALIZATIONDIALOG_H
