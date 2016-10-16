#include "visualizationdialog.h"
#include "ui_visualizationdialog.h"

VisualizationDialog::VisualizationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VisualizationDialog)
{
    ui->setupUi(this);
}

VisualizationDialog::~VisualizationDialog()
{
    delete ui;
}
