#ifndef _TUI_H
#define _TUI_H

int tui_init(void);
void tui_end();
void tui_add_msg(const char *name, const char *msg);
void tui_get_str(char *s, int size);

#endif