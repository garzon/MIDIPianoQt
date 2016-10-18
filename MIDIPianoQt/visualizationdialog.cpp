#include "visualizationdialog.h"
#include "ui_visualizationdialog.h"

VisualizationDialog::VisualizationDialog(QMainWindow *parent) :
    QDialog(parent),
    _parent(parent),
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

    ui->openGLWidget->loadMidiData(midiData);
    controller = new MidiController(midiData, ui->openGLWidget, _parent, this);
    return this;
}

VisualizationDialog::~VisualizationDialog()
{
    // if(controller) delete controller; QObject will handle this
    delete ui;
}

void VisualizationDialog::on_pushButton_clicked()
{
    controller->play();
}

void VisualizationDialog::on_btnSwitchView_clicked()
{
    ui->openGLWidget->switchView();
}

void VisualizationDialog::on_pushButton_2_clicked()
{
    controller->pause();
}
