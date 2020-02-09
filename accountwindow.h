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

#ifndef ACCOUNTWINDOW_H
#define ACCOUNTWINDOW_H

#include <QDialog>
#include "shared/pwm_common.h"

class PasswordGenerator;
class PWMAccount;

class AccountWindow : public QDialog {
	Q_OBJECT
public:
	AccountWindow(/* Type */QWidget *parent = 0);

public slots:
	virtual void accept();
	virtual void reject();
	void exec(QString id = "");
	
private slots:
	void urlPartChanged();
	void updateUseLeet(leetType l);
	void updateLeetLevel(int l);
	void changeLeet();
	void updateAlgorithm(int a);
	void changeAlgorithm();

private: // Functions
	void createInterface();
	QWidget *createUrlTab();
	QWidget *createExtendedTab();

private: // Varibles
	PasswordGenerator *pg;
	PWMAccount *currentAccount;
	QString inputURL;
};

#endif // ACCOUNTWINDOW_H
