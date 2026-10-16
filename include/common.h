/*
 * common.h
 *
 * Shared constants, limits, and small utility declarations used across
 * every module in the schedule generator. Kept separate from the
 * domain-specific headers (course.h, faculty.h, ...) so that any module
 * can include it without pulling in unrelated struct definitions.
 */
#ifndef COMMON_H
#define COMMON_H

#define MAX_LINE          512
#define MAX_NAME_LEN       64
#define MAX_CODE_LEN       32
#define MAX_TITLE_LEN     160
#define MAX_ROOM_LEN       32
#define MAX_DAYS_LEN       16   /* e.g. "MW", "TR", "MWF" */
#define MAX_TIME_LEN       16   /* e.g. "09:30" (24h, zero padded) */

#define MAX_COURSES       128
#define MAX_FACULTY        32
#define MAX_COURSES_PER_FACULTY 16
#define MAX_DAILY_BLOCKS   16

/* Standard campus business-hour bounds used when clamping generated
 * office-hour blocks (see scheduler.h for the rules that use these). */
#define CAMPUS_OPEN_MINUTES   (8 * 60)         /* 8:00 AM  */
#define CAMPUS_CLOSE_MINUTES  (18 * 60)        /* 6:00 PM  */

/* Return codes used throughout the program instead of raw ints, so the
 * meaning of a failure is visible at every call site. */
typedef enum {
    RC_OK = 0,
    RC_ERR_FILE_OPEN,
    RC_ERR_PARSE,
    RC_ERR_CAPACITY,
    RC_ERR_ARGS
} ResultCode;

/* Converts "HH:MM" (24-hour, zero padded) into minutes-since-midnight.
 * Returns -1 on malformed input. Implemented in util.c */
int time_to_minutes(const char *hhmm);

/* Converts minutes-since-midnight back into a 12-hour "H:MM AM/PM"
 * style string, writing into buf (caller-supplied, at least 12 bytes). */
void minutes_to_ampm(int minutes, char *buf, size_t buf_size);

/* Trims leading/trailing whitespace from a line in place. */
char *trim(char *s);

#endif /* COMMON_H */
