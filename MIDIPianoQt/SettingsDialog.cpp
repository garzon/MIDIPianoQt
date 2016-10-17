#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(MidiIOManager *handle, QWidget *parent)
	: QWidget(parent), _handle(handle)
{
	ui.setupUi(this);
}

void SettingsDialog::changeIns(){
	unsigned char a=atoi(ui.plainTextEdit->toPlainText().toStdString().c_str());
	_handle->sendMsg(0,a,0xC,0);
}

SettingsDialog::~SettingsDialog()
{

}
