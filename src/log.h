/* 
 * log.h
 *
 * Copyright (C) 2012 James Booth <boothj5@gmail.com>
 * 
 * This file is part of Profanity.
 *
 * Profanity is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Profanity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Profanity.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// log areas
#define PROF "prof"
#define CONN "conn"

// file log
FILE *logp; 

void log_init(void);
void log_msg(const char * const area, const char * const msg);
void log_close(void);

#endif
