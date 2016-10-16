#ifndef VISUALIZATIONDIALOG_H
#define VISUALIZATIONDIALOG_H

#include <QDialog>

namespace Ui {
class VisualizationDialog;
}

class VisualizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VisualizationDialog(QWidget *parent = 0);
    ~VisualizationDialog();

private:
    Ui::VisualizationDialog *ui;
};

#endif // VISUALIZATIONDIALOG_H
