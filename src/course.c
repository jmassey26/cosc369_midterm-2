/*
 * course.c
 *
 * Parses the syllabus / class-schedule input file into a CourseList.
 * File format (one class meeting per line, '#' starts a comment,
 * blank lines are ignored):
 *
 *   CODE|TITLE|LECTURER_ID|DAYS|START|END|ROOM
 *
 * Example:
 *   COSC369|Software Engineering I|jsmith|TR|09:30|10:45|CS 108
 */
#include <stdio.h>
#include <string.h>
#include "course.h"

static ResultCode parse_course_line(char *line, Course *c)
{
    char *field[7];
    int n = 0;
    char *tok = strtok(line, "|");
    while (tok != NULL && n < 7) {
        field[n++] = trim(tok);
        tok = strtok(NULL, "|");
    }
    if (n != 7)
        return RC_ERR_PARSE;

    snprintf(c->code, sizeof(c->code), "%s", field[0]);
    snprintf(c->title, sizeof(c->title), "%s", field[1]);
    snprintf(c->lecturer_id, sizeof(c->lecturer_id), "%s", field[2]);
    snprintf(c->days, sizeof(c->days), "%s", field[3]);

    c->start_min = time_to_minutes(field[4]);
    c->end_min   = time_to_minutes(field[5]);
    if (c->start_min < 0 || c->end_min < 0 || c->end_min <= c->start_min)
        return RC_ERR_PARSE;

    snprintf(c->room, sizeof(c->room), "%s", field[6]);
    return RC_OK;
}

ResultCode course_list_load(const char *path, CourseList *out)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        return RC_ERR_FILE_OPEN;

    out->count = 0;
    char raw[MAX_LINE];
    int lineno = 0;
    ResultCode rc = RC_OK;

    while (fgets(raw, sizeof(raw), fp)) {
        lineno++;
        char *line = trim(raw);
        if (line[0] == '\0' || line[0] == '#')
            continue;

        if (out->count >= MAX_COURSES) {
            fprintf(stderr,
                    "course_list_load: %s exceeds MAX_COURSES (%d) at line %d\n",
                    path, MAX_COURSES, lineno);
            rc = RC_ERR_CAPACITY;
            break;
        }

        Course c;
        char linecopy[MAX_LINE];
        snprintf(linecopy, sizeof(linecopy), "%s", line);
        if (parse_course_line(linecopy, &c) != RC_OK) {
            fprintf(stderr,
                    "course_list_load: malformed line %d in %s: \"%s\"\n",
                    lineno, path, line);
            rc = RC_ERR_PARSE;
            continue;
        }
        out->items[out->count++] = c;
    }

    fclose(fp);
    return rc;
}

int course_meets_on(const Course *c, char day)
{
    return strchr(c->days, day) != NULL;
}
