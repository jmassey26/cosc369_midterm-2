/*
 * output.h
 *
 * Renders a FacultySchedule in two forms:
 *   - a human-readable text report (styled after the sample in the
 *     assignment: a "CLASS SCHEDULE" table followed by an
 *     "OFFICE HOURS" list), written to an open FILE*.
 *   - a flat CSV file, one row per block, meant to be consumed by the
 *     Python step (scripts/make_docx.py) that turns the report into a
 *     Word document.
 */
#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>
#include "common.h"
#include "course.h"
#include "scheduler.h"

/* Writes the "CLASS SCHEDULE" + "OFFICE HOURS" text report for one
 * faculty member to fp. `all_courses` is the full parsed course list;
 * this function filters it down to the rows taught by `sched->faculty`
 * for the CLASS SCHEDULE table (grouping same-course meetings that
 * share days/time, e.g. "Tuesday & Thursday"). The OFFICE HOURS list
 * comes from the already-computed blocks in `sched`. */
void output_write_report(FILE *fp, const CourseList *all_courses,
                          const FacultySchedule *sched);

/* Appends one faculty member's blocks as CSV rows to fp. If
 * write_header is non-zero, a header row is written first (callers
 * pass 1 exactly once, before writing the first faculty member). CSV
 * columns: faculty,day,kind,start,end,label,room */
void output_write_csv(FILE *fp, const FacultySchedule *sched, int write_header);

#endif /* OUTPUT_H */
