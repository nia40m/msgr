#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <form.h>

WINDOW *msglog;
FIELD *my_field[2];
FORM *my_form;

char buf[BUFSIZ];

char *strtrim(char *str)
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

void tui_end()
{
	unpost_form(my_form);
	free_form(my_form);
	free_field(my_field[0]);
	endwin();
}

void tui_add_msg(const char *name, const char *msg)
{
	wattron(msglog, COLOR_PAIR(1));
	wprintw(msglog, "%s -> ", name);
	wattroff(msglog, COLOR_PAIR(1));

	wprintw(msglog, "%s\n", msg);
	wrefresh(msglog);
	pos_form_cursor(my_form);
}

/* Locking call, copies string from input field to s */
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

int main(int argc, char const *argv[])
{
	char buf[BUFSIZ] = "";

	tui_init();

	while (1) {
		tui_get_str(buf, BUFSIZ - 1);

		if (buf[0] == '/' && strcmp(buf, "/exit") == 0)
			break;

		tui_add_msg("me", buf);
	}

	tui_end();

	return 0;
}