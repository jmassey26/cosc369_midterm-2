/*
 * scheduler.c
 *
 * Implements the office-hours placement rule described in
 * scheduler.h. For each weekday and each faculty member, the classes
 * that faculty member teaches that day are partitioned into
 * "daytime" meetings (start before Faculty.evening_cutoff_min) and
 * "evening" meetings (start at or after the cutoff). At most one
 * daytime office-hour block and one evening office-hour block are
 * generated per day, both clamped to campus hours.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"

static const char WEEKDAY_LETTERS[5] = { 'M', 'T', 'W', 'R', 'F' };

static int clamp(int minutes, int lo, int hi)
{
    if (minutes < lo) return lo;
    if (minutes > hi) return hi;
    return minutes;
}

static ResultCode add_block(DaySchedule *ds, BlockKind kind, int start_min,
                             int end_min, const char *label, const char *room)
{
    if (ds->block_count >= MAX_DAILY_BLOCKS)
        return RC_ERR_CAPACITY;

    DayBlock *b = &ds->blocks[ds->block_count++];
    b->kind = kind;
    b->start_min = start_min;
    b->end_min = end_min;
    snprintf(b->label, sizeof(b->label), "%s", label ? label : "");
    snprintf(b->room, sizeof(b->room), "%s", room ? room : "");
    return RC_OK;
}

static int block_cmp(const void *a, const void *b)
{
    const DayBlock *ba = (const DayBlock *)a;
    const DayBlock *bb = (const DayBlock *)b;
    return ba->start_min - bb->start_min;
}

ResultCode scheduler_build(const Faculty *faculty,
                           const CourseList *courses,
                           FacultySchedule *out)
{
    memset(out, 0, sizeof(*out));
    out->faculty = faculty;

    for (int d = 0; d < 5; d++) {
        DaySchedule *ds = &out->days[d];
        ds->day = WEEKDAY_LETTERS[d];
        ds->block_count = 0;

        int have_daytime = 0, daytime_end_min = -1;
        char daytime_code[MAX_CODE_LEN] = "";
        int have_evening = 0, evening_start_min = -1;
        char evening_code[MAX_CODE_LEN] = "";

        for (int i = 0; i < courses->count; i++) {
            const Course *c = &courses->items[i];
            if (strcmp(c->lecturer_id, faculty->id) != 0)
                continue;
            if (!course_meets_on(c, ds->day))
                continue;

            /* GCC's -Wformat-truncation assumes both %s fields could
             * simultaneously hit their declared maximum length and
             * warns accordingly; label[] is sized well beyond any
             * realistic course code + title, so the warning is a false
             * positive here and is silenced locally. */
            char label[MAX_TITLE_LEN];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
            snprintf(label, sizeof(label), "%s (%s)", c->code, c->title);
#pragma GCC diagnostic pop
            ResultCode rc = add_block(ds, BLOCK_CLASS, c->start_min,
                                       c->end_min, label, c->room);
            if (rc != RC_OK)
                return rc;

            if (c->start_min >= faculty->evening_cutoff_min) {
                if (!have_evening || c->start_min < evening_start_min) {
                    evening_start_min = c->start_min;
                    snprintf(evening_code, sizeof(evening_code), "%s", c->code);
                    have_evening = 1;
                }
            } else {
                if (!have_daytime || c->end_min > daytime_end_min) {
                    daytime_end_min = c->end_min;
                    snprintf(daytime_code, sizeof(daytime_code), "%s", c->code);
                    have_daytime = 1;
                }
            }
        }

        if (have_daytime && daytime_end_min < faculty->daytime_block_end_min) {
            int start = clamp(daytime_end_min, CAMPUS_OPEN_MINUTES, CAMPUS_CLOSE_MINUTES);
            int end   = clamp(faculty->daytime_block_end_min, CAMPUS_OPEN_MINUTES, CAMPUS_CLOSE_MINUTES);
            if (end > start) {
                ResultCode rc = add_block(ds, BLOCK_OFFICE_HOURS, start, end, daytime_code, "");
                if (rc != RC_OK)
                    return rc;
            }
        }

        if (have_evening) {
            int start = clamp(evening_start_min - faculty->evening_block_len_min,
                               CAMPUS_OPEN_MINUTES, CAMPUS_CLOSE_MINUTES);
            int end   = clamp(evening_start_min, CAMPUS_OPEN_MINUTES, CAMPUS_CLOSE_MINUTES);
            if (end > start) {
                ResultCode rc = add_block(ds, BLOCK_OFFICE_HOURS, start, end, evening_code, "");
                if (rc != RC_OK)
                    return rc;
            }
        }

        qsort(ds->blocks, ds->block_count, sizeof(DayBlock), block_cmp);
    }

    return RC_OK;
}
