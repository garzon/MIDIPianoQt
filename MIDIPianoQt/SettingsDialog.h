#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include "ui_SettingsDialog.h"

#include <MidiIOManager.h>

class SettingsDialog : public QWidget
{
	Q_OBJECT

public:
    SettingsDialog(MidiIOManager *handle, QWidget *parent = 0);
	~SettingsDialog();

private slots:
	void changeIns();

private:
	Ui::SettingsDialog ui;
    MidiIOManager *_handle;
};

#endif // SETTINGSDIALOG_H
