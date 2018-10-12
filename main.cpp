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

#include <QApplication>
#include <QTranslator>
#include <QLocale>

#include "mainwindow.h"
#include "pwmsettings.h"

int main(int argc, char **argv) {
	int ret;
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(resources);
	
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name());
	app.installTranslator(&qtTranslator);
	
	QTranslator pwmTranslator;
	pwmTranslator.load("passwordmaker_" + QLocale::system().name());
	app.installTranslator(&pwmTranslator);
	
	QApplication::addLibraryPath("."); // For SQLite plugin
	QApplication::setApplicationName("PasswordMaker Desktop Edition");
	QApplication::setOrganizationDomain("passwordmaker.org");
	QApplication::setOrganizationName("PasswordMaker");
	
	PWMSettings *settings = new PWMSettings();

	MainWindow window;
	
	window.show();
	ret = app.exec();
	settings->exportSettings();
	delete settings;
	return ret;
}
