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

#include <QRegExp>

#include "leet.h"

static QRegExp alphabet[] = {
	QRegExp("a"),
	QRegExp("b"),
	QRegExp("c"),
	QRegExp("d"),
	QRegExp("e"),
	QRegExp("f"),
	QRegExp("g"),
	QRegExp("h"),
	QRegExp("i"),
	QRegExp("j"),
	QRegExp("k"),
	QRegExp("l"),
	QRegExp("m"),
	QRegExp("n"),
	QRegExp("o"),
	QRegExp("p"),
	QRegExp("q"),
	QRegExp("r"),
	QRegExp("s"),
	QRegExp("t"),
	QRegExp("u"),
	QRegExp("v"),
	QRegExp("w"),
	QRegExp("x"),
	QRegExp("y"),
	QRegExp("z")
};

// Unless we make an Array class, we can only do this with
static char *levels[][26] = {
	{"4", "b", "c", "d", "3", "f", "g", "h", "i", "j", "k", "1", "m", "n", "0", "p", "9", "r", "s", "7", "u", "v", "w", "x", "y", "z"},
	{"4", "b", "c", "d", "3", "f", "g", "h", "1", "j", "k", "1", "m", "n", "0", "p", "9", "r", "5", "7", "u", "v", "w", "x", "y", "2"},
	{"4", "8", "c", "d", "3", "f", "6", "h", "'", "j", "k", "1", "m", "n", "0", "p", "9", "r", "5", "7", "u", "v", "w", "x", "'/", "2"},
	{"@", "8", "c", "d", "3", "f", "6", "h", "'", "j", "k", "1", "m", "n", "0", "p", "9", "r", "5", "7", "u", "v", "w", "x", "'/", "2"},
	{"@", "|3", "c", "d", "3", "f", "6", "#", "!", "7", "|<", "1", "m", "n", "0", "|>", "9", "|2", "$", "7", "u", "\\/", "w", "x", "'/", "2"},
	{"@", "|3", "c", "|)", "&", "|=", "6", "#", "!", ",|", "|<", "1", "m", "n", "0", "|>", "9", "|2", "$", "7", "u", "\\/", "w", "x", "'/", "2"},
	{"@", "|3", "[", "|)", "&", "|=", "6", "#", "!", ",|", "|<", "1", "^^", "^/", "0", "|*", "9", "|2", "5", "7", "(_)", "\\/", "\\/\\/", "><", "'/", "2"},
	{"@", "8", "(", "|)", "&", "|=", "6", "|-|", "!", "_|", "|(", "1", "|\\/|", "|\\|", "()", "|>", "(,)", "|2", "$", "|", "|_|", "\\/", "\\^/", ")(", "'/", "\"/_"},
	{"@", "8", "(", "|)", "&", "|=", "6", "|-|", "!", "_|", "|{", "|_", "/\\/\\", "|\\|", "()", "|>", "(,)", "|2", "$", "|", "|_|", "\\/", "\\^/", ")(", "'/", "\"/_"}
};

/**
 * Convert the string in _message_ to l33t-speak
 * using the l33t level specified by _leetLevel_.
 * l33t levels are 0-8 with 0 being the simplest
 * form of l33t-speak and 8 being the most complex.
 *
 * Note that _message_ is converted to lower-case if
 * the l33t conversion is performed. To maintain
 * backwards compatibility, do not change this function
 * so it uses mixed-case.
 *
 * Using a _leetLevel_ <= 0 results in the original message
 * being returned.
 *
 */
QString leetConvert(int level, QString message)
{
	if (level > -1 && level < 9) {
		QString ret = message.toLower();
		for (int item = 0; item < 26; item++) {
			ret = ret.replace(alphabet[item], levels[level][item]);
		}
		return ret;
	}
	return message;
}
