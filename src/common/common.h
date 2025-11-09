#ifndef COMMON_H
#define COMMON_H

typedef enum {
    INITIALIZATION_OK = 0,
    INITIALIZATION_ERROR = -1
} InitializationStatusEnum;

#define ALLEGRO_INIT_OK 0

#define PROGRAM_OK 0
#define PROGRAM_ERROR 1

#ifndef uchar
#define uchar   unsigned char
#endif

#endif /* COMMON_H */
