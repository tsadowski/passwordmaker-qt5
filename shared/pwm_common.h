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

/*
 * This file contains definition that are used in all files.
 */
#ifndef PWM_COMMON_H
#define PWM_COMMON_H

enum hashType {
	PWM_MD4 = 1,
	PWM_MD5,
	PWM_SHA1,
	PWM_RIPEMD160,
	PWM_SHA256,
	// Following are bitmasks that can be applied
	PWM_HMAC = 0x0100,
	PWM_V6 = 0x0200, /* Sets trim to false and charset set to hexstring*/
	PWM_HMAC_BUG = 0x0400 /* Use the buggy version of HMAC-SHA-256 */
};

enum leetType {
	LEET_NONE,
	LEET_BEFORE,
	LEET_AFTER,
	LEET_BOTH
};

#endif // PWM_COMMON_H
