#!/usr/bin/env python3
"""
build_report.py

Generates report/COSC369_Project1_Report.pdf from the fixed text below
using reportlab. This script is a one-off authoring tool (not part of
the Makefile pipeline); the PDF it produces is what gets submitted.
"""
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import inch
from reportlab.lib.enums import TA_CENTER
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, PageBreak, Preformatted, ListFlowable, ListItem
)
from reportlab.lib import colors

styles = getSampleStyleSheet()
styles.add(ParagraphStyle(name="TitleCentered", parent=styles["Title"], alignment=TA_CENTER))
styles.add(ParagraphStyle(name="H1", parent=styles["Heading1"], spaceBefore=18, spaceAfter=8))
styles.add(ParagraphStyle(name="H2", parent=styles["Heading2"], spaceBefore=12, spaceAfter=6))
styles.add(ParagraphStyle(name="Body", parent=styles["BodyText"], spaceAfter=8, leading=15))
styles.add(ParagraphStyle(name="CodeBlock", parent=styles["Code"], fontSize=8.3, leading=10.5,
                           backColor=colors.Color(0.95, 0.95, 0.95)))

def P(text):
    return Paragraph(text, styles["Body"])

def H1(text):
    return Paragraph(text, styles["H1"])

def H2(text):
    return Paragraph(text, styles["H2"])

def CODE(text):
    return Preformatted(text, styles["CodeBlock"])

def bullets(items):
    return ListFlowable(
        [ListItem(Paragraph(t, styles["Body"])) for t in items],
        bulletType="bullet", start="•", leftIndent=18,
    )

story = []

story.append(Paragraph("COSC 369 - Software Engineering I", styles["TitleCentered"]))
story.append(Paragraph("Project 1: Faculty Teaching &amp; Office-Hours Schedule Generator",
                        ParagraphStyle(name="Sub", parent=styles["Heading2"], alignment=TA_CENTER)))
story.append(Spacer(1, 8))
story.append(Paragraph("Design and Implementation Report", styles["Heading3"]))
story.append(Spacer(1, 24))

# ---------------------------------------------------------------- 1
story.append(H1("1. Project Overview"))
story.append(P(
    "This project is a C program, <b>schedgen</b>, that reads two required input "
    "files &mdash; a syllabus / class-schedule file and a faculty / professor-information "
    "file &mdash; and produces, for every professor listed, a class-schedule table and "
    "a derived weekly office-hours schedule. The program is built as a set of "
    "reusable modules that are compiled into a static library, plus a small "
    "main program that links against that library. A Makefile drives the entire "
    "build: compiling the modules, archiving the static library, linking the "
    "executable, running it against the two input files, and invoking a Python "
    "script that turns the generated CSV data into a formatted Word document."
))

# ---------------------------------------------------------------- 2
story.append(H1("2. Module Organization"))
story.append(P(
    "The source is split so that each file has one clear responsibility, and so "
    "that the parsing and scheduling logic can be reused (and unit-tested) "
    "independently of the program's entry point:"
))
story.append(bullets([
    "<b>common.h / util.c</b> &mdash; shared constants, the <font face='Courier'>ResultCode</font> "
    "error-reporting type, and small time/string helpers (<font face='Courier'>time_to_minutes</font>, "
    "<font face='Courier'>minutes_to_ampm</font>, <font face='Courier'>trim</font>) used by every other module.",
    "<b>course.h / course.c</b> &mdash; the <font face='Courier'>Course</font> data model (one class "
    "meeting: code, title, lecturer, days, start/end time, room) and the loader that parses "
    "<font face='Courier'>class_schedule.txt</font>.",
    "<b>faculty.h / faculty.c</b> &mdash; the <font face='Courier'>Faculty</font> data model (a professor's "
    "id, display name, department, and office-hours policy) and the loader that parses "
    "<font face='Courier'>faculty_info.txt</font>.",
    "<b>scheduler.h / scheduler.c</b> &mdash; the scheduling logic. For one professor, it walks the parsed "
    "course list, builds a Monday-Friday table of class blocks, and applies the office-hours placement "
    "rule (section 5) to generate office-hour blocks for each day.",
    "<b>output.h / output.c</b> &mdash; rendering only. Turns a computed schedule into the plain-text "
    "CLASS SCHEDULE / OFFICE HOURS report and into CSV rows for the Python step. Contains no parsing "
    "or scheduling logic, so the on-screen/on-file format can change without touching the data model.",
    "<b>main.c</b> &mdash; the only file <i>not</i> included in the static library. It handles command-line "
    "arguments, opens the four files involved (two input, two output), and calls into the library "
    "modules in sequence. Keeping it separate is what makes the library/executable split (section 3) "
    "meaningful: main.c is the \"program\", the library is the \"reusable engine\".",
]))
story.append(P(
    "This mirrors a standard layered design: data-model + parsing modules at the bottom, a "
    "scheduling/business-logic module in the middle, a rendering module above that, and a thin "
    "driver on top. Each module's header file is its contract; a module never reaches into "
    "another module's internals, only through the functions and structs its header declares."
))

# ---------------------------------------------------------------- 3
story.append(H1("3. Static Libraries in This Project"))
story.append(H2("3.1 What a static library is"))
story.append(P(
    "A static library is an archive of pre-compiled object files (<font face='Courier'>.o</font> files). "
    "On Linux/Unix, the conventional format is the <font face='Courier'>.a</font> (\"archive\") file created by "
    "the <font face='Courier'>ar</font> tool. Unlike a shared/dynamic library (<font face='Courier'>.so</font>), "
    "which is loaded at run time and can be shared in memory by several running programs, a static library's "
    "code is copied into the final executable at link time: whatever object code the linker needs to resolve "
    "undefined symbols is pulled out of the archive and embedded directly in the output binary. The result is a "
    "single self-contained executable with no run-time dependency on the archive file being present on the "
    "target machine."
))
story.append(H2("3.2 How this project creates and links its static library"))
story.append(P(
    "Every module except <font face='Courier'>main.c</font> "
    "(<font face='Courier'>util.c</font>, <font face='Courier'>course.c</font>, "
    "<font face='Courier'>faculty.c</font>, <font face='Courier'>scheduler.c</font>, "
    "<font face='Courier'>output.c</font>) is first compiled to an object file with "
    "<font face='Courier'>gcc -c</font>, exactly as any C source file would be. Those five object files are then "
    "archived into <font face='Courier'>lib/libschedgen.a</font> with:"
))
story.append(CODE("ar rcs lib/libschedgen.a obj/util.o obj/course.o obj/faculty.o \\\n"
                   "                        obj/scheduler.o obj/output.o"))
story.append(P(
    "The <font face='Courier'>r</font> flag inserts (or replaces) the given object files in the archive, "
    "<font face='Courier'>c</font> creates the archive if it does not already exist, and "
    "<font face='Courier'>s</font> writes an index of the symbols each object file defines so the linker can "
    "search the archive efficiently instead of scanning every member. "
    "<font face='Courier'>main.c</font> is compiled separately into "
    "<font face='Courier'>obj/main.o</font>, and the executable is produced by linking that one object file "
    "against the archive:"
))
story.append(CODE("gcc -std=c11 -Wall -Wextra -Iinclude -g obj/main.o -Llib -lschedgen -o bin/schedgen"))
story.append(P(
    "<font face='Courier'>-Llib</font> tells the linker to look for libraries in the "
    "<font face='Courier'>lib/</font> directory, and <font face='Courier'>-lschedgen</font> tells it to link "
    "against <font face='Courier'>liba/libschedgen.a</font> "
    "(<font face='Courier'>-l&lt;name&gt;</font> maps to <font face='Courier'>lib&lt;name&gt;.a</font>). The "
    "linker resolves every symbol <font face='Courier'>main.o</font> references &mdash; "
    "<font face='Courier'>course_list_load</font>, <font face='Courier'>faculty_list_load</font>, "
    "<font face='Courier'>scheduler_build</font>, <font face='Courier'>output_write_report</font>, "
    "<font face='Courier'>output_write_csv</font> &mdash; by pulling the corresponding object files out of the "
    "archive and embedding them in <font face='Courier'>bin/schedgen</font>."
))
story.append(H2("3.3 Why a static library here"))
story.append(P(
    "The assignment specifically asks for a static library to demonstrate organizing a project into modules with "
    "a library boundary. A static library is also the natural choice for a single-machine command-line tool like "
    "this one: there is only one executable that ever uses these modules, the program is meant to be copied to a "
    "lab machine and run without an installation step, and there is no benefit here from the memory sharing or "
    "independent upgrade path that a shared library would offer to multiple, separately-installed programs."
))

# ---------------------------------------------------------------- 4
story.append(H1("4. How the Makefile Manages Dependencies and Automation"))
story.append(P(
    "The Makefile expresses the whole build as a dependency graph of targets, each with a rule for how to "
    "produce it from the files it depends on:"
))
story.append(bullets([
    "<b>Pattern rule for object files</b> &mdash; <font face='Courier'>$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/*.h</font> "
    "tells <font face='Courier'>make</font> that any <font face='Courier'>obj/X.o</font> depends on "
    "<font face='Courier'>src/X.c</font> and on every header file. If a header changes, every "
    "<font face='Courier'>.o</font> file is considered out of date and is recompiled the next time "
    "<font face='Courier'>make</font> runs; if only one <font face='Courier'>.c</font> file changes, "
    "<font face='Courier'>make</font> recompiles only that one object file, because it compares each target's "
    "modification time against its prerequisites' modification times and skips a rule whose target is already "
    "newer than everything it depends on.",
    "<b>Static-library target</b> &mdash; <font face='Courier'>$(STATIC_LIB): $(LIB_OBJECTS)</font> means the "
    "archive is only rebuilt when at least one of the five library object files is newer than the archive "
    "itself, which in turn only happens when the corresponding source or a header changed.",
    "<b>Executable target</b> &mdash; <font face='Courier'>$(EXECUTABLE): $(MAIN_OBJECT) $(STATIC_LIB)</font> "
    "re-links the final binary whenever either <font face='Courier'>main.o</font> or the archive itself is "
    "newer than the existing executable.",
    "<b>Automation targets</b> &mdash; <font face='Courier'>run</font> depends on <font face='Courier'>all</font>, "
    "so typing <font face='Courier'>make run</font> first brings the executable up to date (rebuilding only "
    "what changed) and only then executes it against the two files in <font face='Courier'>data/</font>. The "
    "same target then invokes <font face='Courier'>make report</font>, which calls "
    "<font face='Courier'>python3 scripts/make_docx.py</font> to turn the freshly generated CSV into a "
    "Word document. This is the \"Python program invoked from the Makefile\" the assignment allows for turning "
    "the C program's output into a Word or Excel document: the C program owns the data (parsing and "
    "scheduling), and the Python script owns only the final document layout.",
    "<b>Housekeeping targets</b> &mdash; <font face='Courier'>clean</font> removes "
    "<font face='Courier'>obj/</font>, <font face='Courier'>lib/</font>, and <font face='Courier'>bin/</font> so the "
    "next build starts from source; <font face='Courier'>distclean</font> additionally removes the generated "
    "report files, giving a fully reproducible build from a clean checkout.",
]))
story.append(P(
    "In short, the Makefile's job is to never do more work than necessary (thanks to timestamp-based dependency "
    "checking) while still guaranteeing that a single command, <font face='Courier'>make run</font>, takes the "
    "project from source files all the way to a finished Word document."
))

# ---------------------------------------------------------------- 5
story.append(H1("5. The Office-Hours Placement Rule"))
story.append(P(
    "The two input files supply everything the program needs except one thing: a rule for turning a teaching "
    "schedule into an office-hours schedule. Rather than hard-coding one rule for all professors, each "
    "professor's <font face='Courier'>faculty_info.txt</font> row supplies three policy values &mdash; this is "
    "the \"office-hours table provided by the lecturer\" the assignment refers to:"
))
story.append(bullets([
    "<b>daytime_block_end_min</b> &mdash; how late a daytime office-hour block runs (e.g. 1:00 PM).",
    "<b>evening_block_len_min</b> &mdash; how long an office-hour block placed before an evening class should be "
    "(e.g. 120 minutes).",
    "<b>evening_cutoff_min</b> &mdash; the clock time at or after which a class is treated as an \"evening\" "
    "class rather than a daytime one (e.g. 4:00 PM).",
]))
story.append(P(
    "For each weekday and each professor, <font face='Courier'>scheduler_build</font> (in "
    "<font face='Courier'>scheduler.c</font>) separates that day's classes into daytime and evening meetings "
    "using <font face='Courier'>evening_cutoff_min</font>, then:"
))
story.append(bullets([
    "if the professor has a daytime meeting that day, one office-hour block is placed from the end of the "
    "<i>last-ending</i> daytime meeting until <font face='Courier'>daytime_block_end_min</font>;",
    "if the professor has an evening meeting that day, a second office-hour block of length "
    "<font face='Courier'>evening_block_len_min</font> is placed immediately before the "
    "<i>earliest-starting</i> evening meeting;",
    "a day with neither kind of meeting gets no office-hour block at all;",
    "every generated block is clamped to an 8:00 AM-6:00 PM campus-hours window so the rule can never place "
    "office hours outside reasonable working hours, even with an unusual class time.",
]))
story.append(P(
    "Applied to the sample professor in the assignment (Monday/Wednesday COSC 330 at 9:30-10:45 AM, "
    "Tuesday/Thursday COSC 369 at 9:30-10:45 AM, and a Wednesday evening COSC 631 at 5:00-7:30 PM, with "
    "<font face='Courier'>daytime_block_end_min</font> = 1:00 PM, <font face='Courier'>evening_block_len_min</font> "
    "= 120, <font face='Courier'>evening_cutoff_min</font> = 4:00 PM), this rule reproduces the exact office-hours "
    "table given in the assignment: 10:45 AM-1:00 PM on Monday, Tuesday, Wednesday and Thursday, plus a second "
    "3:00-5:00 PM block on Wednesday, and nothing on Friday."
))

# ---------------------------------------------------------------- 6
story.append(H1("6. Input File Location: Local Files, Not the Internet"))
story.append(P(
    "Both required input files are read from the local filesystem, passed to the program as command-line "
    "arguments (defaulted to <font face='Courier'>data/class_schedule.txt</font> and "
    "<font face='Courier'>data/faculty_info.txt</font> by the Makefile's <font face='Courier'>run</font> target). "
    "The program opens them with ordinary C standard-library file I/O "
    "(<font face='Courier'>fopen</font>/<font face='Courier'>fgets</font>/<font face='Courier'>fclose</font> in "
    "<font face='Courier'>course.c</font> and <font face='Courier'>faculty.c</font>); there is no networking code "
    "anywhere in the project."
))
story.append(P("This design was chosen deliberately, for several reasons:"))
story.append(bullets([
    "<b>Scope and reliability.</b> The assignment's own description treats the two files as artifacts a "
    "department already has &mdash; a syllabus/class-schedule table and a faculty roster &mdash; not as data that "
    "needs to be fetched from a remote service. Reading local files means the program has no dependency on "
    "network availability, no need for HTTP client code or a JSON/XML parser, and nothing that can fail because "
    "a remote server is down or requires authentication.",
    "<b>Reproducibility for grading.</b> A lab machine may not have outbound internet access at all, and even if "
    "it does, fetching \"the class schedule\" from the internet would require deciding which of countless "
    "possible remote sources counts as authoritative. Local files make the exact same run reproducible on any "
    "machine that has the project directory, which matters for a course project that will be compiled and run "
    "by someone other than its author.",
    "<b>Separation of concerns.</b> If the schedule data did need to come from a web service in some other "
    "deployment, the right place for that is a small preprocessing step (or a different data-acquisition module "
    "entirely) that writes the same pipe-delimited local file format the parser already expects &mdash; not a "
    "change to the parsing or scheduling modules themselves. Keeping <font face='Courier'>course_list_load</font> "
    "and <font face='Courier'>faculty_list_load</font> as pure \"read this local file\" functions keeps that door "
    "open without adding any actual networking to this project.",
]))

# ---------------------------------------------------------------- 7
story.append(H1("7. Error Handling"))
story.append(P(
    "Every module function that can fail returns a <font face='Courier'>ResultCode</font> "
    "(<font face='Courier'>RC_OK</font>, <font face='Courier'>RC_ERR_FILE_OPEN</font>, "
    "<font face='Courier'>RC_ERR_PARSE</font>, <font face='Courier'>RC_ERR_CAPACITY</font>, "
    "<font face='Courier'>RC_ERR_ARGS</font>) instead of a bare integer, so a call site can tell exactly what went "
    "wrong. A malformed line in either input file is reported to <font face='Courier'>stderr</font> with its line "
    "number and contents and is then skipped, so one bad line in a large file does not abort the entire run; a "
    "file that cannot be opened at all, or a fixed-size array that would overflow "
    "(<font face='Courier'>MAX_COURSES</font>, <font face='Courier'>MAX_FACULTY</font>, "
    "<font face='Courier'>MAX_DAILY_BLOCKS</font>), stops that step and is reported clearly rather than silently "
    "truncating data."
))

story.append(H1("8. Sample Output"))
story.append(P(
    "See <font face='Courier'>output/report.txt</font>, <font face='Courier'>output/report.csv</font>, and "
    "<font face='Courier'>output/report.docx</font> in the submission for the program's output on the sample "
    "input files in <font face='Courier'>data/</font>. The plain-text report for the first sample professor "
    "reproduces the exact CLASS SCHEDULE and OFFICE HOURS tables given as the sample output in the assignment."
))

doc = SimpleDocTemplate(
    "report/COSC369_Project1_Report.pdf",
    pagesize=letter,
    leftMargin=0.9 * inch, rightMargin=0.9 * inch,
    topMargin=0.8 * inch, bottomMargin=0.8 * inch,
    title="COSC 369 Project 1 Report",
)
doc.build(story)
print("PDF built.")
