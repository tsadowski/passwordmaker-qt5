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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
#include <QMainWindow>

class PasswordGenerator;
class QLabel;
class QLineEdit;
class QComboBox;
class QTimer;
class PWMAccount;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);

private slots:
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void calculateUrl(const QString url);
	void setSettings();
	void preGeneratePassword();
	void generatePassword();
	void copyToClipboard();
	void clearClipboard();
	void updateSettings();
	void exportSettings();
	void importSettings();
	void flushSettings();

protected:
	virtual void closeEvent(QCloseEvent *event);

private: // Functions
	void createMenu();
	void createInterface();

private: // Varibles
	// Need to be active at all times
	bool generatePasswords;
	PWMAccount *currentAccount;
	PasswordGenerator *pg;
	QSystemTrayIcon *trayIcon;
	
	// Master Password stuff. Used for easy hiding (and confirmation)
	QLabel *masterPasswordLabel;
	QLineEdit *masterPasswordControl;
	QLabel *masterConfirmLabel;
	QLineEdit *masterConfirmControl;
	
	QLineEdit *passwordControl;
	QTimer *timer;
	QTimer *clipBoardTimer;
	QString genPassword;
	bool havePassword;
};

#endif // MAINWINDOW_H
