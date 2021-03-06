/* 
 * log.c
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

#include <stdio.h>
#include <stdlib.h>

#include "glib.h"

#include "log.h"
#include "common.h"

extern FILE *logp;

void
log_msg(const char * const area, const char * const msg)
{
    fprintf(logp, "%s DEBUG: %s\n", area, msg);
}

void
log_init(void)
{
    GString *log_file = g_string_new(getenv("HOME"));
    g_string_append(log_file, "/.profanity/log");
    create_dir(log_file->str);
    g_string_append(log_file, "/profanity.log");
    logp = fopen(log_file->str, "a");
    g_string_free(log_file, TRUE);
    log_msg(PROF, "Starting Profanity...");
}

void
log_close(void)
{
    fclose(logp);
}
