#ifndef _COMMON_H
#define _COMMON_H

#define MSGR_PORT 1488

#define BUFF_SIZE 4096
#define STR_LNGTH 256
#define EXTRA STR_LNGTH
#define MSG_BUFF BUFF_SIZE+STR_LNGTH+EXTRA

struct clnt_info {
    char name[STR_LNGTH];
    char room[STR_LNGTH];
};

enum states {
    /* connection was established */
    ST_OK = 0,
    /* login is already taken */
    ST_LBUSSY,
    /* an unknown error has occured */
    ST_ERROR
};

#endif
