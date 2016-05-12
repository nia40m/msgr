#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <form.h>

int main(int argc, char const *argv[])
{
	FIELD *my_field[2];
	FORM *my_form;
	int ch;
	char str[255] = "";

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	my_field[0] = new_field(1, COLS - 4, LINES - 2, 2, 13, 0);
	my_field[1] = NULL;

	set_field_back(my_field[0], A_UNDERLINE);
	field_opts_off(my_field[0], O_AUTOSKIP);
	field_opts_off(my_field[0], O_WRAP);	// Disable wrap by words

	my_form = new_form(my_field);
	post_form(my_form);
	refresh();

	mvprintw(LINES - 2, 0, ">");
	refresh();
	pos_form_cursor(my_form);

	/* Loop through to get user requests */
	while ((ch = getch()) != '\n') {
		switch (ch) {
		case KEY_F(2):
			/* Update field buffer */
			form_driver(my_form, REQ_VALIDATION);

			move(2, 2);
			printw("%s", field_buffer(my_field[0], 0));

			form_driver(my_form, REQ_CLR_FIELD);
			refresh();

			pos_form_cursor(my_form);
			break;
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

	unpost_form(my_form);
	free_form(my_form);
	free_field(my_field[0]);

	endwin();

	puts(str);

	return 0;
}