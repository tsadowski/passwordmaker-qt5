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

#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "shared/pwm_common.h"
#include "pwmabout.h"
#include "pwmglobal.h"

PWMAbout::PWMAbout(QWidget *parent) : QDialog(parent) {
	setLayout(new QVBoxLayout);
	
	QGroupBox *box = new QGroupBox;
	layout()->addWidget(box);
	QLabel *message = new QLabel(tr("<h1>PasswordMaker Desktop Edition</h1>"
	"<h3>One Password To Rule Them All&trade;</h3>"
	"<p>Version %1</p>"
	"<p>Web Site:<br /><tt>http://passwordmaker.org</tt></p>"
	"<p>Created By:<br />LeahScape, Inc"
	"<br />Miquel Fire</p>").arg(PWMDE_VERSION));
	box->setLayout(new QVBoxLayout);
	box->layout()->addWidget(message);
	
	QHBoxLayout *bottom = new QHBoxLayout;
	((QBoxLayout *)layout())->addLayout(bottom);
	bottom->addStretch(1);
	QPushButton *closeButton = new QPushButton(tr("Close"));
	bottom->addWidget(closeButton);
	closeButton->setDefault(true);
	connect(closeButton, SIGNAL(clicked(bool)), SLOT(accept()));
}
