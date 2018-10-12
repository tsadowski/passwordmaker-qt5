/**
 * PasswordMaker - Creates and manages passwords
 * Copyright (C) 2006 Eric H. Jung and LeahScape, Inc.
 * http://passwordmaker.org/
 * grimholtz@yahoo.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Written by Miquel Burns <miquelfire@gmail.com> and Eric H. Jung
*/

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QFontMetrics>

#include "optionswindow.h"
#include "pwmsettings.h"

OptionsWindow::OptionsWindow(QWidget *parent) : QDialog(parent) {
	createInterface();
}

void OptionsWindow::accept() {
	PWMSettings settings(this);
	QVariant setting;
	// Save settings
	setting = QVariant(findChild<QCheckBox *>("maskGenerated")->isChecked());
	settings.setSetting("maskGenerated", setting, 1);
	setting = QVariant(findChild<QCheckBox *>("hideMaster")->isChecked());
	settings.setSetting("hideMaster", setting, 1);
	setting = QVariant(findChild<QCheckBox *>("confirmMaster")->isChecked());
	settings.setSetting("confirmMaster", setting, 1);
	setting = QVariant(findChild<QCheckBox *>("autoClear")->isChecked());
	settings.setSetting("autoClearClipboard", setting, 1);
	setting = QVariant(findChild<QSpinBox *>("autoClearTime")->value());
	settings.setSetting("autoClearClipboardSeconds", setting, 1);
	QDialog::accept();
}

void OptionsWindow::exec() {
	QVariant setting;
	PWMSettings settings(this);
	// Load settings, here only because an import can change things
	setting = settings.setting("maskGenerated");
	if (setting.isValid() && setting.convert(QVariant::Bool)) {
		findChild<QCheckBox *>("maskGenerated")->setChecked(setting.toBool());
	}
	setting = settings.setting("hideMaster");
	if (setting.isValid() && setting.convert(QVariant::Bool)) {
		findChild<QCheckBox *>("hideMaster")->setChecked(setting.toBool());
	}
	setting = settings.setting("confirmMaster");
	if (setting.isValid() && setting.convert(QVariant::Bool)) {
		findChild<QCheckBox *>("confirmMaster")->setChecked(setting.toBool());
	}
	setting = settings.setting("autoClearClipboard");
	if (setting.isValid() && setting.convert(QVariant::Bool)) {
		findChild<QCheckBox *>("autoClear")->setChecked(setting.toBool());
	}
	setting = settings.setting("autoClearClipboardSeconds");
	if (setting.isValid() && setting.convert(QVariant::Int)) {
		findChild<QSpinBox *>("autoClearTime")->setValue(setting.toInt());
	}
	QDialog::exec();
}

void OptionsWindow::createInterface() {
	QVBoxLayout *layout = new QVBoxLayout;
	QWidget *tab;

	QTabWidget *tabs = new QTabWidget();
	
	// Global Settings tab
	tab = createGlobalSettingsTab();
	tabs->addTab(tab, tr("Glo&bal Settings"));
	
	// Upload/Download tab
	/*tab = createUploadTab();
	tabs->addTab(tab, tr("Up&load/Download"));
	
	// Special Domains tab
	tab = createDomainTab();
	tabs->addTab(tab, tr("&Special Domains"));*/
	
	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	
	layout->addWidget(tabs);
	layout->addWidget(buttons);
	setLayout(layout);
}

QWidget *OptionsWindow::createGlobalSettingsTab() {
	QWidget *tab = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;

	QCheckBox *checkBox;
	QSpinBox *spinBox;
	QLabel *label;
	QHBoxLayout *hLayout = new QHBoxLayout;
	
	// Defaults to false
	checkBox = new QCheckBox(tr("Mask generated password with Asterisks"));
	checkBox->setObjectName("maskGenerated");
	checkBox->setChecked(false);
	layout->addWidget(checkBox);
	
	// Defaults to false
	checkBox = new QCheckBox(tr("Hide master password field (number of asterisks)"));
	checkBox->setObjectName("hideMaster");
	checkBox->setChecked(false);
	layout->addWidget(checkBox);
	
	// Defaults to true
	checkBox = new QCheckBox(tr("Confirm master password by typing it twice instead of once"));
	checkBox->setObjectName("confirmMaster");
	checkBox->setChecked(true);
	layout->addWidget(checkBox);

	// Defaults to true
	checkBox = new QCheckBox(tr("Auto-clear clipboard"));
	checkBox->setObjectName("autoClear");
	checkBox->setChecked(true);
	hLayout->addWidget(checkBox);

	// Defaults to 10
	spinBox = new QSpinBox;
	spinBox->setObjectName("autoClearTime");
	spinBox->setMaximum(999);
	spinBox->setMinimum(1);
	spinBox->setValue(10);
	spinBox->setEnabled(true);
	connect(checkBox, SIGNAL(toggled(bool)), spinBox, SLOT(setEnabled(bool)));
	hLayout->addWidget(spinBox);

	label = new QLabel(tr("seconds after copying it there"));
	hLayout->addWidget(label);
	hLayout->addStretch(1);

	layout->addLayout(hLayout);
	
	tab->setLayout(layout);
	return tab;
}

// TODO: Create this tab
QWidget *OptionsWindow::createUploadTab() {
	QWidget *tab = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;

	tab->setLayout(layout);
	return tab;
}

// TODO: Create this tab
QWidget *OptionsWindow::createDomainTab() {
	QWidget *tab = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;

	tab->setLayout(layout);
	return tab;
}
