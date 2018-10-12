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

#ifdef USE_QT
#include <QFile>
#include <QtDebug>

#ifdef USE_QSA
#error QSA is no longer supported
#endif // USE_QSA

#else
#ifdef USE_QTSCRIPT
#error Config error! USE_QTSCRIPT can only be used with USE_QT
#endif // USE_QTSCRIPT
#endif // USE_QT
#ifdef USE_MHASH
#include <mhash.h>
#include <iostream>
#ifndef mutils_word8
#define mutils_word8 unsigned char
#define mutils_word8_cast (void*)
#else
#define mutils_word8_cast
#endif
#endif // USE_MHASH
#ifdef USE_SPIDERMONKEY
#include <jsapi.h>
#include <iostream>
#include <fstream>
#endif // USE_SPIDERMONKEY
#include <math.h>
#include "hasher.h"

using namespace std;

#ifdef USE_SPIDERMONKEY
// Emulate PHP's add_slashes to exscape JavaScript control characters
static string add_slashes(string str);

// This allow us to do a debugging message in Javascript
static JSBool dump(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
// Javascript error reporting
static void printError(JSContext *cx, const char *message, JSErrorReport *report);

static JSFunctionSpec m_functions[] =
{
	{
		"dump",dump, 1, 0, 0
	},
	{
		0, 0, 0, 0, 0
	}
};

static JSClass globalClass =
{
	"Global", 0,
	JS_PropertyStub,  JS_PropertyStub,
	JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub,  JS_FinalizeStub
};
#endif // USE_SPIDERMONKEY

Hasher::Hasher(
#ifdef USE_QT
	QObject *parent
#else
	string path
#endif
)
#ifdef USE_QT
: QObject(parent)//, script(this)
#endif
{
#ifdef USE_QTSCRIPT
	aOK = false;
	loadScript();
#endif // USE_QTSCRIPT
#ifdef USE_MHASH
	hashid tests[] = {MHASH_MD5, MHASH_SHA1, MHASH_RIPEMD160, MHASH_MD4, MHASH_SHA256};
	unsigned char hash[32];
	MHASH td;
	aOK = true;

	for (int i = 0; (i < (int)(sizeof(tests)/sizeof(hashid))) && aOK; i++) {
		td = mhash_init(tests[i]);
		if (td == MHASH_FAILED) {
			// MHASH Failed.
			aOK = false;
		}
		mhash_deinit(td, hash);
	}
#endif // USE_MHASH
#ifdef USE_SPIDERMONKEY
	cx = NULL; // in case the runtime doesn't load
	aOK = false;
	rt = JS_NewRuntime(1000000L);
	if (rt)
	{
		cx = JS_NewContext(rt, 8192);
		if (cx)
		{
			globalObj = JS_NewObject(cx, &globalClass, 0, 0);
			JS_InitStandardClasses(cx, globalObj);
			JS_SetErrorReporter(cx, printError);

			string script, buffer;
			path.append("getHash.js");
			ifstream istr(path.c_str());
			if (istr.is_open())
			{
				while (!istr.fail())
				{
					std::getline(istr, buffer);
					script += buffer + "\n"; // Line break to allow comments
				}
			}
			else
			{
				return;
			}

			JSBool ok = JS_DefineFunctions(cx, globalObj, m_functions);
			if (ok == JS_TRUE)
			{
				jsval rval;
				uintN lineno = 0;
				ok = JS_EvaluateScript(cx, globalObj, script.c_str(), (uintN)script.length(), "getHash.js", lineno, &rval);
				if (ok == JS_TRUE)
					aOK = true;
			}
		}
	}
#endif
}

Hasher::~Hasher(void)
{
#ifdef USE_SPIDERMONKEY
	if (cx) JS_DestroyContext(cx);
	if (rt) JS_DestroyRuntime(rt);
#endif
}

#ifdef USE_QTSCRIPT
void Hasher::loadScript() {
	QFile file(":/getHash.qs");
	QString qs;
	if (aOK) return;
	aOK = false;
	if (file.exists()) {
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qs = file.readAll();
			script.evaluate(qs, "getHash.qs");
			if (script.hasUncaughtException()) {
				// For debugging reasons
				qDebug() << script.uncaughtException().toString() << script.uncaughtExceptionLineNumber();
			}
			// Note: Currently, this is sure to work. Not sure what will be needed to combine with the evaluate line above
			getHashFunc = script.evaluate("function(algo, data, encoding, key, trim, sha256_bug) { return getHash(algo, data, encoding, key, trim, sha256_bug); }");
			getHashScope = script.globalObject();
			aOK = true;
		}
	}
}
#endif // USE_QTSCRIPT

#ifdef USE_QT
QString Hasher::hash(hashType algo, bool usingHMAC, bool trim, QString encoding, QString data, QString key, bool hmac_sha256_bug) {
	if (!aOK) return "ERROR!"; // Just in case
	// data changes to key + data if the hashType is not a HMAC algorithm
	if (!usingHMAC)
	{
		data = key+data;
		key = "";
	}
	QString ret = "ERROR!";

	#ifdef USE_QTSCRIPT
	// TODO: Redo this part
	QString algorithm;
	switch(algo)
	{
	case PWM_MD4:
		algorithm = "md4";
		break;
	case PWM_MD5:
		algorithm = "md5";
		break;
	case PWM_SHA1:
		algorithm = "sha1";
		break;
	case PWM_SHA256:
		algorithm = "sha256";
		break;
	case PWM_RIPEMD160:
		algorithm = "ripemd160";
		break;
	default:
		return "ERROR!"; // Otherwise, an infinite loop may occur
	}
	
	if (usingHMAC) {
		algorithm = "hmac-" + algorithm;
	}
	QScriptValueList args;
	args.append(QScriptValue(&script, algorithm));
	args.append(QScriptValue(&script, data));
	args.append(QScriptValue(&script, encoding));
	args.append(QScriptValue(&script, key));
	args.append(QScriptValue(&script, trim));
	args.append(QScriptValue(&script, hmac_sha256_bug));
	//function getHash(algo, data, encoding, key, trim) {
	ret = getHashFunc.call(getHashScope, args).toString();
	if (ret.isEmpty()) {
		if (script.hasUncaughtException()) {
			// For debugging reasons
			qDebug() << script.uncaughtException().toString();
		}
		ret = "ERROR!";
	}
	#elif USE_MHASH
	MHASH td;
	hashid algorithm;
	mutils_word8 hash[32];
	switch (algo) {
	case PWM_MD4:
		algorithm = MHASH_MD4;
		break;
	case PWM_MD5:
		algorithm = MHASH_MD5;
		break;
	case PWM_SHA1:
		algorithm = MHASH_SHA1;
		break;
	case PWM_SHA256:
		algorithm = MHASH_SHA256;
		break;
	case PWM_RIPEMD160:
		algorithm = MHASH_RIPEMD160;
		break;
	default:
		return "ERROR!";
	}
	if (usingHMAC) {
		td = mhash_hmac_init(algorithm, 
(void*)key.toLatin1().data(), key.size(), 
mhash_get_hash_pblock(algorithm));
		mhash(td, data.toLatin1().data(), data.size());
		mhash_hmac_deinit(td, mutils_word8_cast hash);
	} else {
		td = mhash_init(algorithm);
		mhash(td, (void*)data.toLatin1().data(), data.size());
		mhash_deinit(td, mutils_word8_cast hash);
	}
	ret = rstr2any(hash, mhash_get_block_size(algorithm), encoding, trim);
	#endif // USE_MHASH

	return ret;
}
#endif // USE_QT


string Hasher::hash(hashType algo, bool usingHMAC, bool trim, std::string encoding, std::string data, std::string key, bool hmac_sha256_bug) {
	// data changes to key + data if the hashType is not a HMAC algorithm
	if (!usingHMAC)
	{
		data = key+data;
		key = "";
	}
	string ret = "ERROR!";

#ifdef USE_SPIDERMONKEY
	string algorithm;
	switch(algo)
	{
	case PWM_MD4:
		algorithm = "md4";
		break;
	case PWM_MD5:
		algorithm = "md5";
		break;
	case PWM_SHA1:
		algorithm = "sha1";
		break;
	case PWM_SHA256:
		algorithm = "sha256";
		break;
	case PWM_RIPEMD160:
		algorithm = "ripemd160";
		break;
	default:
		algorithm = "";
	}

	if (!aOK) return "";
	jsval rval;
	uintN lineno = 0;

	string script = "getHash('";
	script += ((usingHMAC)?"hmac-":"");
	script += algorithm;
	script += "', '";
	script += add_slashes(data);
	script += "', '";
	script += add_slashes(encoding);
	script += "', '";
	script += add_slashes(key);
	script += "', ";
	script += ((trim) ? "true" : "false");
	script += ", ";
	script += ((hmac_sha256_bug) ? "true" : "false");
	script += ");";

	JSBool ok = JS_EvaluateScript(cx, globalObj, script.c_str(), (uintN)script.length(), "cFunc_hash()", lineno, &rval);
	if (ok == JS_FALSE) return "";

	JSString *str = JS_ValueToString(cx, rval);
	ret = JS_GetStringBytes(str);
#elif USE_MHASH
	MHASH td;
	unsigned char *hash;
	hashid malgo;
	switch(algo) {
	case PWM_MD4:
		malgo = MHASH_MD4;
		break;
	case PWM_SHA1:
		malgo = MHASH_SHA1;
		break;
	case PWM_SHA256:
		malgo = MHASH_SHA256;
		break;
	case PWM_RIPEMD160:
		malgo = MHASH_RIPEMD160;
		break;
	case PWM_MD5:
	default:
		malgo = MHASH_MD5;
		break;
	}
	if (usingHMAC) {
		// Might offer a custom SHA256 version of these functions at some point
		// Depends on how easy that would be to do
		td = mhash_hmac_init(malgo, (void*)key.c_str(), (int)key.length(), mhash_get_hash_pblock(malgo));
		mhash(td, data.c_str(), (int)data.length());
		if (algo == PWM_SHA256) {
			cout << "WARNING: Due to a bug in Javascript based versions, this is wrong;" << endl;
		}
		hash = (unsigned char*)mhash_hmac_end(td);
	} else {
		td = mhash_init(malgo);
		mhash(td, (void*)data.c_str(), (int)data.length());
		hash = (unsigned char*)mhash_end(td);
	}
	ret = rstr2any(hash, mhash_get_block_size(malgo), encoding, trim);
#endif
	return ret;
}

#ifdef USE_MHASH
#ifdef USE_QT
QString Hasher::rstr2any(unsigned char *input, int length, QString encoding, bool trim)
{
	int divisor;
	int full_length;
	int *dividend;
	int *remainders;
	int remainders_count = 0; // for use with trimming zeros method
	int dividend_length;
	// Counter 
	int i, j;
	QString output = "";

	// Can't handle odd lengths for input correctly with this function
	if (length % 2) return "";
	divisor = (int)encoding.length();
	dividend_length = (int)ceil((double)length/2);
	dividend = new int[dividend_length];
	for (i = 0; i < dividend_length; i++)
	{
		dividend[i] = (((int) input[i*2]) << 8)
			| ((int) input[i*2+1]);
	}

	full_length = (int)ceil((double)length * 8
		/ (log((double)encoding.length()) / log((double)2)));
	remainders = new int[full_length];
	if (trim)
	{
		while(dividend_length > 0)
		{
			int *quotient;
			int quotient_length = 0;
			int qCounter = 0;
			int x = 0;

			quotient = new int[dividend_length];
			for(i = 0; i < dividend_length; i++)
			{
				int q;
				x = (x << 16) + dividend[i];
				q = (int)floor((double)x / divisor);
				x -= q * divisor;
				if (quotient_length > 0 || q > 0)
				{
					quotient[qCounter++] = q;
					quotient_length++;
				}
			}
			remainders[remainders_count++] = x;
			delete[] dividend;
			dividend_length = quotient_length;
			dividend = quotient;
		}
		full_length = remainders_count;
	}
	else
	{
		for (j = 0; j < full_length; j++)
		{
			int *quotient;
			int quotient_length = 0;
			int qCounter = 0;
			int x = 0;

			quotient = new int[dividend_length];
			for(i = 0; i < dividend_length; i++)
			{
				int q;
				x = (x << 16) + dividend[i];
				q = (int)floor((double)x / divisor);
				x -= q * divisor;
				if (quotient_length > 0 || q > 0)
				{
					quotient[qCounter++] = q;
					quotient_length++;
				}
			}
			remainders[j] = x;
			delete[] dividend;
			dividend_length = quotient_length;
			dividend = quotient;
		}
	}

	for (i = full_length - 1; i>=0; i--)
		output.append(encoding[remainders[i]]);

	delete[] dividend;
	delete[] remainders;

	return output;
}
#endif // USE_QT

// TODO: Test trimming leading 'zeros'
string Hasher::rstr2any(unsigned char *input, int length, string encoding, bool trim)
{
	int divisor;
	int full_length;
	int *dividend;
	int *remainders;
	int remainders_count = 0; // for use with trimming zeros method
	int dividend_length;
	// Counter 
	int i, j;
	string output;

	// Can't handle odd lengths for input correctly with this function
	if (length % 2) return "";
	output = "";
	divisor = (int)encoding.length();
	dividend_length = (int)ceil((double)length/2);
	dividend = new int[dividend_length];
	for (i = 0; i < dividend_length; i++)
	{
		dividend[i] = (((int) input[i*2]) << 8)
			| ((int) input[i*2+1]);
	}

	full_length = (int)ceil((double)length * 8
		/ (log((double)encoding.length()) / log((double)2)));
	remainders = new int[full_length];
	if (trim)
	{
		while(dividend_length > 0)
		{
			int *quotient;
			int quotient_length = 0;
			int qCounter = 0;
			int x = 0;

			quotient = new int[dividend_length];
			for(i = 0; i < dividend_length; i++)
			{
				int q;
				x = (x << 16) + dividend[i];
				q = (int)floor((double)x / divisor);
				x -= q * divisor;
				if (quotient_length > 0 || q > 0)
				{
					quotient[qCounter++] = q;
					quotient_length++;
				}
			}
			remainders[remainders_count++] = x;
			delete[] dividend;
			dividend_length = quotient_length;
			dividend = quotient;
		}
		full_length = remainders_count;
	}
	else
	{
		for (j = 0; j < full_length; j++)
		{
			int *quotient;
			int quotient_length = 0;
			int qCounter = 0;
			int x = 0;

			quotient = new int[dividend_length];
			for(i = 0; i < dividend_length; i++)
			{
				int q;
				x = (x << 16) + dividend[i];
				q = (int)floor((double)x / divisor);
				x -= q * divisor;
				if (quotient_length > 0 || q > 0)
				{
					quotient[qCounter++] = q;
					quotient_length++;
				}
			}
			remainders[j] = x;
			delete[] dividend;
			dividend_length = quotient_length;
			dividend = quotient;
		}
	}

	for (i = full_length - 1; i>=0; i--)
		output.append(1, encoding[remainders[i]]);

	delete[] dividend;
	delete[] remainders;

	return output;
}

#endif // USE_MHASH

#ifdef USE_SPIDERMONKEY
// Prevent certain characters from messing up a call
static string add_slashes(string str)
{
	string ret = "";
	char c;
	for (int i = 0; i < (int)str.length(); i++)
	{
		c = str[i];
		switch (c)
		{
		case '\n':
			ret += "\\n";
			break;
		case '\r':
			ret += "\\r";
			break;
		case '\'':
		case '\"':
		case '\\':
			ret += "\\";
		default:
			ret.append(1, c);
		}
	}
	return ret;
}

// JavaScript Core does not provide a function for debugging reason, here's my dump
static JSBool dump(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	uintN i;
	rval = JSVAL_NULL;
	for (i = 0; i < argc; i++)
	{
		if (JSVAL_IS_VOID(argv[i]))
		{
			std::cout << "Void" << std::endl;
		}
		if (JSVAL_IS_STRING(argv[i]))
		{
			JSString *str = JS_ValueToString(cx, argv[i]);
			std::cout << "String: " << JS_GetStringBytes(str) << std::endl;
		}
		if (JSVAL_IS_NULL(argv[i]))
		{
			std::cout << "Null" << std::endl;
		}
		if (JSVAL_IS_INT(argv[i]))
		{
			int value = JSVAL_TO_INT(argv[i]);
			std::cout << "Int: " << value << std::endl;
		}
		if (JSVAL_IS_DOUBLE(argv[i]))
		{
			double value;
			JS_ValueToNumber(cx, argv[i], &value);
			std::cout << "Double: " << value << std::endl;
		}
		if (JSVAL_IS_BOOLEAN(argv[i]))
		{
			int value = JSVAL_TO_BOOLEAN(argv[i]);
			std::cout << "Bool: " << ((value) ? "true" : "false") << std::endl;
		}
	}
	return JS_TRUE;
}

// Print out errors
static void printError(JSContext *cx, const char *message, JSErrorReport *report) {
	cout << "Error in Javascript: " << ((report->filename) ? report->filename : "NULL") <<
		"(" << report->lineno << "): " << message << endl;
	if (report->linebuf) {
		cout << "\t" << report->linebuf << endl;
	}
}

#endif // USE_SPIDERMONKEY
