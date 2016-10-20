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
    ui->openGLWidget->setUpdateFrameCallback([this]() {
        unsigned long time = this->controller->playTime();
        if(!this->controller->isPaused())
            this->ui->horizontalSlider->setValue(time * 100 / this->midiData.totalTime);
        time /= 1000;
        this->ui->lblPlayTime->setText(QString().sprintf("%02d:%02d", time / 60, time % 60));
    });
    controller = new MidiController(midiData, ui->openGLWidget, _parent, this);
    return this;
}

VisualizationDialog::~VisualizationDialog()
{
    if(controller) controller->deleteLater();
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

void VisualizationDialog::on_btnRewind_clicked()
{
    ui->horizontalSlider->setValue(0);
    controller->jumpTo();
}

void VisualizationDialog::on_horizontalSlider_sliderMoved(int position)
{

}

void VisualizationDialog::on_horizontalSlider_sliderPressed()
{
    isJustPause = controller->isPaused();
    controller->pause();
}

void VisualizationDialog::on_horizontalSlider_sliderReleased()
{
    controller->jumpTo(midiData.totalTime * ui->horizontalSlider->value() / 100);
    if(!isJustPause) controller->play();
}
