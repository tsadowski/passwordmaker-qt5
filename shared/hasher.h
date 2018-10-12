/**
 * PasswordMaker - Creates and manages passwords
 * Copyright (C) 2005-2006 Eric H. Jung and LeahScape, Inc.
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

#ifndef HASHER_H
#define HASHER_H

#ifdef USE_QSA
#error QSA is no longer supported
#endif // USE_QSA
#include "pwm_common.h"
#ifdef USE_QT

#include <QObject>
#include <QString>

#ifdef USE_QTSCRIPT
#include <QScriptEngine>
#include <QScriptValue>
#endif // USE_QTSCRIPT

#else
#ifdef USE_QTSCRIPT
#error WHOOPS! QtScript can only be used with QT.
#error Check the build system for the correct defines
#endif

#ifdef USE_SPIDERMONKEY
#include <jsapi.h>
#endif

#endif
#include <string>

class Hasher
#ifdef USE_QT
: public QObject
#endif
{
public:
	Hasher(
#ifdef USE_QT
		QObject *parent
#else
		std::string path = ""
#endif
	);
	~Hasher(void);
	
#ifdef USE_QT
	QString hash(hashType algo, bool usingHMAC, bool trim, QString encoding, QString data, QString key, bool hmac_sha256_bug = true);
#endif
	std::string hash(hashType algo, bool usingHMAC, bool trim, std::string encoding, std::string data, std::string key, bool hmac_sha256_bug = true);
	
	bool initialized() { return aOK; };
	
private:
	bool aOK;
#ifdef USE_SPIDERMONKEY
	JSRuntime *rt;
	JSContext *cx;
	JSObject *globalObj;
#endif // USE_SPIDERMONKEY
#ifdef USE_QTSCRIPT
	QScriptEngine script;
	QScriptValue getHashFunc;
	QScriptValue getHashScope;
	void loadScript();
#endif // USE_QTSCRIPT
#ifdef USE_MHASH
#ifdef USE_QT
	QString rstr2any(unsigned char *input, int length, QString encoding, bool trim = true);
#endif
	std::string rstr2any(unsigned char *input, int length, std::string encoding, bool trim = true);
#endif // USE_MHASH
};

#endif //HASHER_H
