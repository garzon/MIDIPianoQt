#include "visualizationdialog.h"
#include "ui_visualizationdialog.h"

VisualizationDialog::VisualizationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VisualizationDialog),
    midiData(),
    controller(NULL)
{
    ui->setupUi(this);
}

VisualizationDialog *VisualizationDialog::loadMidiFile(QString midiFilePath) {
    if(controller) throw "VisualizationDialog::loadMidiFile - Midi file is already loaded.";

    MidiFileReader fileReader(midiFilePath.toLocal8Bit());
    try {
        fileReader.load(midiData);
    } catch(const char* ex) {
        QMessageBox::warning(this, QString("Error - Cannot open midi file"), QString::fromStdString(ex));
        close();
        return NULL;
    }

    controller = new MidiController(midiData);
    ui->openGLWidget->loadMidiData(midiData);
    return this;
}

VisualizationDialog::~VisualizationDialog()
{
    if(controller) delete controller;
    delete ui;
}

void VisualizationDialog::on_pushButton_clicked()
{
    ui->openGLWidget->update();
}

void VisualizationDialog::on_btnSwitchView_clicked()
{
    ui->openGLWidget->switchView();
}
