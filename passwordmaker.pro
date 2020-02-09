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

CONFIG += warn_on qt
QT += xml sql widgets

# To make the command line window on Windows
CONFIG(debug, debug|release):CONFIG += console
CONFIG(debug, debug|release):DEFINES += DEBUG

CONFIG(debug, debug|release):OBJECTS_DIR = debug
CONFIG(release, debug|release):OBJECTS_DIR = release
#prevents the exe from entering the release directory
CONFIG(release, debug|release):CONFIG -= debug_and_release_target
DESTDIR = .

isEmpty(PREFIX) {
 PREFIX=/usr/local
}

target.path = $$PREFIX/bin/passwordmaker
INSTALLS += target
icon16.path = $$PREFIX/share/icons/hicolor/16x16/apps/
icon16.extra = mkdir -p $$shadowed($$PWD)/16x16 && \
               cp $$PWD/images/ring-16x16.png $$shadowed($$PWD)/16x16/ring.png
icon16.files = $$shadowed($$PWD)/16x16/ring.png
INSTALLS += icon16
icon24.path = $$PREFIX/share/icons/hicolor/24x24/apps/
icon24.extra = mkdir -p $$shadowed($$PWD)/24x24 && \
               cp $$PWD/images/ring-24x24.png $$shadowed($$PWD)/24x24/ring.png
icon24.files = $$shadowed($$PWD)/24x24/ring.png
INSTALLS += icon24
icon32.path = $$PREFIX/share/icons/hicolor/32x32/apps/
icon32.extra = mkdir -p $$shadowed($$PWD)/32x32 && \
               cp $$PWD/images/ring-32x32.png $$shadowed($$PWD)/32x32/ring.png
icon32.files = $$shadowed($$PWD)/32x32/ring.png
INSTALLS += icon32
icon48.path = $$PREFIX/share/icons/hicolor/48x48/apps/
icon48.extra = mkdir -p $$shadowed($$PWD)/48x48 && \
               cp $$PWD/images/ring-48x48.png $$shadowed($$PWD)/48x48/ring.png
icon48.files = $$shadowed($$PWD)/48x48/ring.png
INSTALLS += icon48
icon64.path = $$PREFIX/share/icons/hicolor/64x64/apps/
icon64.extra = mkdir -p $$shadowed($$PWD)/64x64 && \
               cp $$PWD/images/ring-64x64.png $$shadowed($$PWD)/64x64/ring.png
icon64.files = $$shadowed($$PWD)/64x64/ring.png
INSTALLS += icon64
icon128.path = $$PREFIX/share/icons/hicolor/128x128/apps/
icon128.extra = mkdir -p $$shadowed($$PWD)/128x128 && \
                cp $$PWD/images/ring-128x128.png $$shadowed($$PWD)/128x128/ring.png
icon128.files = $$shadowed($$PWD)/128x128/ring.png
INSTALLS += icon128
icon256.path = $$PREFIX/share/icons/hicolor/256x256/apps/
icon256.extra = mkdir -p $$shadowed($$PWD)/256x256 && \
                cp $$PWD/images/ring-256x256.png $$shadowed($$PWD)/256x256/ring.png
icon256.files = $$shadowed($$PWD)/256x256/ring.png
INSTALLS += icon256
desktop.path = $$PREFIX/share/applications/
desktop.files = $$PWD/passwordmaker.desktop
INSTALLS += desktop

#!isEmpty(USE_MHASH) {
#	!isEmpty(MHASH_DIR) {
#		LIBS += -L$$MHASH_DIR/lib
#		INCLUDEPATH += $$MHASH_DIR/include
#	}
	LIBS += -lmhash
	DEFINES += USE_MHASH
#} else {
#	QT += script
#	DEFINES += USE_QTSCRIPT USE_SHA256_BUG
#}

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
