/*
 * scheduler.h
 *
 * Builds, per faculty member, the two tables the assignment asks for:
 *   1. that professor's weekly class-meeting schedule, and
 *   2. a derived weekly office-hours schedule.
 *
 * Office-hour placement rule (documented in the report, section 4):
 * for each weekday, look at the professor's class meetings that day.
 *   - If the last "daytime" meeting (one that starts before that
 *     professor's evening_cutoff_min) ends by early afternoon, an
 *     office-hour block is placed from that end time until the
 *     professor's daytime_block_end_min.
 *   - If the professor has a class that starts at or after
 *     evening_cutoff_min, a second office-hour block of length
 *     evening_block_len_min is placed immediately BEFORE that class.
 *   - A day with no class meetings gets no office-hour block.
 *   - All generated blocks are clamped to CAMPUS_OPEN_MINUTES /
 *     CAMPUS_CLOSE_MINUTES so a professor's schedule can never produce
 *     an office hour before 8 AM or after 6 PM.
 * This mirrors the sample office-hours table supplied with the
 * assignment: a class ending at 10:45 AM produces office hours from
 * 10:45 AM to 1:00 PM, and an evening class at 5:00 PM produces office
 * hours from 3:00 PM to 5:00 PM.
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"
#include "course.h"
#include "faculty.h"

typedef enum { BLOCK_CLASS, BLOCK_OFFICE_HOURS } BlockKind;

typedef struct {
    BlockKind kind;
    int start_min;
    int end_min;
    char label[MAX_TITLE_LEN];  /* course code (+ title for class blocks) */
    char room[MAX_ROOM_LEN];    /* empty for office-hour blocks           */
} DayBlock;

typedef struct {
    char day;                   /* 'M','T','W','R','F'                   */
    DayBlock blocks[MAX_DAILY_BLOCKS];
    int block_count;
} DaySchedule;

typedef struct {
    const Faculty *faculty;     /* borrowed pointer into a FacultyList   */
    DaySchedule days[5];        /* Monday .. Friday, fixed order          */
} FacultySchedule;

/* Builds the full week (classes + generated office hours) for a single
 * faculty member from that faculty member's Course entries in
 * `courses`. Returns RC_OK, or RC_ERR_CAPACITY if a day overflows
 * MAX_DAILY_BLOCKS. */
ResultCode scheduler_build(const Faculty *faculty,
                           const CourseList *courses,
                           FacultySchedule *out);

#endif /* SCHEDULER_H */
