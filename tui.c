#define _POSIX_C_SOURCE 2

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <form.h>
#include <time.h>
#include "tui.h"

WINDOW *msglog;
FIELD *my_field[2];
FORM *my_form;

/*
 * Trim leading and trailing whitespace.
 */
static char *strtrim(char *str)
{
	char *end;

	while (isspace(*str))
		++str;

	/* Blank string? */
	if (*str == '\0')
		return str;

	/* Trim trailing whitespace */
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		--end;

	/* Null-terminate the string */
	*(end + 1) = '\0';

	return str;
}

/*
 * Initialization of TUI.
 */
int tui_init(void)
{
	if (initscr() == NULL)
		return -1;

	/* Init colors if possible */
	if (has_colors())
		start_color();

	/* Create color pair for nickname */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);

	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	/* Create subwindow to display messages log */
	msglog = subwin(stdscr, LINES - 2, COLS, 0, 0);

	/* Enable scroll in messages log */
	scrollok(msglog, 1);

	my_field[0] = new_field(1, COLS - 4, LINES - 2, 2, 13, 0);
	my_field[1] = NULL;

	set_field_back(my_field[0], COLOR_PAIR(2) | A_UNDERLINE);
	field_opts_off(my_field[0], O_AUTOSKIP);
	field_opts_off(my_field[0], O_WRAP);	// Disable wrap by words

	my_form = new_form(my_field);
	post_form(my_form);
	refresh();

	mvprintw(LINES - 2, 0, ">");
	refresh();
	pos_form_cursor(my_form);

	return 0;
}

/*
 * Free all allocated resources.
 * This must always be called after using TUI.
 */
void tui_end()
{
	unpost_form(my_form);
	free_form(my_form);
	free_field(my_field[0]);
	endwin();
}

/*
 * Add message to messages log.
 */
void tui_add_msg(const char *name, const char *msg)
{
	time_t cur_time;
	struct tm tm_struct;

	time(&cur_time);
	localtime_r(&cur_time, &tm_struct);

	wattron(msglog, COLOR_PAIR(3));
	wprintw(msglog, "[%02d:%02d:%02d] ",
		tm_struct.tm_hour, tm_struct.tm_min, tm_struct.tm_sec);
	wattroff(msglog, COLOR_PAIR(3));

	if (name != NULL) {
		wattron(msglog, COLOR_PAIR(1) | A_BOLD);
		wprintw(msglog, "%s", name);
		wattroff(msglog, A_BOLD);
		wprintw(msglog, " -> ");
		wattroff(msglog, COLOR_PAIR(1));
	}

	if (msg != NULL)
		wprintw(msglog, "%s\n", msg);

	wrefresh(msglog);
	pos_form_cursor(my_form);
}

/*
 * Locking call, copies string from input field to s.
 */
void tui_get_str(char *s, int size)
{
	int ch;

	while ((ch = getch()) != '\n') {
		switch (ch) {
		case KEY_LEFT:
			form_driver(my_form, REQ_PREV_CHAR);
			break;
		case KEY_RIGHT:
			form_driver(my_form, REQ_NEXT_CHAR);
			break;
		case KEY_BACKSPACE:
			form_driver(my_form, REQ_DEL_PREV);
			break;
		case KEY_DC:
			form_driver(my_form, REQ_DEL_CHAR);
			break;
		default:
			form_driver(my_form, ch);
			break;
		}
	}

	/* Update field buffer */
	form_driver(my_form, REQ_VALIDATION);

	strncpy(s, strtrim(field_buffer(my_field[0], 0)), size);
	form_driver(my_form, REQ_CLR_FIELD);
}
