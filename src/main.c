/*
 * main.c
 *
 * Driver program. Everything reusable (parsing, scheduling, output
 * rendering) lives in libschedgen.a; main.c only wires those pieces
 * together and owns argument handling and top-level file I/O, which
 * is why it is compiled and linked separately from the library rather
 * than being part of it.
 *
 * Usage:
 *   schedgen <class_schedule_file> <faculty_info_file> \
 *            <report_out.txt> <report_out.csv>
 */
#include <stdio.h>
#include "course.h"
#include "faculty.h"
#include "scheduler.h"
#include "output.h"

static void print_usage(const char *argv0)
{
    fprintf(stderr,
        "Usage: %s <class_schedule_file> <faculty_info_file> "
        "<report_out.txt> <report_out.csv>\n", argv0);
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        print_usage(argv[0]);
        return RC_ERR_ARGS;
    }

    const char *class_schedule_path = argv[1];
    const char *faculty_info_path   = argv[2];
    const char *report_txt_path     = argv[3];
    const char *report_csv_path     = argv[4];

    CourseList courses;
    FacultyList faculty;

    if (course_list_load(class_schedule_path, &courses) != RC_OK) {
        fprintf(stderr, "main: failed to load class schedule from %s\n",
                class_schedule_path);
        return RC_ERR_FILE_OPEN;
    }
    if (faculty_list_load(faculty_info_path, &faculty) != RC_OK) {
        fprintf(stderr, "main: failed to load faculty info from %s\n",
                faculty_info_path);
        return RC_ERR_FILE_OPEN;
    }

    FILE *txt = fopen(report_txt_path, "w");
    if (!txt) {
        fprintf(stderr, "main: cannot open %s for writing\n", report_txt_path);
        return RC_ERR_FILE_OPEN;
    }
    FILE *csv = fopen(report_csv_path, "w");
    if (!csv) {
        fprintf(stderr, "main: cannot open %s for writing\n", report_csv_path);
        fclose(txt);
        return RC_ERR_FILE_OPEN;
    }

    for (int i = 0; i < faculty.count; i++) {
        FacultySchedule sched;
        ResultCode rc = scheduler_build(&faculty.items[i], &courses, &sched);
        if (rc != RC_OK) {
            fprintf(stderr,
                    "main: could not build schedule for %s (code %d), skipping\n",
                    faculty.items[i].display_name, rc);
            continue;
        }
        output_write_report(txt, &courses, &sched);
        output_write_csv(csv, &sched, i == 0);
    }

    fclose(txt);
    fclose(csv);

    printf("Generated %s and %s for %d faculty member(s).\n",
           report_txt_path, report_csv_path, faculty.count);
    return RC_OK;
}
