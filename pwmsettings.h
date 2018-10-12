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

#ifndef PWMSETTINGS
#define PWMSETTINGS
#include <QObject>
#include <QRect>
#include <QList>

class PWMAccount;
class PWMGroup;

/**
 * This class is used to set/read the settings for desktop edition.
 * Internally, everything is stored in a SQLite memeory database
 */
class PWMSettings : public QObject {
	Q_OBJECT
public:
	PWMSettings(QObject *parent = 0);
	
	/**
	 * Import and export settings from XML files.
	 * Using empty string for xmlPath will default to pwmsettings.xml in the
	 * same directory of the real config file, for backuping up the settings to
	 * a backup type of file
	 * May support exporting rdf as well
	 */
	bool exportSettings(QString xmlPath = "", bool rdf = false);
	bool importSettings(QString xmlPath = "");
	
	/**
	 * This controls the size and positions of the windows.
	 */
	QRect windowGeometry(QString window);
	void saveWindowGeometry(QString window, QRect &geo);
	
	/**
	 * Random settings that don't have it's own function yet
	 * level is where to store the setting, 0 means nowhere, 1 is global, 2 is local, and 3 is global and local
	 */
	QVariant setting(QString name);
	void setSetting(QString name, QVariant &value, int level = 1);
	
	/**
	 * Gets a list of groups
	 * Not needed until accounts are used
	 */
	QList<PWMGroup *> getGroups();
	
	/**
	 * Gets a list of accounts for a group
	 * Not needed until accounts are used
	 */
	QList<PWMAccount *> getAccounts(PWMGroup *parent);
	
	/**
	 * Get an specific account. Empty string is default account.
	 * May not be needed for first beta release
	 */
	PWMAccount *getAccount(QString id = "");
	//void saveAccount(QString id = "", PWMAccount &account);
	
	/**
	 * Gets the default settings
	 */
	PWMAccount *getDefaults();
	
	/**
	 * Saves default settings
	 */
	void setDefaults(PWMAccount *defaults);

private:
	QString path; // Path to config directory we're using

};

#endif // PWMSETTINGS
