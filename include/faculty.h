/*
 * faculty.h
 *
 * Data model for a professor's record as read from the faculty /
 * professor-information input file. This file supplies the pieces of
 * information that cannot be derived from the class schedule alone:
 * the professor's display name, department, and the office-hours
 * policy this project uses to place office-hour blocks around that
 * professor's teaching schedule.
 */
#ifndef FACULTY_H
#define FACULTY_H

#include "common.h"

typedef struct {
    char id[MAX_NAME_LEN];        /* short key, matches Course.lecturer_id */
    char display_name[MAX_NAME_LEN]; /* e.g. "Dr. A. Smith"              */
    char department[MAX_NAME_LEN];   /* e.g. "Computer Science"          */

    /* Office-hours policy for this professor, supplied by the lecturer
     * (this is the "office-hours table provided by the lecturer" the
     * assignment refers to). Both are expressed in minutes. */
    int  daytime_block_end_min;   /* daytime office hours run from the
                                      end of a morning/early class up to
                                      this clock time (e.g. 13:00)       */
    int  evening_block_len_min;   /* length of the office-hour block
                                      placed immediately before an
                                      evening class (e.g. 120)           */
    int  evening_cutoff_min;      /* a class starting at or after this
                                      clock time is treated as "evening"
                                      for office-hour placement purposes
                                      (e.g. 16:00)                       */
} Faculty;

typedef struct {
    Faculty items[MAX_FACULTY];
    int count;
} FacultyList;

/* Loads the faculty-information file at path into out. Each
 * non-comment, non-blank line has the pipe-delimited format:
 *   ID|DISPLAY_NAME|DEPARTMENT|DAYTIME_END|EVENING_LEN|EVENING_CUTOFF
 * e.g.
 *   jsmith|Dr. J. Smith|Computer Science|13:00|120|16:00
 * Returns RC_OK, or an error code if the file cannot be opened or a
 * line fails to parse. */
ResultCode faculty_list_load(const char *path, FacultyList *out);

/* Finds a faculty record by id. Returns NULL if not found. */
const Faculty *faculty_find(const FacultyList *list, const char *id);

#endif /* FACULTY_H */
