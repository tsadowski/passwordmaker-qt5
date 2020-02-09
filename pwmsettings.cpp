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
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QtXml> // Until I know exactly which classes are needed

#include <QMessageBox>

#include "pwmsettings.h"
#include "pwmaccount.h"
#include "shared/pwm_common.h"

PWMSettings::PWMSettings(QObject *parent) : QObject(parent) {
	#ifdef USE_DATA_DIR
	path = USE_DATA_DIR;
	// Create directory if needed
	if (!QFile::exists(path)) {
		QDir dir;
		dir.mkdir(path);
	}
	path +="/settings.xml";
	#else
	QSettings settingsObj(QSettings::IniFormat, QSettings::UserScope,
	#ifdef Q_WS_MAC
	QApplication::organizationDomain(),
	#else
	QApplication::organizationName(),
	#endif
	QApplication::applicationName(), this
	);
	QFileInfo fi(settingsObj.fileName());
	
	path = fi.absolutePath() + "/settings.xml";
	#endif
	
	if (!QSqlDatabase::database().isValid()) {
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
		if (!db.isValid()) {
			// Try QSQLITE2 just in case the QT library was built with it instead of 3.x (Needs testing)
			db = QSqlDatabase::addDatabase("QSQLITE2");
			if (!db.isValid()) {
				QMessageBox::critical(NULL, "PWMSettings Constructor: Error", "Error creating internal database. May not have QT SQLite plugin.");
				QApplication::exit(1);
				return;
			}
		}
		db.setDatabaseName(":memory:");
		if (!db.open()) {
			QMessageBox::critical(NULL, "PWMSettings Constructor: Error", "Error opening internal database. Ask for a special build to debug this error.");
			QApplication::exit(1);
			return;
		}
		
		// Create tables
		QSqlQuery queries;
		// Defaults table, only one row used
		// Defaults table has MPWHash stuff to go inline with the Firefox Edition
			// Note: sizes are not used with SQLite
		queries.prepare("create table pwm_defaults (\
			useLeet int,\
			leetLevel int,\
			hashAlgorithm int,\
			passwordLength int,\
			username varchar(255),\
			modifier varchar(255),\
			charset varchar(255),\
			pwprefix varchar(255),\
			pwsuffix varchar(255),\
			uriparts int,\
			mpwSalt varchar(32),\
			mpwHash varchar(64)\
		)");
		if (!queries.exec()) {
			QMessageBox::critical(NULL, "PWMSettings Constructor: Error", "Error creating table \"pwm_defaults\".");
			QApplication::exit(1);
			return;
		}
		
		// Main settings table
		queries.prepare("create table settings ( name varchar(255), value varchar(255), level int(1) )");
		if (!queries.exec()) {
			QMessageBox::critical(NULL, "PWMSettings Constructor: Error", "Error creating table \"settings\".");
			QApplication::exit(1);
			return;
		}
		
		if (!importSettings()) {
			PWMAccount *defaults = new PWMAccount;
			setDefaults(defaults);
			delete defaults;
		}
	}
}

bool PWMSettings::importSettings(QString xmlPath) {
	QDomDocument doc("");
	QSqlQuery query;
	QString errorMsg;
	int errorRow, errorCol;
	bool handledDefaults = false;
	bool readAccounts = false;
	bool readDomains = false;
	bool readSettings = false;
	
	if (xmlPath.isEmpty()) {
		xmlPath = path;
	}
	QFile file(xmlPath);
	if (!file.exists()) {
		return false;
	}
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	if (!doc.setContent(&file, true, &errorMsg)) {
		file.close();
		return false;
	}
	file.close();
	QDomElement docElem = doc.documentElement();
	
	if (docElem.tagName() == "passwordmaker") {
		// TODO: Reject encrypted XML files, for now
		QDomNode treetop = docElem.firstChild();
		while (!treetop.isNull()) {
			QDomElement treeleaf = treetop.toElement();
			if (!treeleaf.isNull()) {
				if (!readAccounts && treeleaf.tagName() == "accounts") {
					QDomNodeList defaults = treeleaf.elementsByTagName("default");
					if (defaults.size()) {
						QString name, description, username, modifier, charset, prefix, suffix, temp;
						leetType useLeet;
						int hashAlgorithm = PWM_MD5;
						int leetLevel, passwordLength;
						bool num_ok;
						PWMAccount::UrlParts urlParts;
						QDomElement def = defaults.item(0).toElement();
						temp = def.attribute("whereLeet");
						// Convert temp to leetType
						useLeet = LEET_NONE;
						if (temp == "before-hashing") {
							useLeet = LEET_BEFORE;
						}
						if (temp == "after-hashing") {
							useLeet = LEET_AFTER;
						}
						if (temp == "both") {
							useLeet = LEET_BOTH;
						}
						leetLevel = def.attribute("leetLevel").toInt(&num_ok);
						temp = def.attribute("hashAlgorithm");
						// Convert temp to hashType
						if (temp.contains("md4")) {
							hashAlgorithm = PWM_MD4;
						}
						if (temp.contains("md5")) {
							hashAlgorithm = PWM_MD5;
						}
						if (temp.contains("sha1")) {
							hashAlgorithm = PWM_SHA1;
						}
						if (temp.contains("rmd160")) {
							hashAlgorithm = PWM_RIPEMD160;
						}
						if (temp.contains("sha256")) {
							hashAlgorithm = PWM_SHA256;
						}
						if (temp.contains("hmac-")) {
							#ifdef USE_SHA256_BUG
							if (hashAlgorithm == PWM_SHA256)
								hashAlgorithm |= (temp.contains("-fixed")) ? 0 : PWM_HMAC_BUG;
							#endif
							hashAlgorithm |= PWM_HMAC;
						}
						if (temp.contains("-v0.6")) {
							hashAlgorithm |= PWM_V6;
						}
						passwordLength = def.attribute("passwordLength").toInt(&num_ok);
						username = def.attribute("username");
						modifier = def.attribute("modifier");
						charset = def.attribute("charset");
						prefix = def.attribute("prefix");
						suffix = def.attribute("suffix");
						if (def.attribute("protocol").toLower() == "true") {
							urlParts |= PWMAccount::UrlProtocol;
						}
						if (def.attribute("subdomain").toLower() == "true") {
							urlParts |= PWMAccount::UrlSubdomains;
						}
						if (def.attribute("domain").toLower() == "true") {
							urlParts |= PWMAccount::UrlDomain;
						}
						if (def.attribute("path").toLower() == "true") {
							urlParts |= PWMAccount::UrlOthers;
						}
						query.exec("delete pwm_defaults");
						query.prepare("insert into pwm_defaults \
						values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
						query.addBindValue(useLeet);
						query.addBindValue(leetLevel);
						
						query.addBindValue(hashAlgorithm);
						query.addBindValue(passwordLength);
						query.addBindValue(username);
						query.addBindValue(modifier);
						
						query.addBindValue(charset);
						query.addBindValue(prefix);
						query.addBindValue(suffix);
						query.addBindValue((int)urlParts);
						
						// For Master Password hash stuff
						query.addBindValue(QString(""));
						query.addBindValue(QString(""));
						
						query.exec();
						handledDefaults = true;
					}
					else {
						// No defaults defined? Odd... Just skip this tag then.
						continue;
					}
					// Handle other accounts
					// Need to finalize the new XML format a little more before handling account support
					readAccounts = true;
				}
				if (!readDomains && treeleaf.tagName() == "domains") {
					// Handle special domains
				}
				if ((!readSettings) && treeleaf.tagName() == "settings") {
					// Local settings
					QDomElement setting;
					QDomNode branch;
					QDomNodeList settings = treeleaf.elementsByTagName("programsettings");
					branch = settings.item(0);
					while (!branch.isNull()) {
						if (branch.toElement().attribute("prog") == "desktop-qt") {
							settings = branch.toElement().elementsByTagName("setting");
							branch = settings.item(0);
							while (!branch.isNull()) {
								int level = 1;
								QString name;
								// Read settings for this level
								setting = branch.toElement();
								name = setting.attribute("name");
								query.prepare("select level from settings where name = ?");
								query.addBindValue(name);
								if (query.exec() && query.first()) {
									level = query.value(0).toInt() | 1;
									query.prepare("delete from settings where name = ?");
									query.addBindValue(name);
									query.exec();
								}
								query.prepare("insert into settings (name, value, level) values (?, ?, ?)");
								query.addBindValue(name);
								query.addBindValue(setting.attribute("value"));
								query.addBindValue(level);
								query.exec();
								branch = branch.nextSibling();
							}
							break;
						}
						branch = branch.nextSibling();
					}
					// Global settings
					settings = treeleaf.elementsByTagName("globalsettings");
					branch = settings.item(0);
					while (!branch.isNull()) {
						settings = branch.toElement().elementsByTagName("setting");
						branch = settings.item(0);
						while (!branch.isNull()) {
							int level = 2;
							QString name;
							// Read settings for this level
							setting = branch.toElement();
							name = setting.attribute("name");
							query.prepare("select level from settings where name = ?");
							query.addBindValue(name);
							if (query.exec() && query.first()) {
								level = query.value(0).toInt() | 2;
								query.prepare("delete from settings where name = ?");
								query.addBindValue(name);
								query.exec();
							}
							query.prepare("insert into settings (name, value, level) values (?, ?, ?)");
							query.addBindValue(name);
							query.addBindValue(setting.attribute("value"));
							query.addBindValue(level);
							query.exec();
							branch = branch.nextSibling();
						}
					}
				}
			}
			treetop = treetop.nextSibling();
		}
		// Must have at least the default account.
		// Chances are, the user wiped out the contents of the file or at least the defaults tag
		if (!handledDefaults) {
			PWMAccount *defaults = new PWMAccount;
			setDefaults(defaults);
			delete defaults;
		}
		return true;
	}
	else {
		QString rdfNS("http://www.w3.org/1999/02/22-rdf-syntax-ns#");
		QString pwmNS("http://passwordmaker.mozdev.org/rdf#");
		if (docElem.tagName() == "RDF") {
			QDomNodeList rdfSeq, rdfDes;
			bool handledAccounts = false;
			bool handledDefaults = false;
			rdfDes = docElem.elementsByTagNameNS(rdfNS, "Description");
			rdfSeq = docElem.elementsByTagNameNS(rdfNS, "Seq"); // For when Account and/or special domain support is added
			for (int i = 0; i < rdfDes.size(); ++i) {
				QDomElement des = rdfDes.item(i).toElement();
				/**
					Default account only handling
					To be replaced when complete account support is done
				*/
				if (des.attributeNS(rdfNS, "about") == "http://passwordmaker.mozdev.org/defaults") {
					PWMAccount *defaults = new PWMAccount;
					PWMAccount::UrlParts urlParts;
					leetType useLeet;
					int hashAlgorithm = PWM_MD5;
					QString temp;

					if (des.attributeNS(pwmNS, "protocolCB").toLower() == "true") {
						urlParts |= PWMAccount::UrlProtocol;
					}
					if (des.attributeNS(pwmNS, "subdomainCB").toLower() == "true") {
						urlParts |= PWMAccount::UrlSubdomains;
					}
					if (des.attributeNS(pwmNS, "domainCB").toLower() == "true") {
						urlParts |= PWMAccount::UrlDomain;
					}
					if (des.attributeNS(pwmNS, "pathCB").toLower() == "true") {
						urlParts |= PWMAccount::UrlOthers;
					}
					defaults->setUrlParts(urlParts);
					defaults->setPasswordPrefix(des.attributeNS(pwmNS, "prefix"));
					defaults->setPasswordSuffix(des.attributeNS(pwmNS, "suffix"));
					defaults->setLeetLevel(des.attributeNS(pwmNS, "leetLevelLB").toInt() - 1);
					defaults->setPasswordLength(des.attributeNS(pwmNS, "passwordLength").toInt());
					defaults->setUsername(des.attributeNS(pwmNS, "usernameTB"));
					defaults->setModifier(des.attributeNS(pwmNS, "counter"));
					defaults->setCharset(des.attributeNS(pwmNS, "charset"));
					temp = des.attributeNS(pwmNS, "hashAlgorithmLB");
					if (temp.contains("md4")) {
						hashAlgorithm = PWM_MD4;
					}
					if (temp.contains("md5")) {
						hashAlgorithm = PWM_MD5;
					}
					if (temp.contains("sha1")) {
						hashAlgorithm = PWM_SHA1;
					}
					if (temp.contains("rmd160")) {
						hashAlgorithm = PWM_RIPEMD160;
					}
					if (temp.contains("sha256")) {
						hashAlgorithm = PWM_SHA256;
					}
					if (temp.contains("hmac-")) {
						#ifdef USE_SHA256_BUG
						if (hashAlgorithm == PWM_SHA256)
							hashAlgorithm |= (temp.contains("-fixed")) ? 0 : PWM_HMAC_BUG;
						#endif
						hashAlgorithm |= PWM_HMAC;
					}
					if (temp.contains("-v0.6")) {
						hashAlgorithm |= PWM_V6;
					}
					defaults->setHashAlgorithm(hashAlgorithm);
					temp = des.attributeNS(pwmNS, "whereLeetLB");
					useLeet = LEET_NONE;
					if (temp == "before-hashing") {
						useLeet = LEET_BEFORE;
					}
					if (temp == "after-hashing") {
						useLeet = LEET_AFTER;
					}
					if (temp == "both") {
						useLeet = LEET_BOTH;
					}
					defaults->setUseLeet(useLeet);
					setDefaults(defaults);
					delete defaults;
					handledDefaults = true;
					/*
					NS24:selectedTabIndex="2" /> // May not support here, will need feedback on this
					*/
				}
				/**
					End default account only handling
				*/
				if (des.attributeNS(rdfNS, "about") == "http://passwordmaker.mozdev.org/globalSettings") {
					// Import settings that would fit the global setting space
					// TODO: setup importing as each function is implimented
					/*
					maskMasterPassword - mask generated password (Eh?!?)
					hideMasterPasswordField - hides master password field(s)
					confirmMPW - Confirms master password
					autoClearClipboard - Autoclear clipboard
					autoClearClipboardSeconds - clear clipboard after how many seconds?
					useMultipleMasterPasswords - If multiple Master Password hashes are being used. If account support is not used, only used to know if we need to look at default setting or not
					mpwSalt2 - Salt for MPW hash (2 because of logic bug in a beta of 1.7 Firefox edition, and to allow hash function to work). Used as the HMAC-SHA256 key
					mpwHash2 - Hash of MPW
					*/
				}
			}
			if (!handledDefaults) {
				PWMAccount *defaults = new PWMAccount;
				setDefaults(defaults);
				delete defaults;
			}
			return true; // Not supporting RDF files just yet. Will before first beta
		}
		qDebug() << "Unknown XML file format";
	}
	return false;
}

bool PWMSettings::exportSettings(QString xmlPath, bool rdf) {
	QDomDocument doc("");
	QSqlQuery query;
	
	if (xmlPath.isEmpty()) {
		xmlPath = path;
	}
	QFile file(xmlPath);
	if (!file.exists()) {
		// Create enough to get at least the default
		doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));
		if (rdf) {
			// Creates the root element, with the namespaces
			QDomElement root = doc.createElement("RDF:RDF");
			root.setAttribute("xmlns:RDF", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
			root.setAttribute("xmlns:pwmrdf", "http://passwordmaker.mozdev.org/rdf#");
			doc.appendChild(root);
			
			// Settings
			QDomElement des = doc.createElement("RDF:Description");
			des.setAttribute("RDF:about", "http://passwordmaker.mozdev.org/globalSettings");
			root.appendChild(des);
			
			// Defaults
			des = doc.createElement("RDF:Description");
			des.setAttribute("RDF:about", "http://passwordmaker.mozdev.org/defaults");
			des.setAttribute("pwmrdf:name", "Defaults");
			des.setAttribute("pwmrdf:description", "Default settings for URLs not elsewhere in this list");
			root.appendChild(des);
			
			// Account tree for Firefox Extension to not gag on our RDF, Might not be needing this however
			QDomElement seq = doc.createElement("RDF:Seq");
			seq.setAttribute("RDF:about", "http://passwordmaker.mozdev.org/accounts");
			root.appendChild(seq);
			
			QDomElement seqLi = doc.createElement("RDF:li");
			seqLi.setAttribute("RDF:resource", "http://passwordmaker.mozdev.org/defaults");
			seq.appendChild(seqLi);
		}
		else {
			QDomElement root = doc.createElement("passwordmaker");
			doc.appendChild(root);
			
			QDomElement accountRoot = doc.createElement("accounts");
			root.appendChild(accountRoot);
			
			QDomElement defaults = doc.createElement("default");
			accountRoot.appendChild(defaults);
			
			QDomElement settings = doc.createElement("settings");
			root.appendChild(settings);
			
			QDomElement globalSettings = doc.createElement("globalsettings");
			settings.appendChild(globalSettings);
			
			QDomElement programSettings = doc.createElement("programsettings");
			settings.appendChild(programSettings);
			programSettings.setAttribute("prog", "desktop-qt");
		}
	}
	else {
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			// Can't read the file. How the heck are we expected to write to it?
			return false;
		}
		if (!doc.setContent(&file)) {
			file.close();
			return false;
		}
		file.close();
	}
	QDomElement docElem = doc.documentElement();	

	if (rdf) {
		// Note, this is nothing but the prefix, QT makes a huge mess of the file when using it's namespace functions on write out for some reason.
		// Need to look into a way to insure a clean xml output with QT's namespace support
		QString rdfNS, pwmNS;
		QDomNamedNodeMap attributes = docElem.attributes();
		for (int i = 0; i < attributes.size(); ++i) {
			QDomAttr attr = attributes.item(i).toAttr();
			if (attr.value() == "http://www.w3.org/1999/02/22-rdf-syntax-ns#") {
				if (attr.name() != "xmlns" && attr.name().startsWith("xmlns:"))
					rdfNS = attr.name().remove("xmlns:") + ":";
			}
			if (attr.value() == "http://passwordmaker.mozdev.org/rdf#") {
				if (attr.name() != "xmlns" && attr.name().startsWith("xmlns:"))
					pwmNS = attr.name().remove("xmlns:") + ":";
			}
		}
		
		if (docElem.tagName() != rdfNS+"RDF") {
			// Something is not right
			qDebug() << "Whoops";
			return false;
		}
		
		QDomElement defaults, settings; // For when they're found, keep a copy, as RDF elements are not in a predictable order anyway
		QDomNodeList des = docElem.elementsByTagName(rdfNS + "Description");
		for (int i = 0; i < des.size(); ++i) {
			if (des.item(i).toElement().attribute(rdfNS + "about") == "http://passwordmaker.mozdev.org/defaults") {
				defaults = des.item(i).toElement();
			}
			if (des.item(i).toElement().attribute(rdfNS + "about") == "http://passwordmaker.mozdev.org/globalSettings") {
				settings = des.item(i).toElement();
			}
		}
		
		// Load default (and later, accounts)
		if (defaults.isNull()) {
			qDebug() << "Default part of RDF not found. Aborting.";
			return false;
		}
		query.exec("select useleet, leetlevel,\
			hashalgorithm, passwordlength, username, modifier, charset,\
			pwprefix, pwsuffix, uriparts from pwm_defaults");
		if (!query.first()) {
			QMessageBox::critical(NULL, "PWMSettings Export: Error", "Error with getting defaults for export.");
			return false;
		}
		switch(query.value(0).toInt()) {
		case LEET_BEFORE:
			defaults.setAttribute(pwmNS + "whereLeetLB", "before-hashing");
			break;
		case LEET_AFTER:
			defaults.setAttribute(pwmNS + "whereLeetLB", "after-hashing");
			break;
		case LEET_BOTH:
			defaults.setAttribute(pwmNS + "whereLeetLB", "both");
			break;
		case LEET_NONE:
		default:
			defaults.setAttribute(pwmNS + "whereLeetLB", "off");
			break;
		}
		defaults.setAttribute(pwmNS + "leetLevelLB", query.value(1).toInt() + 1);
		QString hashAlgo;
		int hash = query.value(2).toInt();
		if (hash & PWM_HMAC) {
			hashAlgo.append("hmac-");
		}
		switch (hash & 0xff) {
		case PWM_MD4:
			hashAlgo.append("md4");
			break;
		case PWM_MD5:
			hashAlgo.append("md5");
			break;
		case PWM_SHA1:
			hashAlgo.append("sha1");
			break;
		case PWM_RIPEMD160:
			hashAlgo.append("rmd160");
			break;
		case PWM_SHA256:
			hashAlgo.append("sha256");
			if (hash & PWM_HMAC && !(hash & PWM_HMAC_BUG))
				hashAlgo.append("-fixed");
			break;
		default:
			return false;
		}
		if (query.value(2).toInt() & PWM_V6) {
			hashAlgo.append("-v0.6");
		}
		defaults.setAttribute(pwmNS + "hashAlgorithmLB", hashAlgo);
		defaults.setAttribute(pwmNS + "passwordLength", query.value(3).toString());
		defaults.setAttribute(pwmNS + "usernameTB", query.value(4).toString());
		defaults.setAttribute(pwmNS + "counter", query.value(5).toString()); // Modifier
		defaults.setAttribute(pwmNS + "charset", query.value(6).toString());
		defaults.setAttribute(pwmNS + "prefix", query.value(7).toString());
		defaults.setAttribute(pwmNS + "suffix", query.value(8).toString());
		int uriparts = query.value(9).toInt();
		defaults.setAttribute(pwmNS + "protocolCB", (uriparts & PWMAccount::UrlProtocol) ? "true" : "false");
		defaults.setAttribute(pwmNS + "subdomainCB", (uriparts & PWMAccount::UrlSubdomains) ? "true" : "false");
		defaults.setAttribute(pwmNS + "domainCB", (uriparts & PWMAccount::UrlDomain) ? "true" : "false");
		defaults.setAttribute(pwmNS + "pathCB", (uriparts & PWMAccount::UrlOthers) ? "true" : "false");
		
		// Updated shared settings
		/*
			maskMasterPassword - mask generated Password (Eh?!?)
			hideMasterPasswordField - hides master password field(s)
			confirmMPW - Confirms master password
			autoClearClipboard - Autoclear clipboard
			autoClearClipboardSeconds - clear clipboard after how many seconds?
		*/
		file.setFileName(xmlPath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			out << doc.toString() << "\n";
			out.flush();
			file.close();
			return true;
		}
	}
	else {
		QDomElement accounts, def, settings, globalSettings, localSettings;
		if (docElem.tagName() != "passwordmaker") {
			// Not in the new format, ask the user if they're sure about this
			return false;
		}
		// TODO: Reject encrypted XML files
		// Find the default node and point to it
		QDomNode treetop = docElem.firstChild();
		while (!treetop.isNull()) {
			QDomElement treeleaf = treetop.toElement();
			if (!treeleaf.isNull()) {
				if (treeleaf.tagName() == "accounts") {
					accounts = treeleaf;
					QDomNodeList leaves = treeleaf.elementsByTagName("default");
					if (leaves.size()) {
						def = leaves.item(0).toElement();
					}
					else {
						def = doc.createElement("default");
						accounts.appendChild(def);
					}
					break;
				}
			}
			treetop = treetop.nextSibling();
		}
		if (accounts.isNull()) {
			accounts = doc.createElement("accounts");
			docElem.appendChild(accounts);
		}
		if (def.isNull()) {
			def = doc.createElement("default");
			accounts.appendChild(def);
		}
		
		// Handle defaults
		if (!query.exec("select useleet, leetlevel,\
			hashalgorithm, passwordlength, username, modifier, charset,\
			pwprefix, pwsuffix, uriparts from pwm_defaults")) {
				QMessageBox::critical(NULL, "PWMSettings Export: Error", "Error with query of defaults.");
				return false;
		}
		if (!query.first()) {
			QMessageBox::critical(NULL, "PWMSettings Export: Error", "Error with getting defaults for export.");
			return false;
		}
		switch(query.value(0).toInt()) {
			case LEET_BEFORE:
				def.setAttribute("whereLeet", "before-hashing");
				break;
			case LEET_AFTER:
				def.setAttribute("whereLeet", "after-hashing");
				break;
			case LEET_BOTH:
				def.setAttribute("whereLeet", "both");
				break;
			case LEET_NONE:
			default:
				def.setAttribute("whereLeet", "off");
				break;
		}
		def.setAttribute("leetLevel", query.value(1).toString());
		QString hashAlgo;
		int hash = query.value(2).toInt();
		if (hash & PWM_HMAC) {
			hashAlgo.append("hmac-");
		}
		switch (hash & 0xff) {
		case PWM_MD4:
			hashAlgo.append("md4");
			break;
		case PWM_MD5:
			hashAlgo.append("md5");
			break;
		case PWM_SHA1:
			hashAlgo.append("sha1");
			break;
		case PWM_RIPEMD160:
			hashAlgo.append("rmd160");
			break;
		case PWM_SHA256:
			hashAlgo.append("sha256");
			if (hash & PWM_HMAC && !(hash & PWM_HMAC_BUG))
				hashAlgo.append("-fixed");
			break;
		default:
			return false;

		}
		if (query.value(2).toInt() & PWM_V6) {
			hashAlgo.append("-v0.6");
		}
		def.setAttribute("hashAlgorithm", hashAlgo);
		def.setAttribute("passwordLength", query.value(3).toString());
		def.setAttribute("username", query.value(4).toString());
		def.setAttribute("modifier", query.value(5).toString());
		def.setAttribute("charset", query.value(6).toString());
		def.setAttribute("prefix", query.value(7).toString());
		def.setAttribute("suffix", query.value(8).toString());
		int uriparts = query.value(9).toInt();
		def.setAttribute("protocol", (uriparts & PWMAccount::UrlProtocol) ? "true" : "false");
		def.setAttribute("subdomain", (uriparts & PWMAccount::UrlSubdomains) ? "true" : "false");
		def.setAttribute("domain", (uriparts & PWMAccount::UrlDomain) ? "true" : "false");
		def.setAttribute("path", (uriparts & PWMAccount::UrlOthers) ? "true" : "false");
		// TODO: Need to finalize the new XML format a little more before handling account support
		
		// TODO: Handle Special Domains
		
		// Handle Settings
		treetop = docElem.firstChild();
		while (!treetop.isNull()) {
			QDomElement treeleaf = treetop.toElement();
			if (!treeleaf.isNull()) {
				if (treeleaf.tagName() == "settings") {
					settings = treeleaf;
					QDomNodeList leaves = treeleaf.elementsByTagName("globalsettings");
					if (leaves.size()) {
						globalSettings = leaves.item(0).toElement();
					}
					else {
						globalSettings = doc.createElement("globalsettings");
						settings.appendChild(globalSettings);
					}
					leaves = treeleaf.elementsByTagName("programsettings");
					if (leaves.size()) {
						localSettings = leaves.item(0).toElement();
						while (!localSettings.isNull()) {
							if (localSettings.attribute("prog") == "desktop-qt") {
								break;
							}
							localSettings = localSettings.nextSibling().toElement();
						}
					}
					else {
						localSettings = doc.createElement("programsettings");
						settings.appendChild(localSettings);
					}
					break;
				}
			}
			
			treetop = treetop.nextSibling();
		}
		if (settings.isNull()) {
			settings = doc.createElement("settings");
			docElem.appendChild(settings);
		}
		
		if (globalSettings.isNull()) {
			globalSettings = doc.createElement("globalsettings");
			settings.appendChild(globalSettings);
		}
		
		if (localSettings.isNull()) {
			localSettings = doc.createElement("programsettings");
			localSettings.setAttribute("prog", "desktop-qt");
			settings.appendChild(localSettings);
		}
		QDomElement setting, nullElement;
		// Handle Global Settings
		query.exec("select name, value from settings where level & 1");
		while (query.next()) {
			treetop = globalSettings.firstChild();
			while (!treetop.isNull()) {
				if (treetop.toElement().attribute("name") == query.value(0).toString()) {
					setting = treetop.toElement();
					break;
				}
				treetop = treetop.nextSibling();
			}
			if (setting.isNull()) {
				setting = doc.createElement("setting");
				setting.setAttribute("value", query.value(1).toString());
				setting.setAttribute("name", query.value(0).toString());
				globalSettings.appendChild(setting);
			}
			else {
				setting.setAttribute("value", query.value(1).toString());
			}
			setting = nullElement;
		}
		
		// Handle Local Settings
		query.exec("select name, value from settings where level & 2");
		while (query.next()) {
			treetop = localSettings.firstChild();
			while (!treetop.isNull()) {
				if (treetop.toElement().attribute("name") == query.value(0).toString()) {
					setting = treetop.toElement();
					break;
				}
				treetop = treetop.nextSibling();
			}
			if (setting.isNull()) {
				setting = doc.createElement("setting");
				setting.setAttribute("value", query.value(1).toString());
				setting.setAttribute("name", query.value(0).toString());
				localSettings.appendChild(setting);
			}
			else {
				setting.setAttribute("value", query.value(1).toString());
			}
			setting = nullElement;
		}
		
		file.setFileName(xmlPath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			out << doc.toString() << "\n";
			out.flush();
			file.close();
			return true;
		}
	}
	return false;
}

QRect PWMSettings::windowGeometry(QString window) {
	QRect geo;
	QStringList geoParts = setting(QString("WinRect-") + window).toString().split(" ");
	
	if (geoParts.count() == 4) {
		bool ok;
		int x, y, w, h;
		x = geoParts.at(0).toInt(&ok);
		if (ok) y = geoParts.at(1).toInt(&ok);
		if (ok) w = geoParts.at(2).toInt(&ok);
		if (ok) h = geoParts.at(3).toInt(&ok);
		if (ok) {
			geo.setRect(x, y, w, h);
		}
	}
	return geo;
}

void PWMSettings::saveWindowGeometry(QString window, QRect &geo) {
	QString geoStr("%1 %2 %3 %4");
	QVariant value;
	value = geoStr.arg(geo.x()).arg(geo.y()).arg(geo.width()).arg(geo.height());
	setSetting(QString("WinRect-") + window, value, 2);
}

QVariant PWMSettings::setting(QString name) {
	QSqlQuery query;
	
	query.prepare("select value from settings where name = ? order by level desc");
	query.addBindValue(name);
	if (query.exec() && query.first()) {
		return query.value(0);
	}
	
	return QVariant();
}

void PWMSettings::setSetting(QString name, QVariant &value, int level) {
	QSqlQuery query;
	
	query.prepare("select count(*) from settings where name = ?");
	query.addBindValue(name);
	if (query.exec() && query.first() && query.value(0).toInt() > 0) {
		query.prepare("delete from settings where name = ?");
		query.addBindValue(name);
		query.exec();
	}
	query.prepare("insert into settings (name, value, level) values (?, ?, ?)");
	query.addBindValue(name);
	query.addBindValue(value);
	query.addBindValue(level);
	if (!query.exec()) {
		qDebug() << "Error setting value";
	}
}

PWMAccount *PWMSettings::getDefaults() {
	PWMAccount *defaults = new PWMAccount();
	QSqlQuery query;
	QSqlRecord rec;
	
	// Do query stuff
	if (!query.exec("select useLeet, leetLevel, hashAlgorithm, passwordLength, username, modifier, charset, pwprefix, pwsuffix, uriparts from pwm_defaults")) {
		qDebug() << "Error with query for default account settings:" << query.lastError();
		return defaults;
	}
	if (!query.next()) {
		query.clear();
		setDefaults(defaults);
		return defaults;
	}
	defaults->setUseLeet((leetType)query.value(0).toInt());
	defaults->setLeetLevel(query.value(1).toInt());
	defaults->setHashAlgorithm(query.value(2).toInt());
	defaults->setPasswordLength(query.value(3).toInt());
	defaults->setUsername(query.value(4).toString());
	defaults->setModifier(query.value(5).toString());
	defaults->setCharset(query.value(6).toString());
	defaults->setPasswordPrefix(query.value(7).toString());
	defaults->setPasswordSuffix(query.value(8).toString());
	defaults->setUrlParts((PWMAccount::UrlParts)query.value(9).toInt());
	return defaults;
}

void PWMSettings::setDefaults(PWMAccount *defaults) {
	QSqlQuery queries;
	if (defaults->accountType() != PWMAccount::DefaultAccount) {
		return;
	}
	queries.exec("delete from pwm_defaults");
	queries.clear();
	queries.prepare(
		"insert into pwm_defaults"
		"(useLeet, leetLevel, hashAlgorithm, passwordLength, username, modifier, charset, pwprefix, pwsuffix, uriparts) values"
		"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
	);
	queries.addBindValue(defaults->useLeet());
	queries.addBindValue(defaults->leetLevel());
	queries.addBindValue(defaults->hashAlgorithm());
	queries.addBindValue(defaults->passwordLength());
	queries.addBindValue(defaults->username());
	queries.addBindValue(defaults->modifier());
	queries.addBindValue(defaults->charset());
	queries.addBindValue(defaults->passwordPrefix());
	queries.addBindValue(defaults->passwordSuffix());
	queries.addBindValue((int)defaults->urlParts());
	
	if (!queries.exec()) {
		qDebug() << "\nSQL failed with error:" << queries.lastError();
	}
}
