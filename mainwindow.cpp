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

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

#include "mainwindow.h"
#include "passwordgenerator.h"
#include "pwmsettings.h"
#include "pwmaccount.h"
#include "pwmabout.h"
#include "accountwindow.h"
#include "optionswindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	QIcon icon;
	QAction *action;
	QMenu *menu;
	QFont font;
	QVariant setting;
	PWMSettings settings(this);
	
	icon.addFile(":images/ring-16x16");
	icon.addFile(":images/ring-24x24");
	icon.addFile(":images/ring-32x32");
	icon.addFile(":images/ring-48x48");
	icon.addFile(":images/ring-64x64");
	icon.addFile(":images/ring-128x128");
	icon.addFile(":images/ring-256x256");
	setWindowIcon(icon);
	setWindowTitle(tr("PasswordMaker Desktop Edition"));
	currentAccount = settings.getDefaults();
	pg = new PasswordGenerator(this);
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(500);
	connect(timer, SIGNAL(timeout()), SLOT(generatePassword()));

	clipBoardTimer = new QTimer(this);
	clipBoardTimer->setSingleShot(true);
	setting = settings.setting("autoClearClipboardSeconds");
	if (setting.isValid() && setting.convert(QVariant::Int)) {
		setting = QVariant(1000*setting.toInt());
	}
	else {
		setting = QVariant(10000);
	}
	clipBoardTimer->setInterval(setting.toInt());
	connect(clipBoardTimer, SIGNAL(timeout()), SLOT(clearClipboard()));
	
	if (!pg->initialized()) {
		QMessageBox::critical(this, tr("PasswordMaker Desktop Edition"),
		tr("Password Generator is unable to load:\n") + pg->getError(),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		close();
	}
	
	createMenu();
	// Tray Icon
	if (QSystemTrayIcon::isSystemTrayAvailable()) {
		trayIcon = new QSystemTrayIcon(icon, this);
		trayIcon->setToolTip(tr("PasswordMaker Desktop Edition"));
		trayIcon->show();
		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
		menu = new QMenu(this);
		
		// Open PasswordMaker
		action = new QAction(tr("&Open PasswordMaker"), this);
		font = action->font();
		font.setBold(true);
		action->setFont(font);
		connect(action, SIGNAL(triggered()), SLOT(show()));
		menu->addAction(action);
		
		menu->addSeparator();
		menu->addAction(findChild<QAction *>("exit"));
		trayIcon->setContextMenu(menu);
	}

	createInterface();
	connect(pg, SIGNAL(settingChanged()), this, SLOT(updateSettings()));
	setSettings();
	
	QRect geo = settings.windowGeometry("MainWindow");
	if (!geo.isNull()) {
		resize(geo.size());
		move(geo.topLeft());
	}
}

void MainWindow::flushSettings() {
	PWMSettings settings(this);
	QRect geo;
	// While there is geometry(), QT suggest handling the size and pos separetly
	// instead. It's stores as a QRect anyway.
	geo.setTopLeft(pos());
	geo.setSize(size());
	settings.saveWindowGeometry("MainWindow", geo);
	settings.setDefaults(currentAccount);
	settings.exportSettings();
}

void MainWindow::closeEvent(QCloseEvent *event) {
	if (trayIcon && !trayIcon->isVisible() || !trayIcon) {
		delete currentAccount;
		currentAccount = 0;
		event->accept();
		flushSettings();
	}
	else {
		hide();
		event->ignore();
	}
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
	switch (reason) {
	case QSystemTrayIcon::DoubleClick:
		show();
		break;
	default:
		break;
	}
}

void MainWindow::calculateUrl(const QString url) {
	QRegExp rx("([^:/]+://)?([^:/]+)([^#]*)");
	if (rx.indexIn(url) != -1) {
		QString endURL;
		
		QStringList domainSegments = rx.cap(2).split(".");
		if (domainSegments.count() < 3) {
			domainSegments.prepend("");
		}
		
		if (currentAccount->urlParts() & PWMAccount::UrlSubdomains) {
			for (int i = 0; i < domainSegments.count()-2; i++) {
				endURL += domainSegments[i];
				if (i+1 < domainSegments.count()-2)
					endURL += ".";
			}
		}
		
		if (currentAccount->urlParts() & PWMAccount::UrlDomain) {
			if (endURL != "" && endURL[endURL.size()-1] != QChar('.'))
				endURL += ".";
			endURL += domainSegments[domainSegments.count()-2] + "." + domainSegments[domainSegments.count()-1];
		}
		
		if (currentAccount->urlParts() & PWMAccount::UrlProtocol) {
			endURL.prepend(rx.cap(1));
		}
		
		if (currentAccount->urlParts() & PWMAccount::UrlOthers) {
			endURL += rx.cap(3);
		}
		
		findChild<QLineEdit *>("urlUsedControl")->setText(endURL);
	}
}

void MainWindow::exportSettings() {
	static QString dir;
	PWMSettings settings;
	// Default to RDF until all other editions support the xml format?
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export settings"), dir, tr("PWM XML (*.xml);;PWM RDF (*.rdf)"));
	if (fileName.isEmpty()) {
		// User hit cancel
		return;
	}
	QFileInfo fileInfo(fileName);
	dir = fileInfo.absolutePath();
	
	// Little inline trick to auto detect rdf formats
	if (!settings.exportSettings(fileName, ((fileInfo.suffix() == "rdf") ? true : false))) {
		QMessageBox::critical(this, tr("PasswordMaker Error"), tr("Error exporting settings to %1.\nCheck for write premissions.").arg(fileName));
		return;
	}
	QMessageBox::information(this, tr("PasswordMaker"), tr("Settings exported to %1.").arg(fileName));
}

void MainWindow::importSettings() {
	static QString dir; // Set default to current dir?
	PWMSettings settings;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import settings"), dir, tr("PWM XML (*.xml);;PWM RDF (*.rdf)"));
	if (fileName.isEmpty()) {
		// User hit cancel
		return;
	}
	QFileInfo fileInfo(fileName);
	if (!fileInfo.isFile()) {
		// Inform the user about a mistake on their part? Not sure if this is possible though
		return;
	}
	dir = fileInfo.absolutePath();
	
	if (QMessageBox::Yes == QMessageBox::question(this, tr("PasswordMaker"),
		tr("All existing settings will be overwritten. ARE YOU SURE YOU WANT TO IMPORT THIS FILE?"),
		QMessageBox::Yes|QMessageBox::No, QMessageBox::No)) {
		if (!settings.importSettings(fileName)) {
			QMessageBox::critical(this, tr("PasswordMaker Error"), tr("Error importing settings from %1.\nPossible it's not a PasswordMaker export file.").arg(fileName));
			return;
		}
		setSettings();
		updateSettings();
		QMessageBox::information(this, tr("PasswordMaker"), tr("Settings imported from %1.").arg(fileName));
	}
}

void MainWindow::setSettings() {
	delete currentAccount;
	PWMSettings settings(this);
	currentAccount = settings.getDefaults();
	pg->setSettings(currentAccount);
}

void MainWindow::preGeneratePassword() {
	PWMSettings settings(this);
	QVariant setting = settings.setting("confirmMaster");
	if (!setting.isValid()) setting = true;
	if (!setting.convert(QVariant::Bool)) setting = true;
	havePassword = false;
	if (masterPasswordControl->text() == "") {
		passwordControl->setText(tr("Please enter a Master Password"));
	}
	else {
		if (!setting.toBool() || masterPasswordControl->text() == masterConfirmControl->text()) {
			passwordControl->setText(tr("Generating..."));
			timer->start();
		}
		else {
			passwordControl->setText(tr("Passwords don't match"));
		}
	}
	passwordControl->setEchoMode(QLineEdit::Normal);
}

void MainWindow::generatePassword() {
	PWMSettings settings(this);
	QVariant setting;
	pg->generatePassword(masterPasswordControl->text());
	setting = settings.setting("maskGenerated");
	if (setting.isValid() && setting.convert(QVariant::Bool)) {
		passwordControl->setEchoMode((setting.toBool()) ? QLineEdit::Password : QLineEdit::Normal);
	}
	havePassword = true;
}

/*void MainWindow::changeLeet()
{
	leetType lt = (leetType)useLeetControl->itemData(useLeetControl->currentIndex()).toInt();
	pg->setLeetLevel(leetLevelControl->itemData(leetLevelControl->currentIndex()).toInt());
	if (lt == LEET_NONE) {
		leetLevelControl->setEnabled(false);
	}
	else {
		leetLevelControl->setEnabled(true);
	}
	pg->setUseLeet(lt);
}*/

/*void MainWindow::changeAlgorithm()
{
	int hash = algoControl->itemData(algoControl->currentIndex()).toInt();
	pg->setAlgorithm((hashType)(hash & 0xff));
	if (hash & PWM_V6) {
		charsetControl->setEnabled(false);
		pg->setCharacterSet("0123456789abcdef");
		pg->setTrim(false);
	}
	else {
	#ifdef USE_SHA256_BUG
		pg->setHMACBug((bool)(hash & PWM_HMAC_BUG));
	#endif //USE_SHA256_BUG
		pg->setCharacterSet(charsetControl->currentText());
		pg->setTrim(true);
		charsetControl->setEnabled(true);
	}
	pg->setHMAC(hash & PWM_HMAC);
}*/

void MainWindow::copyToClipboard() {
	if (!havePassword) return;
	PWMSettings settings(this);
	QVariant setting;
	genPassword = passwordControl->text();
	QApplication::clipboard()->setText(genPassword);
	setting = settings.setting("autoClearClipboard");
	// Set timeout for clearing clipboard
	if (!setting.isValid() || setting.toBool()) {
		clipBoardTimer->start();
	}
}

void MainWindow::clearClipboard() {
	if (QApplication::clipboard()->text() == genPassword) {
		QApplication::clipboard()->clear();
	}
}

void MainWindow::updateSettings() {
	PWMSettings settings(this);
	QVariant setting = settings.setting("autoClearClipboardSeconds");
	if (setting.isValid() && setting.convert(QVariant::Int)) clipBoardTimer->setInterval(1000*setting.toInt());

	setting = settings.setting("maskGenerated");
	if (setting.isValid() && setting.convert(QVariant::Bool)) {
		passwordControl->setEchoMode((setting.toBool()) ? QLineEdit::Password : QLineEdit::Normal);
	}

	// When storing master password, populate it here

	setting = settings.setting("confirmMaster");
	if (!setting.isValid()) setting = true;
	if (setting.convert(QVariant::Bool)) {
		masterConfirmLabel->setVisible(setting.toBool());
		masterConfirmControl->setVisible(setting.toBool());
	}

	setting = settings.setting("hideMaster");
	if (!setting.isValid()) setting = false;
	if (setting.convert(QVariant::Bool)) {
		if (masterPasswordControl->text().isEmpty()) {
			// Can't hide master password control if there's no password
			setting = false;
			settings.setSetting("hideMaster", setting, 1);
			masterPasswordLabel->show();
			masterPasswordControl->show();
		}
		else {
			masterPasswordLabel->setVisible(!setting.toBool());
			masterPasswordControl->setVisible(!setting.toBool());
			masterConfirmLabel->setVisible(!setting.toBool());
			masterConfirmControl->setVisible(!setting.toBool());
		}
	}

	currentAccount->setUrlToUse(findChild<QLineEdit *>("urlUsedControl")->text());
	pg->setURL(findChild<QLineEdit *>("urlUsedControl")->text());
	preGeneratePassword();
}

void MainWindow::createMenu() {
	QAction *action;
	QMenu *menu;
	OptionsWindow *settings = new OptionsWindow(this);
	
	// Actions
	action = new QAction(tr("&Import Settings"), this);
	action->setObjectName("importSettings");
	connect(action, SIGNAL(triggered()), SLOT(importSettings()));
	
	action = new QAction(tr("&Export Settings"), this);
	action->setObjectName("exportSettings");
	connect(action, SIGNAL(triggered()), SLOT(exportSettings()));
	
	action = new QAction(tr("&Quit"), this);
	action->setObjectName("exit");
	//Alt+F4 under Windows is like hitting the Close button, even if we are looking for it, so had to use something else
	action->setShortcut(QKeySequence(tr("Ctrl+Q")));
	connect(action, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(action, SIGNAL(triggered()), SLOT(flushSettings()));
	
	action = new QAction(tr("C&opy to clipboard"), this);
	action->setObjectName("copyClip");
	connect(action, SIGNAL(triggered()), SLOT(copyToClipboard()));
	
	action = new QAction(tr("&Settings"), this);
	action->setObjectName("programSettings");
	action->setShortcut(QKeySequence(tr("Ctrl+P")));
	connect(action, SIGNAL(triggered()), settings, SLOT(exec()));
	connect(settings, SIGNAL(accepted()), SLOT(setSettings()));
	connect(settings, SIGNAL(accepted()), SLOT(flushSettings()));
	//action->setEnabled(false); // Until a settings dialog and system is created
	
	action = new QAction(tr("About Trolltech &QT"), this);
	action->setObjectName("aboutQT");
	connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	
	action = new QAction(tr("&About PasswordMaker Desktop Edition"), this);
	action->setObjectName("aboutPWM");
	PWMAbout *aboutPWMDialog = new PWMAbout(this);
	connect(action, SIGNAL(triggered()), aboutPWMDialog, SLOT(exec()));
	
	// Menu stuff
	
	// File Menu
	menu = menuBar()->addMenu(tr("&File"));
	menu->setObjectName("fileMenu");
	menu->addAction(findChild<QAction *>("importSettings"));
	menu->addAction(findChild<QAction *>("exportSettings"));
	// Do we want to bother with printing settings with this version?
	menu->addSeparator();
	menu->addAction(findChild<QAction *>("exit"));
	
	// Edit Menu
	// Good place for settings menu item?
	menu = menuBar()->addMenu(tr("&Edit"));
	menu->setObjectName("editMenu");
	menu->addAction(findChild<QAction *>("copyClip"));
	menu->addSeparator();
	menu->addAction(findChild<QAction *>("programSettings"));
	
	// Help Menu
	menu = menuBar()->addMenu(tr("&Help"));
	menu->setObjectName("helpMenu");
	menu->addAction(findChild<QAction *>("aboutQT"));
	menu->addAction(findChild<QAction *>("aboutPWM"));
}

void MainWindow::createInterface() {
	PWMSettings settings(this);
	QVariant setting;
	QLabel *label;
	QLineEdit *lineEdit;
	QPushButton *button;
	QHBoxLayout *rowLayout;
	QVBoxLayout *mainLayout = new QVBoxLayout;
	QGridLayout *layout = new QGridLayout;
	int gridRow;
	
	// Widget stuff
	QWidget *centralWidget = new QWidget();
	setCentralWidget(centralWidget);
	
	label = new QLabel(tr("Input URL"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("inputUrlControl");
	label->setBuddy(lineEdit);
	layout->addWidget(label, 0, 0);
	layout->addWidget(lineEdit, 0, 1);
	connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(calculateUrl(QString)));
	
	masterPasswordLabel = new QLabel(tr("Master Password"));
	masterPasswordControl = new QLineEdit();
	masterPasswordLabel->setBuddy(masterPasswordControl);
	masterPasswordControl->setEchoMode(QLineEdit::Password);
	connect(masterPasswordControl, SIGNAL(textChanged(QString)), this, SLOT(preGeneratePassword()));
	layout->addWidget(masterPasswordLabel, 1, 0);
	layout->addWidget(masterPasswordControl, 1, 1);
	
	// Master Password confirmation fields.
	masterConfirmLabel = new QLabel(tr("Confrimation"));
	masterConfirmControl = new QLineEdit();
	masterConfirmLabel->setBuddy(masterConfirmControl);
	masterConfirmControl->setEchoMode(QLineEdit::Password);
	connect(masterConfirmControl, SIGNAL(textChanged(QString)), this, SLOT(preGeneratePassword()));
	layout->addWidget(masterConfirmLabel, 2, 0);
	layout->addWidget(masterConfirmControl, 2, 1);

	// Store Password option
	/* Layout for store password using row 3 */
	gridRow = 4;
	
	// Extras, to be options for being shown later on
	// Move off to change defaults dialog
	/*
	label = new QLabel(tr("Use l33t"));
	useLeetControl = new QComboBox();
	comboBox = new QComboBox();
	comboBox->setObjectName("useLeetControl");
	label->setBuddy(comboBox);
	comboBox->addItem(tr("Not at all"), LEET_NONE);
	comboBox->addItem(tr("Before generating password"), LEET_BEFORE);
	comboBox->addItem(tr("After generating password"), LEET_AFTER);
	comboBox->addItem(tr("Before and after generating password"), LEET_BOTH);
	comboBox->setCurrentIndex(useLeetControl->findData(currentAccount->useLeet()));
	connect(comboBox, SIGNAL(activated(int)), SLOT(changeLeet()));
	
	label = new QLabel(tr("l33t level"));
	comboBox = new QComboBox();
	comboBox->setObjectName("leetLevelControl");
	label->setBuddy(comboBox);
	for (int i = 0; i < 9; i++) {
		comboBox->addItem(QString("%1").arg(i+1), i);
	}
	comboBox->setCurrentIndex(comboBox->findData(currentAccount->leetLevel()));
	connect(comboBox, SIGNAL(activated(int)), SLOT(changeLeet()));
	
	label = new QLabel(tr("Hash algorithm"));
	comboBox = new QComboBox();
	comboBox->setObjectName("algoControl");
	label->setBuddy(comboBox);
	comboBox->addItem("MD4", PWM_MD4);
	comboBox->addItem("HMAC-MD4", PWM_HMAC | PWM_MD4);
	comboBox->addItem("MD5", PWM_MD5);
	comboBox->addItem("MD5 Version 0.6", PWM_MD5 | PWM_V6);
	comboBox->addItem("HMAC-MD5", PWM_HMAC | PWM_MD5);
	comboBox->addItem("HMAC-MD5 Version 0.6", PWM_HMAC | PWM_MD5 | PWM_V6);
	comboBox->addItem("SHA-1", PWM_SHA1);
	comboBox->addItem("HMAC-SHA-1", PWM_HMAC | PWM_SHA1);
	comboBox->addItem("SHA-256", PWM_SHA256);
	comboBox->addItem("HMAC-SHA-256", PWM_HMAC | PWM_SHA256);
	#ifdef USE_SHA256_BUG
	comboBox->addItem("HMAC-SHA-256 Version 1.5.2", PWM_HMAC | PWM_SHA256 | PWM_HMAC_BUG);
	#endif //USE_SHA256_BUG
	comboBox->addItem("RIPEMD-160", PWM_RIPEMD160);
	comboBox->addItem("HMAC-RIPEMD-160", PWM_HMAC | PWM_RIPEMD160);
	comboBox->setCurrentIndex(algoControl->findData(currentAccount->hashAlgorithm()));
	connect(comboBox, SIGNAL(activated(int)), SLOT(changeAlgorithm()));
	
	label = new QLabel(tr("Length of generated password"));
	spinBox = new QSpinBox();
	spinBox->setObjectName("passwordLengthControl");
	label->setBuddy(spinBox);
	spinBox->setMinimum(1);
	spinBox->setValue(currentAccount->passwordLength());
	connect(spinBox, SIGNAL(valueChanged(int)), pg, SLOT(setPasswordLength(int)));
	
	label = new QLabel(tr("Username"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("usernameControl");
	label->setBuddy(lineEdit);
	lineEdit->setText(currentAccount->username());
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setUserName(QString)));
	
	label = new QLabel(tr("Modifier"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("modifierControl");
	label->setBuddy(lineEdit);
	lineEdit->setText(currentAccount->modifier());
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setModifier(QString)));
	
	label = new QLabel(tr("Characters"));
	comboBox = new QComboBox();
	comboBox->setObjectName("charsetControl");
	label->setBuddy(comboBox);
	comboBox->addItem("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./");
	comboBox->addItem("0123456789abcdef");
	comboBox->addItem("0123456789");
	comboBox->addItem("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	comboBox->addItem("`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./");
	// charsetControl->addItem(""); // Random will come later
	comboBox->setCurrentIndex(0);
	comboBox->setEditable(true);
	comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
	comboBox->setInsertPolicy(QComboBox::NoInsert);
	comboBox->setEditText(currentAccount->charset());
	comboBox->lineEdit()->setSelection(0, 0);
	connect(comboBox, SIGNAL(editTextChanged(QString)), pg, SLOT(setCharacterSet(QString)));
	
	label = new QLabel(tr("Password prefix"));
	prefixControl = new QLineEdit();
	lineEdit->setObjectName("prefixControl");
	label->setBuddy(lineEdit);
	lineEdit->setText(currentAccount->passwordPrefix());
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setPrefix(QString)));
	
	label = new QLabel(tr("Password suffix"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("suffixControl");
	label->setBuddy(lineEdit);
	lineEdit->setText(currentAccount->passwordSuffix());
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setSuffix(QString)));
	*/
	
	label = new QLabel(tr("Using text"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("urlUsedControl");
	label->setBuddy(lineEdit);
	layout->addWidget(label, gridRow, 0);
	layout->addWidget(lineEdit, gridRow++, 1);
	lineEdit->setText(currentAccount->urlToUse());
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setURL(QString)));
	
	label = new QLabel(tr("Generated password"));
	passwordControl = new QLineEdit(tr("Please enter a Master Password"));
	label->setBuddy(passwordControl);
	layout->addWidget(label, gridRow, 0);
	layout->addWidget(passwordControl, gridRow++, 1);
	passwordControl->setReadOnly(true);
	setting = settings.setting("maskGenerated");
	if (setting.isValid() && setting.convert(QVariant::Bool) && setting.toBool()) {
		passwordControl->setEchoMode(QLineEdit::Password);
	}
	connect(pg, SIGNAL(passwordGenerated(QString)), passwordControl, SLOT(setText(QString)));
	
	mainLayout->addLayout(layout);
	
	button = new QPushButton(tr("&Copy Password to Clipboard"));
	mainLayout->addWidget(button);
	connect(button, SIGNAL(clicked()), findChild<QAction *>("copyClip"), SIGNAL(triggered()));
	
	rowLayout = new QHBoxLayout;
		AccountWindow *accountWindow = new AccountWindow(this);
		button = new QPushButton(tr("Edit Defaults"));
		rowLayout->addWidget(button);
		connect(button, SIGNAL(clicked()), accountWindow, SLOT(exec()));
		connect(button, SIGNAL(clicked()), SLOT(flushSettings()));
		connect(accountWindow, SIGNAL(finished(int)), SLOT(setSettings()));
		connect(accountWindow, SIGNAL(finished(int)), SLOT(flushSettings()));
		
		button = new QPushButton(tr("Edit Accounts"));
		rowLayout->addWidget(button);
		button->setEnabled(false);
	mainLayout->addLayout(rowLayout);
	mainLayout->addStretch(1);
	
	rowLayout = new QHBoxLayout;
		rowLayout->addStretch(1);
		
		button = new QPushButton(tr("Close"));
		rowLayout->addWidget(button);
		connect(button, SIGNAL(clicked()), SLOT(hide()));
	mainLayout->addLayout(rowLayout);
	
	centralWidget->setLayout(mainLayout);
}
