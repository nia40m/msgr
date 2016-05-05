#ifndef _COMMON_H
#define _COMMON_H

#define MSGR_PORT 1488
#define STR_LNGTH 256

struct clnt_info {
    char name[STR_LNGTH];
    char room[STR_LNGTH];
};

#endif