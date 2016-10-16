#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include "ui_SettingsDialog.h"

#include <myMIDI.h>

class SettingsDialog : public QWidget
{
	Q_OBJECT

public:
	SettingsDialog(myMIDI *handle, QWidget *parent = 0);
	~SettingsDialog();

private slots:
	void changeIns();

private:
	Ui::SettingsDialog ui;
	myMIDI *_handle;
};

#endif // SETTINGSDIALOG_H
