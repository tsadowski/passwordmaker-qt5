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

#include "passwordgenerator.h"
#include "pwmaccount.h"
#include "shared/hasher.h"
#include "leet.h"

// base93Set "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./"

PasswordGenerator::PasswordGenerator(QObject *parent) : QObject(parent)
{
	// Sets default values
	urlToUse = "";
	charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./";
	username = "";
	modifier = "";
	prefix = "";
	suffix = "";
	error = "";
	passwordLength = -1;
	leetLevel = -1;
	useHMAC = false;
	useTrim = true;
	useHex = false;
	aOK = true;
	algorithm = PWM_MD4;
	useLeet = LEET_NONE;
	HMACBug = true;
	fullAlgorithm = -1;

	// Setups the hasher class
	hasher = new Hasher(this);
	if (!hasher->initialized()) {
		aOK = false;
		error = tr("Error loading hasher class.");
	}

	massChange = false;
}

void PasswordGenerator::setSettings(PWMAccount *s)
{
	massChange = true; // stop the others from emitting settingsChanged()
	setUserName(s->username());
	setUseLeet(s->useLeet());
	setLeetLevel(s->leetLevel());
	setPasswordLength(s->passwordLength());
	setModifier(s->modifier());
	setPrefix(s->passwordPrefix());
	setSuffix(s->passwordSuffix());
	setURL(s->urlToUse());
	setCharacterSet(s->charset());
	setFullAlgorithm(s->hashAlgorithm());
	emit settingChanged();
	massChange = false;
}

void PasswordGenerator::getSettings(PWMAccount *s) {
	s->setUsername(username);
	s->setUseLeet(useLeet);
	s->setLeetLevel(leetLevel);
	s->setPasswordLength(passwordLength);
	s->setModifier(modifier);
	s->setPasswordPrefix(prefix);
	s->setPasswordSuffix(suffix);
	s->setUrlToUse(urlToUse);
	s->setCharset(charset);
	if (fullAlgorithm != -1) {
		s->setHashAlgorithm(fullAlgorithm);
	}
	else {
		// Now for one ugly way to create ONE parameter
		s->setHashAlgorithm((int)algorithm | ((HMACBug) ? PWM_HMAC_BUG : 0) | ((useHMAC) ? PWM_HMAC : 0) | ((useTrim && useHex) ? PWM_V6 : 0));
	}
}

void PasswordGenerator::setAlgorithm(hashType a)
{
	if (a != algorithm) {
		algorithm = a;
		if (!massChange) {
			emit settingChanged();
			setFullAlgorithm(-1);
		}
		emit algorithmChanged(a);
	}
}

void PasswordGenerator::setFullAlgorithm(int a)
{
	if (a != fullAlgorithm) {
		fullAlgorithm = a;
		if (a != -1) {
			bool omc = massChange;
			massChange = true;
			// Do the basic steps
			setAlgorithm((hashType)(a & 0xff));
			if (a & PWM_V6) {
				useHex = true;
				setTrim(false);
			}
			else {
			#ifdef USE_SHA256_BUG
				setHMACBug((bool)(a & PWM_HMAC_BUG));
			#endif //USE_SHA256_BUG
				useHex = false;
				setTrim(true);
			}
			setHMAC((bool)(a & PWM_HMAC));
			massChange = omc;
			if (!massChange) emit settingChanged();
		}
		emit fullAlgorithmChanged(a);
	}
}

void PasswordGenerator::setHMAC(bool h)
{
	if (h != useHMAC) {
		useHMAC = h;
		if (!massChange) {
			setFullAlgorithm(-1);
			emit settingChanged();
		}
		emit HMACChanged(h);
	}
}

void PasswordGenerator::setTrim(bool t)
{
	if (t != useTrim) {
		useTrim = t;
		if (!massChange) {
			emit settingChanged();
			setFullAlgorithm(-1);
		}
		emit trimChanged(t);
	}
}

void PasswordGenerator::setURL(QString url)
{
	if (url != urlToUse) {
		urlToUse = url;
		if (!massChange) emit settingChanged();
		emit URLChanged(url);
	}
}

void PasswordGenerator::setPasswordLength(int l)
{
	if (l != passwordLength) {
		passwordLength = l;
		if (!massChange) emit settingChanged();
		emit passwordLengthChanged(l);
	}
}

void PasswordGenerator::setPasswordLength(QString l)
{
	setPasswordLength(l.toInt());
}

void PasswordGenerator::setCharacterSet(QString c)
{
	if (c.length() > 1 && c != charset) {
		charset = c;
		if (!massChange) emit settingChanged();
		emit characterSetChanged(c);
	}
}

void PasswordGenerator::setUseLeet(leetType l)
{
	if (l != useLeet) {
		useLeet = l;
		if (!massChange) emit settingChanged();
		emit useLeetChanged(l);
	}
}

void PasswordGenerator::setLeetLevel(int l)
{
	if (l != leetLevel && l > -1 && l < 9) {
		leetLevel = l;
		if (!massChange) emit settingChanged();
		emit leetLevelChanged(l);
	}
}

void PasswordGenerator::setUserName(QString u)
{
	if (u != username) {
		username = u;
		if (!massChange) emit settingChanged();
		userNameChanged(u);
	}
}

void PasswordGenerator::setModifier(QString m)
{
	if (m != modifier) {
		modifier = m;
		if (!massChange) emit settingChanged();
		emit modifierChanged(m);
	}
}

void PasswordGenerator::setPrefix(QString p)
{
	if (p != prefix) {
		prefix = p;
		if (!massChange) emit settingChanged();
		emit prefixChanged(p);
	}
}

void PasswordGenerator::setSuffix(QString s)
{
	if (s != suffix) {
		suffix = s;
		if (!massChange) emit settingChanged();
		emit suffixChanged(s);
	}
}

void PasswordGenerator::setHMACBug(bool b)
{
	if (b != HMACBug) {
		HMACBug = b;
		if (!massChange) {
			emit settingChanged();
			setFullAlgorithm(-1);
		}
		emit HMACBugChanged(b);
	}
}

QString PasswordGenerator::generatePassword(QString masterPassword)
{
	if (!aOK) return "";
	QString c, data, password = "";
	int count = 0;
	
	c = (useHex) ? "0123456789abcdef" : charset;
	
	data = urlToUse + username + modifier;
	
	if (charset.length() < 2) {
		error = tr("The minimum number of caharacters for the character set is two.");
		return tr("Character set error!");
	}
	
	if (useLeet == LEET_BEFORE || useLeet == LEET_BOTH) {
		masterPassword = leetConvert(leetLevel, masterPassword);
		data = leetConvert(leetLevel, data);
	}
	
	while (password.length() < passwordLength && count < 1000) {
		if (count) {
			password += leetConvert((useLeet == LEET_AFTER || useLeet == LEET_BOTH) ? leetLevel : -1, hasher->hash(algorithm, useHMAC, useTrim, c, data, QString("%1\n%2").arg(masterPassword).arg(count), HMACBug));
		}
		else {
			password = hasher->hash(algorithm, useHMAC, useTrim, c, data, masterPassword, HMACBug);
			if (useLeet == LEET_AFTER || useLeet == LEET_BOTH)
				password = leetConvert(leetLevel, password);
		}
		count++;
	}
	
	
	password = prefix + password;
	password = password.left(qMax(passwordLength - suffix.length(), 0)) + suffix;
	
	password = password.left(passwordLength);
	
	emit passwordGenerated(password);
	return password;
}
