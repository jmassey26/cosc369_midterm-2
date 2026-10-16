/*
 * course.h
 *
 * Data model for a single class meeting as read from the syllabus /
 * class-schedule input file. One Course record corresponds to one row
 * of the class-schedule table (a course can meet on more than one day;
 * the days are stored as a compact code string like "MW" or "TR").
 */
#ifndef COURSE_H
#define COURSE_H

#include "common.h"

typedef struct {
    char code[MAX_CODE_LEN];      /* e.g. "COSC369"                  */
    char title[MAX_TITLE_LEN];    /* e.g. "Software Engineering I"   */
    char lecturer_id[MAX_NAME_LEN]; /* foreign key into FacultyList  */
    char days[MAX_DAYS_LEN];      /* subset of "MTWRF"                */
    int  start_min;               /* minutes since midnight           */
    int  end_min;                 /* minutes since midnight           */
    char room[MAX_ROOM_LEN];      /* e.g. "CS 108"                    */
} Course;

typedef struct {
    Course items[MAX_COURSES];
    int count;
} CourseList;

/* Loads the class-schedule file at path into out. Each non-comment,
 * non-blank line has the pipe-delimited format:
 *   CODE|TITLE|LECTURER_ID|DAYS|START|END|ROOM
 * e.g.
 *   COSC369|Software Engineering I|jsmith|TR|09:30|10:45|CS 108
 * Returns RC_OK, or an error code if the file cannot be opened or a
 * line fails to parse. */
ResultCode course_list_load(const char *path, CourseList *out);

/* Returns 1 if 'day' (one of 'M','T','W','R','F') appears in c->days. */
int course_meets_on(const Course *c, char day);

#endif /* COURSE_H */
