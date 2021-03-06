/* 
 * command.c
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

#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include "common.h"
#include "command.h"
#include "contact_list.h"
#include "history.h"
#include "jabber.h"
#include "ui.h"
#include "util.h"
#include "preferences.h"
#include "prof_autocomplete.h"
#include "tinyurl.h"

/* command structure
 * cmd - The actual string of the command
 * func - The function to execute for the command
 */
struct cmd_t {
    const gchar *cmd;
    gboolean (*func)(const char * const inp, struct cmd_help_t help);
    struct cmd_help_t help;
};


// helpers
static struct cmd_t * _cmd_get_command(const char * const command);
static gboolean _handle_command(const char * const command, 
    const char * const inp);
static void _update_presence(const jabber_presence_t presence, 
    const char * const show, const char * const inp);

// command prototypes
static gboolean _cmd_quit(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_help(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_prefs(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_who(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_ros(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_connect(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_msg(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_tiny(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_close(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_set_beep(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_set_notify(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_set_typing(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_set_flash(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_set_showsplash(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_set_chlog(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_away(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_online(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_dnd(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_chat(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_xa(const char * const inp, struct cmd_help_t help);
static gboolean _cmd_default(const char * const inp);

// The commands
static struct cmd_t main_commands[] = 
{
    { "/help", 
        _cmd_help, 
        { "/help [command]", "Show help summary, or help on a specific command",
        { "/help [command]",
          "---------------",
          "List all commands with short help on what they do.",
          "Specify the command if you want more detailed help on a specific command.",
          "",
          "Example : /help connect",
          NULL } } },

    { "/connect", 
        _cmd_connect, 
        { "/connect user@host", "Login to jabber.",
        { "/connect user@host",
          "------------------",
          "Connect to the jabber server at host using the username user.",
          "Profanity should work with any XMPP (Jabber) compliant chat host.",
          "You can use tab completion to autocomplete any logins you have used before.",
          "",
          "Example: /connect myuser@gmail.com", 
          NULL  } } },
    
    { "/prefs", 
        _cmd_prefs, 
        { "/prefs", "Show current preferences.",
        { "/prefs",
          "------",
          "List all current user preference settings.",
          "User preferences are stored at:",
          "",
          "    ~/.profanity/config",
          "",
          "Preference changes made using the various commands take effect immediately,",
          "you will need to restart Profanity for config file edits to take effect.",
          NULL } } },

    { "/msg", 
        _cmd_msg, 
        { "/msg user@host mesg", "Send mesg to user.",
        { "/msg user@host mesg",
          "-------------------",
          "Send a message to the user specified.",
          "Use tab completion to autocomplete online contacts.",
          "If there is no current chat with the recipient, a new chat window",
          "will be opened, and highlighted in the status bar at the bottom.",
          "pressing the corresponding F key will take you to that window.",
          "This command can be called from any window, including chat with other users.",
          "",
          "Example : /msg boothj5@gmail.com Hey, here's a message!",
          NULL } } },

    { "/tiny", 
        _cmd_tiny, 
        { "/tiny url", "Send url as tinyurl in current chat.",
        { "/tiny url",
          "---------",
          "Send the url as a tiny url.",
          "This command can only be called when in a chat window,",
          "not from the console.",
          "",
          "Example : /tiny http://www.google.com",
          NULL } } },

    { "/ros", 
        _cmd_ros,
        { "/ros", "List all contacts.",
        { "/ros",
          "----",
          "List all contact currently on the chat hosts roster.",
          "See /who for a more useful list of contacts who are currently online.",
          NULL } } },

    { "/who", 
        _cmd_who,
        { "/who", "Find out who is online.",
        { "/who",
          "----",
          "Show the list of all online contacts with their current status message.",
          NULL } } },

    { "/close", 
        _cmd_close, 
        { "/close", "Close current chat window.",
        { "/close",
          "------",
          "Close the current chat window, no message is sent to the recipient,",
          "The chat window will become available for new chats.",
          NULL } } },

    { "/quit", 
        _cmd_quit, 
        { "/quit", "Quit Profanity.",
        { "/quit",
          "-----",
          "Logout of any current sessions, and quit Profanity.",
          NULL } } }
};

static struct cmd_t setting_commands[] = 
{
    { "/beep",
        _cmd_set_beep,
        { "/beep on|off", "Enable/disable sound notifications.",
        { "/beep on|off",
          "------------",
          "Switch the terminal bell on or off.",
          "The bell will sound when incoming messages are received.",
          "If the terminal does not support sounds, it may attempt to",
          "flash the screen instead.",
          "",
          "Config file section : [ui]",
          "Config file value :   beep=true|false",
          NULL } } },

    { "/notify",
        _cmd_set_notify,
        { "/notify on|off", "Enable/disable desktop notifications.",
        { "/notify on|off",
          "--------------",
          "Switch the desktop notifications on or off.",
          "The notification will appear for all incoming messages.",
          "The desktop environment must support desktop notifications.",
          "",
          "Config file section : [ui]",
          "Config file value :   notify=true|false",
          NULL } } },

    { "/typing",
        _cmd_set_typing,
        { "/typing on|off", "Enable/disable typing notifications.",
        { "/typing on|off",
          "--------------",
          "Switch typing notifications on or off for incoming messages",
          "If desktop notifications are also enabled you will receive them",
          "for when users are typing a message to you.",
          "",
          "Config file section : [ui]",
          "Config file value :   typing=true|false",
          NULL } } },

    { "/flash", 
        _cmd_set_flash,
        { "/flash on|off", "Enable/disable screen flash notifications.",
        { "/flash on|off",
          "-------------",
          "Make the terminal flash when incoming messages are recieved.",
          "The flash will only occur if you are not in the chat window associated",
          "with the user sending the message.",
          "The terminal must support flashing, if it doesn't it may attempt to beep.",
          "",
          "Config file section : [ui]",
          "Config file value :   flash=true|false",
          NULL } } },

    { "/showsplash", 
        _cmd_set_showsplash, 
        { "/showsplash on|off", "Enable/disable splash logo on startup.",
        { "/showsplash on|off",
          "------------------",
          "Switch on or off the ascii logo on start up.",
          "",
          "Config file section : [ui]",
          "Config file value :   showsplash=true|false",
          NULL } } },

    { "/chlog", 
        _cmd_set_chlog,
        { "/chlog on|off", "Enable/disable chat logging.",
        { "/chlog on|off",
          "-------------",
          "Switch chat logging on or off.",
          "Chat logs are stored in the ~/.profanoty/log directory.",
          "A folder is created for each login that you have used with Profanity.",
          "Within in those folders, a log file is created for each user you chat to.",
          "",
          "For example if you are logged in as someuser@chatserv.com, and you chat",
          "to myfriend@chatserv.com, the following chat log will be created:",
          "",
          "    ~/.profanity/log/someuser_at_chatserv.com/myfriend_at_chatserv.com",
          NULL } } }
};

static struct cmd_t status_commands[] = 
{
    { "/away",
        _cmd_away,
        { "/away [msg]", "Set status to away.",
        { "/away [msg]",
          "-----------",
          "Set your status to \"away\" with the optional message.",
          "Your current status can be found in the top right of the screen.",
          "",
          "Example : /away Gone for lunch",
          NULL } } },

    { "/chat",
        _cmd_chat,
        { "/chat [msg]", "Set status to chat (available for chat).",
        { "/chat [msg]",
          "-----------",
          "Set your status to \"chat\", meaning \"available for chat\",",
          "with the optional message.",
          "Your current status can be found in the top right of the screen.",
          "",
          "Example : /chat Please talk to me!",
          NULL } } },

    { "/dnd",
        _cmd_dnd,
        { "/dnd [msg]", "Set status to dnd (do not disturb.",
        { "/dnd [msg]",
          "----------",
          "Set your status to \"dnd\", meaning \"do not disturb\",",
          "with the optional message.",
          "Your current status can be found in the top right of the screen.",
          "",
          "Example : /dnd I'm in the zone",
          NULL } } },

    { "/online", 
        _cmd_online,
        { "/online [msg]", "Set status to online.",
        { "/online [msg]",
          "-------------",
          "Set your status to \"online\" with the optional message.",
          "Your current status can be found in the top right of the screen.",
          "",
          "Example : /online Up the Irons!",
          NULL } } },

    { "/xa",
        _cmd_xa,
        { "/xa [msg]", "Set status to xa (extended away).",
        { "/xa [msg]",
          "---------",
          "Set your status to \"xa\", meaning \"extended away\",",
          "with the optional message.",
          "Your current status can be found in the top right of the screen.",
          "",
          "Example : /xa This meeting is going to be a long one",
          NULL } } },
};
    
static PAutocomplete commands_ac;

gboolean
process_input(char *inp)
{
    gboolean result = FALSE;

    g_strstrip(inp);

    if (strlen(inp) > 0)
        history_append(inp);

    if (strlen(inp) == 0) {
        result = TRUE;
    } else if (inp[0] == '/') {
        char inp_cpy[strlen(inp) + 1];
        strcpy(inp_cpy, inp);
        char *command = strtok(inp_cpy, " ");
        result = _handle_command(command, inp);
    } else {
        result = _cmd_default(inp);
    }

    inp_clear();
    reset_search_attempts();
    win_page_off();

    return result;
}

void
command_init(void)
{
    commands_ac = p_autocomplete_new();

    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(main_commands); i++) {
        struct cmd_t *pcmd = main_commands+i;
        p_autocomplete_add(commands_ac, (gchar *)pcmd->cmd);
    }

    for (i = 0; i < ARRAY_SIZE(setting_commands); i++) {
        struct cmd_t *pcmd = setting_commands+i;
        p_autocomplete_add(commands_ac, (gchar *)pcmd->cmd);
    }

    for (i = 0; i < ARRAY_SIZE(status_commands); i++) {
        struct cmd_t *pcmd = status_commands+i;
        p_autocomplete_add(commands_ac, (gchar *)pcmd->cmd);
    }

    history_init();
}

char *
cmd_complete(char *inp)
{
    return p_autocomplete_complete(commands_ac, inp);
}

void
reset_command_completer(void)
{
    p_autocomplete_reset(commands_ac);
}

GSList *
cmd_get_help_list_basic(void)
{
    GSList *result = NULL;

    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(main_commands); i++) {
        result = g_slist_append(result, &((main_commands+i)->help));
    }

    return result;
}

GSList *
cmd_get_help_list_settings(void)
{
    GSList *result = NULL;

    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(setting_commands); i++) {
        result = g_slist_append(result, &((setting_commands+i)->help));
    }

    return result;
}

GSList *
cmd_get_help_list_status(void)
{
    GSList *result = NULL;

    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(status_commands); i++) {
        result = g_slist_append(result, &((status_commands+i)->help));
    }

    return result;
}

static struct cmd_t *
_cmd_get_command(const char * const command)
{
    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(main_commands); i++) {
        struct cmd_t *pcmd = main_commands+i;
        if (strcmp(pcmd->cmd, command) == 0) {
            return pcmd;
        }
    }

    for (i = 0; i < ARRAY_SIZE(setting_commands); i++) {
        struct cmd_t *pcmd = setting_commands+i;
        if (strcmp(pcmd->cmd, command) == 0) {
            return pcmd;
        }
    }

    for (i = 0; i < ARRAY_SIZE(status_commands); i++) {
        struct cmd_t *pcmd = status_commands+i;
        if (strcmp(pcmd->cmd, command) == 0) {
            return pcmd;
        }
    }

    return NULL;
}


static gboolean
_handle_command(const char * const command, const char * const inp)
{
    struct cmd_t *cmd = _cmd_get_command(command);
    
    if (cmd != NULL) {
        return (cmd->func(inp, cmd->help));
    } else {
        return _cmd_default(inp);
    }
}

static gboolean
_cmd_connect(const char * const inp, struct cmd_help_t help)
{
    gboolean result = FALSE;
    jabber_conn_status_t conn_status = jabber_connection_status();

    if ((conn_status != JABBER_DISCONNECTED) && (conn_status != JABBER_STARTED)) {
        cons_show("You are either connected already, or a login is in process.");
        result = TRUE;
    } else if (strlen(inp) < 10) {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
        result = TRUE;
    } else {
        char *user, *lower;
        user = strndup(inp+9, strlen(inp)-9);
        lower = g_utf8_strdown(user, -1);

        status_bar_get_password();
        status_bar_refresh();
        char passwd[21];
        inp_block();
        inp_get_password(passwd);
        inp_non_block();
        
        conn_status = jabber_connect(lower, passwd);
        if (conn_status == JABBER_CONNECTING)
            cons_show("Connecting...");
        if (conn_status == JABBER_DISCONNECTED)
            cons_bad_show("Connection to server failed.");

        result = TRUE;
    }
    
    return result;
}

static gboolean
_cmd_quit(const char * const inp, struct cmd_help_t help)
{
    return FALSE;
}

static gboolean
_cmd_help(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/help") == 0) {
        cons_help();
    } else {
        char *cmd = strndup(inp+6, strlen(inp)-6);
        char cmd_with_slash[1 + strlen(cmd) + 1];
        sprintf(cmd_with_slash, "/%s", cmd);

        const gchar **help_text = NULL;
        struct cmd_t *command = _cmd_get_command(cmd_with_slash);

        if (command != NULL) {
            help_text = command->help.long_help;
        }
        
        cons_show("");

        if (help_text != NULL) {
            int i;
            for (i = 0; help_text[i] != NULL; i++) {
                cons_show(help_text[i]);
            }
        } else {
            cons_show("No such command.");
        }

        cons_show("");
    }

    return TRUE;
}

static gboolean
_cmd_prefs(const char * const inp, struct cmd_help_t help)
{
    cons_prefs();

    return TRUE;
}

static gboolean
_cmd_ros(const char * const inp, struct cmd_help_t help)
{
    jabber_conn_status_t conn_status = jabber_connection_status();

    if (conn_status != JABBER_CONNECTED)
        cons_show("You are not currently connected.");
    else
        jabber_roster_request();

    return TRUE;
}

static gboolean
_cmd_who(const char * const inp, struct cmd_help_t help)
{
    jabber_conn_status_t conn_status = jabber_connection_status();

    if (conn_status != JABBER_CONNECTED) {
        cons_show("You are not currently connected.");
    } else {
        GSList *list = get_contact_list();
        cons_show_online_contacts(list);
    }

    return TRUE;
}

static gboolean
_cmd_msg(const char * const inp, struct cmd_help_t help)
{
    char *usr = NULL;
    char *msg = NULL;

    jabber_conn_status_t conn_status = jabber_connection_status();

    if (conn_status != JABBER_CONNECTED) {
        cons_show("You are not currently connected.");
    } else {
        // copy input    
        char inp_cpy[strlen(inp) + 1];
        strcpy(inp_cpy, inp);

        // get user
        strtok(inp_cpy, " ");
        usr = strtok(NULL, " ");
        if ((usr != NULL) && (strlen(inp) > (5 + strlen(usr) + 1))) {
            // get message
            msg = strndup(inp+5+strlen(usr)+1, strlen(inp)-(5+strlen(usr)+1));
            if (msg != NULL) {
                jabber_send(msg, usr);
                win_show_outgoing_msg("me", usr, msg);
            } else {
                char usage[strlen(help.usage + 8)];
                sprintf(usage, "Usage: %s", help.usage);
                cons_show(usage);
            }
        } else {
            char usage[strlen(help.usage + 8)];
            sprintf(usage, "Usage: %s", help.usage);
            cons_show(usage);
        }
    }

    return TRUE;
}

static gboolean
_cmd_tiny(const char * const inp, struct cmd_help_t help)
{
    if (strlen(inp) > 6) {
        char *url = strndup(inp+6, strlen(inp)-6);

        if (!tinyurl_valid(url)) {
            GString *error = g_string_new("/tiny, badly formed URL: ");
            g_string_append(error, url);
            cons_bad_show(error->str);
            if (win_in_chat()) {
                win_bad_show(error->str);
            }
            g_string_free(error, TRUE);
            free(url);
        } else if (win_in_chat()) {
            char *tiny = tinyurl_get(url);
            char *recipient = win_get_recipient();
            jabber_send(tiny, recipient);
            win_show_outgoing_msg("me", recipient, tiny);
            free(recipient);
            free(tiny);
            free(url);
        } else {
            cons_bad_command(inp);
            free(url);
        }
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
        
        if (win_in_chat()) {
            char usage[strlen(help.usage + 8)];
            sprintf(usage, "Usage: %s", help.usage);
            win_show(usage);
        }
    }

    return TRUE;
}

static gboolean
_cmd_close(const char * const inp, struct cmd_help_t help)
{
    if (!win_close_win())
        cons_bad_command(inp);
    
    return TRUE;
}

static gboolean
_cmd_set_beep(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/beep on") == 0) {
        cons_show("Sound enabled.");
        prefs_set_beep(TRUE);
    } else if (strcmp(inp, "/beep off") == 0) {
        cons_show("Sound disabled.");
        prefs_set_beep(FALSE);
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
    }        

    return TRUE;
}

static gboolean
_cmd_set_notify(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/notify on") == 0) {
        cons_show("Desktop notifications enabled.");
        prefs_set_notify(TRUE);
    } else if (strcmp(inp, "/notify off") == 0) {
        cons_show("Desktop notifications disabled.");
        prefs_set_notify(FALSE);
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
    }        

    return TRUE;
}

static gboolean
_cmd_set_typing(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/typing on") == 0) {
        cons_show("Incoming typing notifications enabled.");
        prefs_set_typing(TRUE);
    } else if (strcmp(inp, "/typing off") == 0) {
        cons_show("Incoming typing notifications disabled.");
        prefs_set_typing(FALSE);
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
    }        

    return TRUE;
}

static gboolean
_cmd_set_flash(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/flash on") == 0) {
        cons_show("Screen flash enabled.");
        prefs_set_flash(TRUE);
    } else if (strcmp(inp, "/flash off") == 0) {
        cons_show("Screen flash disabled.");
        prefs_set_flash(FALSE);
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
    }        

    return TRUE;
}

static gboolean
_cmd_set_showsplash(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/showsplash on") == 0) {
        cons_show("Splash screen enabled.");
        prefs_set_showsplash(TRUE);
    } else if (strcmp(inp, "/showsplash off") == 0) {
        cons_show("Splash screen disabled.");
        prefs_set_showsplash(FALSE);
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
    }

    return TRUE;
}

static gboolean
_cmd_set_chlog(const char * const inp, struct cmd_help_t help)
{
    if (strcmp(inp, "/chlog on") == 0) {
        cons_show("Chat logging enabled.");
        prefs_set_chlog(TRUE);
    } else if (strcmp(inp, "/chlog off") == 0) {
        cons_show("Chat logging disabled.");
        prefs_set_chlog(FALSE);
    } else {
        char usage[strlen(help.usage + 8)];
        sprintf(usage, "Usage: %s", help.usage);
        cons_show(usage);
    }

    return TRUE;
}

static gboolean
_cmd_away(const char * const inp, struct cmd_help_t help)
{
    _update_presence(PRESENCE_AWAY, "away", inp);
    return TRUE;
}

static gboolean
_cmd_online(const char * const inp, struct cmd_help_t help)
{
    _update_presence(PRESENCE_ONLINE, "online", inp);
    return TRUE;
}

static gboolean
_cmd_dnd(const char * const inp, struct cmd_help_t help)
{
    _update_presence(PRESENCE_DND, "dnd", inp);
    return TRUE;
}

static gboolean
_cmd_chat(const char * const inp, struct cmd_help_t help)
{
    _update_presence(PRESENCE_CHAT, "chat", inp);
    return TRUE;
}

static gboolean
_cmd_xa(const char * const inp, struct cmd_help_t help)
{
    _update_presence(PRESENCE_XA, "xa", inp);
    return TRUE;
}

static gboolean
_cmd_default(const char * const inp)
{
    if (win_in_chat()) {
        char *recipient = win_get_recipient();
        jabber_send(inp, recipient);
        win_show_outgoing_msg("me", recipient, inp);
        free(recipient);
    } else {
        cons_bad_command(inp);
    }

    return TRUE;
}

static void
_update_presence(const jabber_presence_t presence, 
    const char * const show, const char * const inp)
{
    char *msg;
    if (strlen(inp) > strlen(show) + 2) {
        msg = strndup(inp+(strlen(show) + 2), strlen(inp)-(strlen(show) + 2));
    } else {
        msg = NULL;
    }

    jabber_conn_status_t conn_status = jabber_connection_status();
    
    if (conn_status != JABBER_CONNECTED) {
        cons_show("You are not currently connected.");
    } else {
        jabber_update_presence(presence, msg);
        title_bar_set_status(presence);
        if (msg != NULL) {
            char str[14 + strlen(show) + 3 + strlen(msg) + 2];
            sprintf(str, "Status set to %s, \"%s\"", show, msg);
            cons_show(str);
            free(msg);
        } else {
            char str[14 + strlen(show) + 1];
            sprintf(str, "Status set to %s", show);
            cons_show(str);
        }
    }

} 
