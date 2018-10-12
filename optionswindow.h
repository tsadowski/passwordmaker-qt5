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

#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QDialog>

class OptionsWindow : public QDialog {
	Q_OBJECT
public:
	OptionsWindow(QWidget *parent = 0);
public slots:
	virtual void accept();
	void exec();
	
private slots:
private: // Functions
	void createInterface();
	QWidget *createGlobalSettingsTab();
	QWidget *createUploadTab();
	QWidget *createDomainTab();
};

#endif // OPTIONSWINDOW_H