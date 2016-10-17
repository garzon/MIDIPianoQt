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
    explicit VisualizationDialog(QWidget *parent = 0);
    VisualizationDialog *loadMidiFile(QString midiFilePath);
    ~VisualizationDialog();
    MidiData midiData;
    MidiController *controller;
private slots:
    void on_pushButton_clicked();

    void on_btnSwitchView_clicked();

private:
    Ui::VisualizationDialog *ui;
};

#endif // VISUALIZATIONDIALOG_H
