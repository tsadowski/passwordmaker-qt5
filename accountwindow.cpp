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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QRegExp>

#include "accountwindow.h"
#include "pwmaccount.h"
#include "pwmsettings.h"
#include "passwordgenerator.h"

AccountWindow::AccountWindow(/* Type */QWidget *parent) : QDialog(parent) {
	pg = new PasswordGenerator(this);
	createInterface();
}

void AccountWindow::accept() {
	PWMSettings settings(this);
	// Save the current settings
	pg->getSettings(currentAccount);
	settings.setDefaults(currentAccount);
	delete currentAccount;
	currentAccount = 0;
	QDialog::accept();
}

void AccountWindow::reject() {
	delete currentAccount;
	currentAccount = 0;
	QDialog::reject();
}

void AccountWindow::exec(QString id) {
	PWMSettings settings(this);
	PWMAccount::UrlParts urlParts;
	if (id.isEmpty()) {
		currentAccount = settings.getDefaults();
	}
	else {
		// Until account support is handled
		QDialog::reject();
	}
	pg->setSettings(currentAccount);
	changeLeet(); // ensure Leet level is disabled/enabled correctly
	// Workaround to avoid editbox focus from changing while the user types in the their character set
	findChild<QComboBox *>("charsetControl")->setEditText(currentAccount->charset());
	
	// TODO: Fix to support accounts that do not use this bit
	inputURL = parent()->findChild<QLineEdit *>("inputUrlControl")->text();
	urlParts = currentAccount->urlParts();
	findChild<QCheckBox *>("protocolControl")->setCheckState((urlParts & PWMAccount::UrlProtocol) ? Qt::Checked : Qt::Unchecked);
	findChild<QCheckBox *>("subdomainControl")->setCheckState((urlParts & PWMAccount::UrlSubdomains) ? Qt::Checked : Qt::Unchecked);
	findChild<QCheckBox *>("domainControl")->setCheckState((urlParts & PWMAccount::UrlDomain) ? Qt::Checked : Qt::Unchecked);
	findChild<QCheckBox *>("pathControl")->setCheckState((urlParts & PWMAccount::UrlOthers) ? Qt::Checked : Qt::Unchecked);
	urlPartChanged(); // ensure calculated URL is correct
	// Load current settings
	QDialog::exec();
}

void AccountWindow::urlPartChanged() {
	// Some url Part changed, check each and update the internal state to reflect it
	QRegExp rx("([^://]+://)?([^:/]+)([^#]*)");
	if (rx.indexIn(inputURL) != -1) {
		QString endURL;
		
		QStringList domainSegments = rx.cap(2).split(".");
		if (domainSegments.count() < 3) {
			domainSegments.prepend("");
		}
		
		if (findChild<QCheckBox *>("subdomainControl")->checkState() == Qt::Checked) {
			for (int i = 0; i < domainSegments.count()-2; i++) {
				endURL += domainSegments[i];
				if (i+1 < domainSegments.count()-2)
					endURL += ".";
			}
		}
		
		if (findChild<QCheckBox *>("domainControl")->checkState() == Qt::Checked) {
			if (endURL != "" && endURL[endURL.size()-1] != QChar('.'))
				endURL += ".";
			endURL += domainSegments[domainSegments.count()-2] + "." + domainSegments[domainSegments.count()-1];
		}
		
		if (findChild<QCheckBox *>("protocolControl")->checkState() == Qt::Checked) {
			endURL.prepend(rx.cap(1));
		}
		
		if (findChild<QCheckBox *>("pathControl")->checkState() == Qt::Checked) {
			endURL += rx.cap(3);
		}
		
		findChild<QLineEdit *>("calculatedUrl")->setText(endURL);
	}
	else {
		findChild<QLineEdit *>("calculatedUrl")->setText("");
	}
	
	// Set account settings for when it's saved later on
	PWMAccount::UrlParts urlParts = PWMAccount::UrlNone;
	if (findChild<QCheckBox *>("protocolControl")->checkState() == Qt::Checked) {
		urlParts |= PWMAccount::UrlProtocol;
	}
	if (findChild<QCheckBox *>("subdomainControl")->checkState() == Qt::Checked) {
		urlParts |= PWMAccount::UrlSubdomains;
	}
	if (findChild<QCheckBox *>("domainControl")->checkState() == Qt::Checked) {
		urlParts |= PWMAccount::UrlDomain;
	}
	if (findChild<QCheckBox *>("pathControl")->checkState() == Qt::Checked) {
		urlParts |= PWMAccount::UrlOthers;
	}
	currentAccount->setUrlParts(urlParts);
}

void AccountWindow::updateUseLeet(leetType l) {
	QComboBox *useLeetControl = findChild<QComboBox *>("useLeetControl");
	useLeetControl->setCurrentIndex(useLeetControl->findData(l));
}

void AccountWindow::updateLeetLevel(int l) {
	QComboBox *leetLevelControl = findChild<QComboBox *>("leetLevelControl");
	leetLevelControl->setCurrentIndex(leetLevelControl->findData(l));
}

void AccountWindow::changeLeet() {
	QComboBox *useLeetControl = findChild<QComboBox *>("useLeetControl");
	QComboBox *leetLevelControl = findChild<QComboBox *>("leetLevelControl");
	leetType lt = (leetType)useLeetControl->itemData(useLeetControl->currentIndex()).toInt();
	pg->setLeetLevel(leetLevelControl->itemData(leetLevelControl->currentIndex()).toInt());
	if (lt == LEET_NONE) {
		leetLevelControl->setEnabled(false);
	}
	else {
		leetLevelControl->setEnabled(true);
	}
	pg->setUseLeet(lt);
}

void AccountWindow::updateAlgorithm(int a) {
	QComboBox *algoControl = findChild<QComboBox *>("algoControl");
	algoControl->setCurrentIndex(algoControl->findData(a));
}

void AccountWindow::changeAlgorithm() {
	QComboBox *algoControl = findChild<QComboBox *>("algoControl");
	QComboBox *charsetControl = findChild<QComboBox *>("charsetControl");
	int hash = algoControl->itemData(algoControl->currentIndex()).toInt();
	pg->setFullAlgorithm(hash);
	if (hash & PWM_V6) {
		charsetControl->setEnabled(false);
	}
	else {
		charsetControl->setEnabled(true);
	}
}

void AccountWindow::createInterface() {
	QPushButton *button;
	QWidget *tab;
	
	QTabWidget *tabs = new QTabWidget();
	
	// Gernal Tab (None for defaults)
	
	// URLs Tab
	tab = createUrlTab();
	tabs->addTab(tab, tr("URLs"));
	
	// Extended Tab
	tab = createExtendedTab();
	tabs->addTab(tab, tr("Extended"));
	
	// Password details area
	
	// Start of replacement with QDialogButtonBox
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	
	button = new QPushButton(tr("&Ok"));
	buttonLayout->addWidget(button);
	connect(button, SIGNAL(clicked()), this, SLOT(accept()));
	
	button = new QPushButton(tr("&Cancel"));
	buttonLayout->addWidget(button);
	connect(button, SIGNAL(clicked()), this, SLOT(reject()));
	// End of replacement with QDialogButtonBox
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabs);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);
}

QWidget *AccountWindow::createUrlTab() {
	QHBoxLayout *hLayout;
	QLabel *label;
	QLineEdit *lineEdit;
	
	QWidget *tab = new QWidget;
	QVBoxLayout *mainLayout = new QVBoxLayout();
	tab->setLayout(mainLayout);
	
	if (true) { // It's a default account window
		QCheckBox *cb;
		
		QGroupBox *UrlBox = new QGroupBox(tr("URL Components"));
		mainLayout->addWidget(UrlBox);
		QVBoxLayout *groupLayout = new QVBoxLayout();
		UrlBox->setLayout(groupLayout);
		
		hLayout = new QHBoxLayout();
		groupLayout->addLayout(hLayout);
		
		cb = new QCheckBox(tr("Protocol"));
		cb->setObjectName("protocolControl");
		hLayout->addWidget(cb);
		connect(cb, SIGNAL(stateChanged(int)), SLOT(urlPartChanged()));
		
		cb = new QCheckBox(tr("Subdomain(s)"));
		cb->setObjectName("subdomainControl");
		hLayout->addWidget(cb);
		connect(cb, SIGNAL(stateChanged(int)), SLOT(urlPartChanged()));
		
		cb = new QCheckBox(tr("Domain"));
		cb->setObjectName("domainControl");
		hLayout->addWidget(cb);
		connect(cb, SIGNAL(stateChanged(int)), SLOT(urlPartChanged()));
		
		cb = new QCheckBox(tr("Port, path, anchor, query parameters"));
		cb->setObjectName("pathControl");
		hLayout->addWidget(cb);
		connect(cb, SIGNAL(stateChanged(int)), SLOT(urlPartChanged()));
		
		hLayout->addStretch(1);
		
		hLayout = new QHBoxLayout();
		groupLayout->addLayout(hLayout);
		
		label = new QLabel(tr("Calculated URL of current site:"));
		hLayout->addWidget(label);
		
		lineEdit = new QLineEdit();
		lineEdit->setObjectName("calculatedUrl");
		hLayout->addWidget(lineEdit, 1);
	}
	else {
		// Until account support is used...
	}
	mainLayout->addStretch(1);
	
	return tab;
}

QWidget *AccountWindow::createExtendedTab() {
	QLabel *label;
	QLineEdit *lineEdit;
	QComboBox *comboBox;
	QHBoxLayout *hLayout;
	
	QWidget *tab = new QWidget();
	QGridLayout *layout = new QGridLayout();
	tab->setLayout(layout);
	
	label = new QLabel(tr("Username"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("usernameControl");
	label->setBuddy(lineEdit);
	layout->addWidget(label, 0, 0);
	layout->addWidget(lineEdit, 0, 1);
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setUserName(QString)));
	connect(pg, SIGNAL(userNameChanged(QString)), lineEdit, SLOT(setText(QString)));
	
	label = new QLabel(tr("Use l33t"));
	layout->addWidget(label, 1, 0);
	hLayout = new QHBoxLayout();
		comboBox = new QComboBox();
		comboBox->setObjectName("useLeetControl");
		label->setBuddy(comboBox);
		comboBox->addItem(tr("Not at all"), LEET_NONE);
		comboBox->addItem(tr("Before generating password"), LEET_BEFORE);
		comboBox->addItem(tr("After generating password"), LEET_AFTER);
		comboBox->addItem(tr("Before and after generating password"), LEET_BOTH);
		hLayout->addWidget(comboBox);
		hLayout->addStretch(1);
		connect(comboBox, SIGNAL(activated(int)), SLOT(changeLeet()));
		connect(pg, SIGNAL(useLeetChanged(leetType)), SLOT(updateUseLeet(leetType)));
		
		label = new QLabel(tr("l33t Level"));
		comboBox = new QComboBox();
		comboBox->setObjectName("leetLevelControl");
		label->setBuddy(comboBox);
		for (int i = 0; i < 9; i++) {
			comboBox->addItem(QString("%1").arg(i+1), i);
		}
		hLayout->addWidget(label);
		hLayout->addWidget(comboBox);
		connect(comboBox, SIGNAL(activated(int)), SLOT(changeLeet()));
		connect(pg, SIGNAL(leetLevelChanged(int)), SLOT(updateLeetLevel(int)));
	layout->addLayout(hLayout, 1, 1);	
	
	// Use hLayout trick here?
	label = new QLabel(tr("Hash Algorithm"));
	comboBox = new QComboBox();
	comboBox->setObjectName("algoControl");
	label->setBuddy(comboBox);
	comboBox->addItem("MD4", PWM_MD4);
	comboBox->addItem("HMAC-MD4", PWM_HMAC | PWM_MD4);
	comboBox->addItem("MD5", PWM_MD5);
	comboBox->addItem("MD5 Version 0.6", PWM_MD5 | PWM_V6);
	comboBox->addItem("HMAC-MD5", PWM_HMAC | PWM_MD5);
	comboBox->addItem("HMAC-MD5 Version 0.6", PWM_HMAC | PWM_MD5 | PWM_V6);
	comboBox->addItem("SHA-1", PWM_SHA1);
	comboBox->addItem("HMAC-SHA-1", PWM_HMAC | PWM_SHA1);
	comboBox->addItem("SHA-256", PWM_SHA256);
	comboBox->addItem("HMAC-SHA-256", PWM_HMAC | PWM_SHA256);
	#ifdef USE_SHA256_BUG
	comboBox->addItem("HMAC-SHA-256 Version 1.5.2", PWM_HMAC | PWM_SHA256 | PWM_HMAC_BUG);
	#endif //USE_SHA256_BUG
	comboBox->addItem("RIPEMD-160", PWM_RIPEMD160);
	comboBox->addItem("HMAC-RIPEMD-160", PWM_HMAC | PWM_RIPEMD160);
	layout->addWidget(label, 2, 0);
	layout->addWidget(comboBox, 2, 1);
	connect(comboBox, SIGNAL(activated(int)), SLOT(changeAlgorithm()));
	connect(pg, SIGNAL(fullAlgorithmChanged(int)), SLOT(updateAlgorithm(int)));

	label = new QLabel(tr("Password Length"));
	layout->addWidget(label, 3, 0);
	hLayout = new QHBoxLayout();
		QSpinBox *spinBox = new QSpinBox();
		spinBox->setObjectName("passwordLengthControl");
		label->setBuddy(spinBox);
		spinBox->setMinimum(1);
		hLayout->addWidget(spinBox);
		hLayout->addStretch(1);
	layout->addLayout(hLayout, 3, 1);
	connect(spinBox, SIGNAL(valueChanged(int)), pg, SLOT(setPasswordLength(int)));
	connect(pg, SIGNAL(passwordLengthChanged(int)), spinBox, SLOT(setValue(int)));
	
	label = new QLabel(tr("Characters"));
	comboBox = new QComboBox();
	comboBox->setObjectName("charsetControl");
	label->setBuddy(comboBox);
	comboBox->addItem("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./");
	comboBox->addItem("0123456789abcdef");
	comboBox->addItem("0123456789");
	comboBox->addItem("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	comboBox->addItem("`~!@#$%^&*()_-+={}|[]\\:\";'<>?,./");
	// charsetControl->addItem(""); // Random will come later
	comboBox->setCurrentIndex(0);
	comboBox->setEditable(true);
	comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
	comboBox->setInsertPolicy(QComboBox::NoInsert);
	comboBox->lineEdit()->setSelection(0, 0);
	// Tips button to come later
	layout->addWidget(label, 4, 0);
	layout->addWidget(comboBox, 4, 1);
	connect(comboBox, SIGNAL(editTextChanged(QString)), pg, SLOT(setCharacterSet(QString)));

	// Modifier
	label = new QLabel(tr("Modifier"));
	lineEdit = new QLineEdit();
	lineEdit->setObjectName("modifierControl");
	label->setBuddy(lineEdit);
	layout->addWidget(label, 5, 0);
	layout->addWidget(lineEdit, 5, 1);
	connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setModifier(QString)));
	connect(pg, SIGNAL(modifierChanged(QString)), lineEdit, SLOT(setText(QString)));

	// Password Prefix/Password Suffix
	label = new QLabel(tr("Password prefix"));
	layout->addWidget(label, 6, 0);
	hLayout = new QHBoxLayout();
		lineEdit = new QLineEdit();
		lineEdit->setObjectName("prefixControl");
		label->setBuddy(lineEdit);
		hLayout->addWidget(lineEdit);
		connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setPrefix(QString)));
		connect(pg, SIGNAL(prefixChanged(QString)), lineEdit, SLOT(setText(QString)));
		
		label = new QLabel(tr("Password suffix"));
		lineEdit = new QLineEdit();
		lineEdit->setObjectName("suffixControl");
		label->setBuddy(lineEdit);
		hLayout->addWidget(label);
		hLayout->addWidget(lineEdit);
		connect(lineEdit, SIGNAL(textChanged(QString)), pg, SLOT(setSuffix(QString)));
		connect(pg, SIGNAL(suffixChanged(QString)), lineEdit, SLOT(setText(QString)));
	layout->addLayout(hLayout, 6, 1);
	
	layout->setRowStretch(7, 1);
	
	return tab;
}
