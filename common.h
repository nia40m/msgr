#ifndef _COMMON_H
#define _COMMON_H

#define STR_LNGTH 256

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