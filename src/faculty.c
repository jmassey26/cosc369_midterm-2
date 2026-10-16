/*
 * faculty.c
 *
 * Parses the faculty / professor-information input file into a
 * FacultyList. File format (one professor per line, '#' starts a
 * comment, blank lines are ignored):
 *
 *   ID|DISPLAY_NAME|DEPARTMENT|DAYTIME_END|EVENING_LEN|EVENING_CUTOFF
 *
 * Example:
 *   jsmith|Dr. J. Smith|Computer Science|13:00|120|16:00
 *
 * DAYTIME_END and EVENING_CUTOFF are "HH:MM" clock times; EVENING_LEN
 * is an integer number of minutes. These three fields are the
 * "office-hours table provided by the lecturer" referenced in the
 * assignment: each professor sets their own policy instead of the
 * program hard-coding one.
 */
#include <stdio.h>
#include <string.h>
#include "faculty.h"

static ResultCode parse_faculty_line(char *line, Faculty *f)
{
    char *field[6];
    int n = 0;
    char *tok = strtok(line, "|");
    while (tok != NULL && n < 6) {
        field[n++] = trim(tok);
        tok = strtok(NULL, "|");
    }
    if (n != 6)
        return RC_ERR_PARSE;

    snprintf(f->id, sizeof(f->id), "%s", field[0]);
    snprintf(f->display_name, sizeof(f->display_name), "%s", field[1]);
    snprintf(f->department, sizeof(f->department), "%s", field[2]);

    f->daytime_block_end_min = time_to_minutes(field[3]);
    if (f->daytime_block_end_min < 0)
        return RC_ERR_PARSE;

    if (sscanf(field[4], "%d", &f->evening_block_len_min) != 1 ||
        f->evening_block_len_min <= 0)
        return RC_ERR_PARSE;

    f->evening_cutoff_min = time_to_minutes(field[5]);
    if (f->evening_cutoff_min < 0)
        return RC_ERR_PARSE;

    return RC_OK;
}

ResultCode faculty_list_load(const char *path, FacultyList *out)
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

        if (out->count >= MAX_FACULTY) {
            fprintf(stderr,
                    "faculty_list_load: %s exceeds MAX_FACULTY (%d) at line %d\n",
                    path, MAX_FACULTY, lineno);
            rc = RC_ERR_CAPACITY;
            break;
        }

        Faculty f;
        char linecopy[MAX_LINE];
        snprintf(linecopy, sizeof(linecopy), "%s", line);
        if (parse_faculty_line(linecopy, &f) != RC_OK) {
            fprintf(stderr,
                    "faculty_list_load: malformed line %d in %s: \"%s\"\n",
                    lineno, path, line);
            rc = RC_ERR_PARSE;
            continue;
        }
        out->items[out->count++] = f;
    }

    fclose(fp);
    return rc;
}

const Faculty *faculty_find(const FacultyList *list, const char *id)
{
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->items[i].id, id) == 0)
            return &list->items[i];
    }
    return NULL;
}
