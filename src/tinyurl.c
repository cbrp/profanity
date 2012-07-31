/* 
 * tinyurl.c
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

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <glib.h>

struct curl_data_t
{
    char *buffer;
    size_t size;
};

static size_t _data_callback(void *ptr, size_t size, size_t nmemb, void *data);

void
tinyurl_init(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
}

gboolean
tinyurl_valid(char *url)
{
    return (g_str_has_prefix(url, "http://") || 
        g_str_has_prefix(url, "https://"));
}

char *
tinyurl_get(char *url)
{
    GString *full_url = g_string_new("http://tinyurl.com/api-create.php?url=");
    g_string_append(full_url, url);

    CURL *handle = curl_easy_init();
    CURLcode result; 
    struct curl_data_t output; 
    output.buffer = NULL;
    output.size = 0;

    curl_easy_setopt(handle, CURLOPT_URL, full_url->str);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _data_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&output);
    
    result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    output.buffer[output.size++] = '\0';
    g_string_free(full_url, TRUE);

    return output.buffer;
}

static size_t 
_data_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct curl_data_t *mem = (struct curl_data_t *) data;
    mem->buffer = realloc(mem->buffer, mem->size + realsize + 1);

    if ( mem->buffer )
    {
        memcpy( &( mem->buffer[ mem->size ] ), ptr, realsize );
        mem->size += realsize;
        mem->buffer[ mem->size ] = 0;
    }
    
    return realsize;
}