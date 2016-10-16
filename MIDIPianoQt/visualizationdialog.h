#ifndef VISUALIZATIONDIALOG_H
#define VISUALIZATIONDIALOG_H

#include <QDialog>
#include <QMessageBox>

#include "midifilereader.h"
#include "midicontroller.h"

namespace Ui {
class VisualizationDialog;
}

class VisualizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VisualizationDialog(QString midiFilePath, QWidget *parent = 0);
    ~VisualizationDialog();
    MidiData midiData;
    MidiController *controller;
private:
    Ui::VisualizationDialog *ui;
};

#endif // VISUALIZATIONDIALOG_H
