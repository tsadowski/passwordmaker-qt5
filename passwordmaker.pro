# PasswordMaker - Creates and manages passwords
# Copyright (C) 2005-2006 Eric H. Jung and LeahScape, Inc.
# http://passwordmaker.org/
# grimholtz@yahoo.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 
# Written by Miquel Burns <miquelfire@gmail.com> and Eric H. Jung

DEFINES = USE_QT
TEMPLATE = app
SOURCES += shared/hasher.cpp \
	leet.cpp \
	main.cpp \
	mainwindow.cpp \
	passwordgenerator.cpp \
	pwmabout.cpp \
	pwmaccount.cpp \
	pwmsettings.cpp \
	accountwindow.cpp \
	optionswindow.cpp
HEADERS += shared/pwm_common.h \
	shared/hasher.h \
	leet.h \
	mainwindow.h \
	passwordgenerator.h \
	pwmabout.h \
	pwmaccount.h \
	pwmglobal.h\
	pwmsettings.h \
	accountwindow.h \
	optionswindow.h
# Until we find a Babzilla type site for QT apps, don't worry about this part.
#TRANSLATIONS = passwordmaker_en_US.ts
RESOURCES = resources.qrc
win32:RC_FILE = passwordmaker.rc

PRECOMPILED_HEADER = stdafx.h
CONFIG += warn_on qt
QT += xml sql

# To make the command line window on Windows
CONFIG(debug, debug|release):CONFIG += console
CONFIG(debug, debug|release):DEFINES += DEBUG

CONFIG(debug, debug|release):OBJECTS_DIR = debug
CONFIG(release, debug|release):OBJECTS_DIR = release
#prevents the exe from entering the release directory
CONFIG(release, debug|release):CONFIG -= debug_and_release_target
DESTDIR = .

!isEmpty(USE_MHASH) {
	!isEmpty(MHASH_DIR) {
		LIBS += -L$$MHASH_DIR/lib
		INCLUDEPATH += $$MHASH_DIR/include
	}
	LIBS += -lmhash
	DEFINES += USE_MHASH
} else {
	QT += script
	DEFINES += USE_QTSCRIPT USE_SHA256_BUG
}

!isEmpty(PORTABLE) {
	TARGET = PasswordMakerPortable
	isEmpty(USE_DATA_DIR) {
		USE_DATA_DIR=data
	}
	CONFIG(debug, debug|release):OBJECTS_DIR = debugPortable
	CONFIG(release, debug|release):OBJECTS_DIR = releasePortable
	DEFINES +=
}
MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR

!isEmpty(USE_DATA_DIR) {
	DEFINES += USE_DATA_DIR="\"\\\"$$USE_DATA_DIR"\"\\\"
}

!isEmpty(USE_CONSOLE) {
	CONFIG *= console
	DEFINES *= DEBUG
}