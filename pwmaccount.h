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

#ifndef PWMACCOUNT
#define PWMACCOUNT

#include <QObject>
#include <QString>

#include "shared/pwm_common.h"

class PWMAccount : public QObject {
	Q_OBJECT
public:
	enum UrlPart {
		UrlNone = 0x00,
		UrlProtocol = 0x01,
		UrlSubdomains = 0x02,
		UrlDomain = 0x04,
		UrlOthers = 0x08
	};
	Q_DECLARE_FLAGS(UrlParts, UrlPart);
	
	enum AccountType {
		DefaultAccount,
		NormalAccount,
		GroupAccount
	};
	
	PWMAccount(QString id = "", QObject *parent = 0);
	
	PWMAccount(const PWMAccount &other);
	void operator=(PWMAccount other);
	
	QString id() { return _id; };
	AccountType accountType() { return _accountType; };
	void setAccountType(AccountType at, QString id = "");
	QString name() { return _name; };
	QString notes() { return _notes; };
	UrlParts urlParts() { return _urlParts; };
	QString urlToUse() { return _urlToUse; };
	QString username() { return _username; };
	leetType useLeet() { return _useLeet; };
	int leetLevel() { return _leetLevel; };
	int hashAlgorithm() { return _hashAlgorithm; };
	int passwordLength() { return _passwordLength; };
	QString charset() { return _charset; };
	QString modifier() { return _modifier; };
	QString passwordPrefix() { return _passwordPrefix; };
	QString passwordSuffix() { return _passwordSuffix; };
	QString mpwSalt() { return _mpwSalt; };
	QString mpwHash() { return _mpwHash; };
	
public slots:
	void setName(const QString &name);
	void setNotes(const QString &notes);
	void setUrlParts(const UrlParts parts);
	void setUrlToUse(const QString &urlToUse);
	void setUsername(const QString &username);
	void setUseLeet(const leetType useLeet);
	void setLeetLevel(const int leetLevel);
	void setHashAlgorithm(const int hashAlgorithm);
	void setPasswordLength(const int passwordLength);
	void setCharset(const QString &charset);
	void setModifier(const QString &modifier);
	void setPasswordPrefix(const QString &passwordPrefix);
	void setPasswordSuffix(const QString &passwordSuffix);
	void setMpwSalt(const QString &salt);
	void setMpwHash(const QString &hash);
	void setMpwHash(const QString &salt, const QString &mpw);
	
private:
	QString _id;
	AccountType _accountType;
	QString _name;
	QString _notes;
	UrlParts _urlParts;
	QString _urlToUse;
	QString _username;
	leetType _useLeet;
	int _leetLevel; // 1-9
	int _hashAlgorithm;
	int _passwordLength;
	QString _charset;
	QString _modifier;
	QString _passwordPrefix;
	QString _passwordSuffix;
	QString _mpwSalt;
	QString _mpwHash;
};
 
Q_DECLARE_OPERATORS_FOR_FLAGS(PWMAccount::UrlParts);

#endif // PWMACCOUNT
