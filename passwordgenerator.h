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

#ifndef PASSWORDMAKER_H
#define PASSWORDMAKER_H

#include <QObject>
#include "shared/pwm_common.h"

class Hasher;
class PWMAccount;

class PasswordGenerator : public QObject {
	Q_OBJECT
public:
	PasswordGenerator(QObject *parent);

	/**
	 * Returns true if everything is ready to start generating passwords.
	 * If false, you can use the getError function to tell the user what may be
	 * wrong. Using setPath(QString) may fix the error if it was because the
	 * getHash.qs file was not found however.
	 */
	bool initialized() { return aOK; };
	
	/**
	 * Returns the last error
	 */
	QString getError() { return error; };
	
	/**
	 * Clears the last error
	 */
	void clearError() { error = ""; };
	void getSettings(PWMAccount *s);
	
public slots:
	/**
	 * Calling this function will generate a password based on the current
	 * settings and the master password supplied.
	 */
	QString generatePassword(QString masterPassword);
	
	/**
	 * Pass an instance of AccountSettings to autopopulate all the settings for
	 * an account. Other functions are for use when changing directly from the
	 * GUI itself
	 */
	void setSettings(PWMAccount *s);
	
	/**
	 * The following functions set of functions set the values of the parameters
	 * used to generate a password. Normally, setSettings() would set these
	 * values anyway.
	 */
	void setAlgorithm(hashType a);
	void setFullAlgorithm(int a); // Direct support for combobox data
	void setHMAC(bool h);
	void setTrim(bool t);
	void setURL(QString url);
	void setPasswordLength(int l);
	void setPasswordLength(QString l);
	void setCharacterSet(QString c);
	void setUseLeet(leetType l);
	void setLeetLevel(int l); // Range is 0-8
	void setUserName(QString u);
	void setModifier(QString m);
	void setPrefix(QString p);
	void setSuffix(QString s);
	void setHMACBug(bool b);
	
signals:
	/**
	 * Emitted whenever a setting is changed. setSettings() will only cause this
	 * to be emitted once
	 */
	void settingChanged();
	
	/**
	 * The following are emitted when the setting has been changed, so that GUI
	 * objects can be adjusted
	 */
	void algorithmChanged(hashType a);
	void fullAlgorithmChanged(int a);
	void HMACChanged(bool h);
	void trimChanged(bool t);
	void URLChanged(QString url);
	void passwordLengthChanged(int l);
	void characterSetChanged(QString c);
	void useLeetChanged(leetType l);
	void leetLevelChanged(int l);
	void userNameChanged(QString u);
	void modifierChanged(QString m);
	void prefixChanged(QString p);
	void suffixChanged(QString s);
	void HMACBugChanged(bool b);
	
	/**
	 * Emitted whenever generatePassword is called
	 */
	void passwordGenerated(QString password);

private:
	QString urlToUse, charset, username, modifier, prefix, suffix, error;
	int passwordLength, leetLevel, fullAlgorithm;
	bool useHMAC, useTrim, HMACBug, useHex, aOK;
	hashType algorithm;
	leetType useLeet;

	bool massChange;
	
	// Custom classes being used
	Hasher *hasher;
	
};

#endif //PASSWORDMAKER_H
