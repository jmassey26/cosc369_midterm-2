/*
 * output.c
 *
 * Renders a faculty member's CLASS SCHEDULE and OFFICE HOURS tables as
 * plain text (styled after the sample in the assignment) and as CSV
 * rows for the Python -> Word document step.
 */
#include <stdio.h>
#include <string.h>
#include "output.h"

static const char *WEEKDAY_NAMES[5] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
};

/* Turns a day-code string like "TR" or "MW" into "Tuesday & Thursday"
 * / "Monday & Wednesday" for the CLASS SCHEDULE table, matching the
 * style of the sample output. */
static void spell_days(const char *days, char *buf, size_t buf_size)
{
    static const char codes[5]  = { 'M', 'T', 'W', 'R', 'F' };
    buf[0] = '\0';
    int first = 1;
    for (int i = 0; i < 5; i++) {
        if (strchr(days, codes[i]) == NULL)
            continue;
        if (!first)
            strncat(buf, " & ", buf_size - strlen(buf) - 1);
        strncat(buf, WEEKDAY_NAMES[i], buf_size - strlen(buf) - 1);
        first = 0;
    }
}

static void format_range(int start_min, int end_min, char *buf, size_t buf_size)
{
    char start_s[16], end_s[16];
    minutes_to_ampm(start_min, start_s, sizeof(start_s));
    minutes_to_ampm(end_min, end_s, sizeof(end_s));
    snprintf(buf, buf_size, "%s to %s", start_s, end_s);
}

void output_write_report(FILE *fp, const CourseList *all_courses,
                          const FacultySchedule *sched)
{
    const Faculty *f = sched->faculty;

    fprintf(fp, "FACULTY: %s (%s)\n", f->display_name, f->department);
    fprintf(fp, "================================================================\n\n");

    fprintf(fp, "CLASS SCHEDULE\n");
    fprintf(fp, "%-28s %-38s %s\n", "Course", "Times", "Location");
    for (int i = 0; i < all_courses->count; i++) {
        const Course *c = &all_courses->items[i];
        if (strcmp(c->lecturer_id, f->id) != 0)
            continue;

        char days_spelled[64];
        char timerange[48];
        char coursecol[MAX_TITLE_LEN + MAX_CODE_LEN];
        spell_days(c->days, days_spelled, sizeof(days_spelled));
        format_range(c->start_min, c->end_min, timerange, sizeof(timerange));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
        snprintf(coursecol, sizeof(coursecol), "%s (%s)", c->code, c->title);
#pragma GCC diagnostic pop

        char timescol[128];
        snprintf(timescol, sizeof(timescol), "%s  %s", days_spelled, timerange);
        fprintf(fp, "%-28s %-38s %s\n", coursecol, timescol, c->room);
    }

    fprintf(fp, "\nOFFICE HOURS\n");
    for (int d = 0; d < 5; d++) {
        const DaySchedule *ds = &sched->days[d];
        fprintf(fp, "\t%s: ", WEEKDAY_NAMES[d]);

        int printed = 0;
        for (int i = 0; i < ds->block_count; i++) {
            const DayBlock *b = &ds->blocks[i];
            if (b->kind != BLOCK_OFFICE_HOURS)
                continue;

            char start_s[16], end_s[16];
            minutes_to_ampm(b->start_min, start_s, sizeof(start_s));
            minutes_to_ampm(b->end_min, end_s, sizeof(end_s));

            if (printed)
                fprintf(fp, "\n\t\t\t& ");
            fprintf(fp, "%s - %s (%s)", start_s, end_s, b->label);
            printed = 1;
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}

void output_write_csv(FILE *fp, const FacultySchedule *sched, int write_header)
{
    const Faculty *f = sched->faculty;

    if (write_header)
        fprintf(fp, "faculty,day,kind,start,end,label,room\n");

    for (int d = 0; d < 5; d++) {
        const DaySchedule *ds = &sched->days[d];
        for (int i = 0; i < ds->block_count; i++) {
            const DayBlock *b = &ds->blocks[i];
            char start_s[16], end_s[16];
            minutes_to_ampm(b->start_min, start_s, sizeof(start_s));
            minutes_to_ampm(b->end_min, end_s, sizeof(end_s));
            fprintf(fp, "\"%s\",%s,%s,%s,%s,\"%s\",\"%s\"\n",
                    f->display_name,
                    WEEKDAY_NAMES[d],
                    (b->kind == BLOCK_CLASS) ? "class" : "office_hours",
                    start_s, end_s, b->label, b->room);
        }
    }
}
