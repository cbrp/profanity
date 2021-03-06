/* 
 * chat_log.c
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

#include "chat_log.h"
#include "common.h"
#include "util.h"

static GHashTable *logs;
static GTimeZone *tz;

static void _close_file(gpointer key, gpointer value, gpointer user_data);

void
chat_log_init(void)
{
    tz = g_time_zone_new_local();
    logs = g_hash_table_new(NULL, (GEqualFunc) g_strcmp0);
}

void
chat_log_chat(const char * const login, char *other, 
    const char * const msg, chat_log_direction_t direction)
{
    gpointer logpp = g_hash_table_lookup(logs, other);
    FILE *logp;
    
    if (logpp == NULL) {
        GString *log_file = g_string_new(getenv("HOME"));
    
        g_string_append(log_file, "/.profanity/log");
        create_dir(log_file->str);
    
        char *login_dir = str_replace(login, "@", "_at_");
        g_string_append_printf(log_file, "/%s", login_dir);
        create_dir(log_file->str);
        free(login_dir);

        char *other_file = str_replace(other, "@", "_at_");
        g_string_append_printf(log_file, "/%s.log", other_file);
        logp = fopen(log_file->str, "a");
        free(other_file);
    
        g_string_free(log_file, TRUE);
        g_hash_table_insert(logs, other, logp);
    } else {
        logp = (FILE *) logpp;
    }

    GDateTime *dt = g_date_time_new_now(tz);
    gchar *date_fmt = g_date_time_format(dt, "%d/%m/%Y %H:%M:%S");

    if (direction == IN) {
        fprintf(logp, "%s: %s: %s\n", date_fmt, other, msg);
    } else {
        fprintf(logp, "%s: %s: %s\n", date_fmt, login, msg);
    }
    fflush(logp);

    g_date_time_unref(dt);
}

void
chat_log_close(void)
{
    g_hash_table_foreach(logs, (GHFunc) _close_file, NULL);
    g_time_zone_unref(tz);
}

static void
_close_file(gpointer key, gpointer value, gpointer user_data)
{
    fclose(value);
}
