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

#include "pwmaccount.h"
#include "shared/hasher.h"

static QString hashSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./";

PWMAccount::PWMAccount(QString id, QObject *parent)
: QObject(parent), _id(id) {
	_accountType = DefaultAccount;
	_name = "Defaults";
	_notes = "Default settings for URLs not elsewhere in this list";
	_urlParts = UrlDomain;
	_urlToUse = "";
	_username = "";
	_useLeet = LEET_NONE;
	_leetLevel = 0;
	_hashAlgorithm = PWM_MD5;
	_passwordLength = 8;
	_charset = hashSet;
	_modifier = "";
	_passwordPrefix = "";
	_passwordSuffix = "";
	_mpwSalt = "";
	_mpwHash = "";
}

PWMAccount::PWMAccount(const PWMAccount &other) {
	_accountType = other._accountType;
	_name = other._name;
	_notes = other._notes;
	_urlParts = other._urlParts;
	_urlToUse = other._urlToUse;
	_username = other._username;
	_useLeet = other._useLeet;
	_leetLevel = other._leetLevel;
	_hashAlgorithm = other._hashAlgorithm;
	_passwordLength = other._passwordLength;
	_charset = other._charset;
	_modifier = other._modifier;
	_passwordPrefix = other._passwordPrefix;
	_passwordSuffix = other._passwordSuffix;
	_mpwSalt = other._mpwSalt;
	_mpwHash = other._mpwHash;
}

void PWMAccount::operator=(PWMAccount other) {
	_accountType = other._accountType;
	_name = other._name;
	_notes = other._notes;
	_urlParts = other._urlParts;
	_urlToUse = other._urlToUse;
	_username = other._username;
	_useLeet = other._useLeet;
	_leetLevel = other._leetLevel;
	_hashAlgorithm = other._hashAlgorithm;
	_passwordLength = other._passwordLength;
	_charset = other._charset;
	_modifier = other._modifier;
	_passwordPrefix = other._passwordPrefix;
	_passwordSuffix = other._passwordSuffix;
	_mpwSalt = other._mpwSalt;
	_mpwHash = other._mpwHash;
}


void PWMAccount::setName(const QString &name) {
	_name = name;
	// Strip newlines
	_name.remove(QChar('\n'));
	_name.remove(QChar('\r'));
}

void PWMAccount::setNotes(const QString &notes) {
	_notes = notes;
}

void PWMAccount::setUrlParts(const UrlParts parts) {
	_urlParts = parts;
}

void PWMAccount::setUrlToUse(const QString &urlToUse) {
	_urlToUse = urlToUse;
}

void PWMAccount::setUsername(const QString &username) {
	_username = username;
}

void PWMAccount::setUseLeet(const leetType useLeet) {
	_useLeet = useLeet;
}

void PWMAccount::setLeetLevel(const int leetLevel) {
	if (leetLevel > 0 && leetLevel < 10)
		_leetLevel = leetLevel;
}

void PWMAccount::setHashAlgorithm(const int hashAlgorithm) {
	int newHash;
	
	// Check to make sure it's a valid algorithm
	newHash = hashAlgorithm & 0xff;
	if (!newHash) {
		qDebug("Not a hash algorithm.");
		return; // Not even an algorithm at all!
	}
	switch (newHash) {
	case PWM_MD4:
	case PWM_MD5:
	case PWM_SHA1:
	case PWM_RIPEMD160:
	case PWM_SHA256:
		break;
	default:
		// Unknown by this class
		qDebug("Unknown hash algorithm.");
		return;
	}
	
	// Check the other flags
	newHash = hashAlgorithm & ~0xff;
	#ifndef USE_SHA256_BUG
	if (newHash & PWM_HMAC_BUG) {
		if ((hashAlgorithm & (PWM_HMAC | PWM_SHA256)) != (PWM_HMAC | PWM_SHA256)) {
			qDebug("HMAC bug is only with HMAC-SHA256.");
			return; // Only SHA256 had a bug in the JS version
		}
		newHash &= ~PWM_HMAC_BUG;
	}
	#endif // USE_SHA256_BUG
	if ((newHash & ~(PWM_HMAC | PWM_V6 | PWM_HMAC_BUG))) {
		qDebug("Unknown flag being used.");
		return; // Not a valid flag being used
	}
	
	_hashAlgorithm = hashAlgorithm;
}

void PWMAccount::setPasswordLength(const int passwordLength) {
	_passwordLength = passwordLength;
}

void PWMAccount::setCharset(const QString &charset) {
	/*
	* TODO: Double check this
	*/
	if (charset.size() > 1)
		_charset = charset;
}

void PWMAccount::setModifier(const QString &modifier) {
	_modifier = modifier;
}

void PWMAccount::setPasswordPrefix(const QString &passwordPrefix) {
	_passwordPrefix = passwordPrefix;
}

void PWMAccount::setPasswordSuffix(const QString &passwordSuffix) {
	_passwordSuffix = passwordSuffix;
}

void PWMAccount::setMpwSalt(const QString &salt) {
	_mpwHash = ""; // Assume this is no loner valid
	_mpwSalt = salt;
}

void PWMAccount::setMpwHash(const QString &hash) {
	_mpwHash = hash;
}

void PWMAccount::setMpwHash(const QString &salt, const QString &mpw) {
	Hasher hasher(this);
	if (!hasher.initialized()) return; // Can't use this function if this class can't use Hasher
	setMpwSalt(salt);
	_mpwHash = hasher.hash(PWM_SHA256, true, true, hashSet, mpw, salt, false); // Last param MUST be false
}
