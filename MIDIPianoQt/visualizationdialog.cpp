#include "visualizationdialog.h"
#include "ui_visualizationdialog.h"

VisualizationDialog::VisualizationDialog(QString midiFilePath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VisualizationDialog),
    midiData()
{
    ui->setupUi(this);
    MidiFileReader fileReader(midiFilePath.toLocal8Bit());
    try {
        fileReader.load(midiData);
    } catch(const char* ex) {
        QMessageBox::warning(this, QString("Error - Cannot open midi file"), QString::fromStdString(ex));
        midiData.header.trackCount = 0;
    }

    controller = new MidiController(midiData);
}

VisualizationDialog::~VisualizationDialog()
{
    delete controller;
    delete ui;
}
