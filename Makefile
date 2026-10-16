#
# Makefile for the COSC 369 professor-schedule generator.
#
# Build layout:
#   include/         header files shared by the library and main.c
#   src/              all .c sources (library modules + main.c)
#   obj/              intermediate .o files (created by this Makefile)
#   lib/libschedgen.a the static library (course, faculty, scheduler,
#                     output, util modules -- everything except main.c)
#   bin/schedgen      the final linked executable
#   data/             the two required input files
#   output/           generated report.txt, report.csv
#
# Key targets:
#   make            builds the static library and the executable
#   make run        builds (if needed) and runs schedgen on data/
#   make clean      removes obj/, lib/, bin/
#   make import     Python: Excel schedule -> the two input files
#   make facts      Python: input files -> prolog/facts.pl
#   make query      Prolog: answer sample questions via pyswip
#   make verify     check Prolog's office hours match the C output
#   make distclean   clean, plus generated files under output/ and prolog/
#

CC       := gcc
CFLAGS   := -std=c11 -Wall -Wextra -Iinclude -g
AR       := ar
ARFLAGS  := rcs
PYTHON   := python3

SRC_DIR  := src
OBJ_DIR  := obj
LIB_DIR  := lib
BIN_DIR  := bin
DATA_DIR := data
OUT_DIR  := output
SCRIPTS_DIR := scripts
PROLOG_DIR  := prolog

# Every module EXCEPT main.c goes into the static library. This is the
# "organize into modules + build a static library" requirement: main.c
# is the only translation unit that is not archived, since it is the
# program's entry point rather than reusable functionality.
LIB_SOURCES := $(SRC_DIR)/util.c $(SRC_DIR)/course.c $(SRC_DIR)/faculty.c \
               $(SRC_DIR)/scheduler.c $(SRC_DIR)/output.c
LIB_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_SOURCES))

MAIN_SOURCE := $(SRC_DIR)/main.c
MAIN_OBJECT := $(OBJ_DIR)/main.o

STATIC_LIB  := $(LIB_DIR)/libschedgen.a
EXECUTABLE  := $(BIN_DIR)/schedgen

CLASS_SCHEDULE_FILE := $(DATA_DIR)/class_schedule.txt
FACULTY_INFO_FILE   := $(DATA_DIR)/faculty_info.txt
REPORT_TXT  := $(OUT_DIR)/report.txt
REPORT_CSV  := $(OUT_DIR)/report.csv
FACTS_PL    := $(PROLOG_DIR)/facts.pl

.PHONY: all run import facts query verify clean distclean directories

all: directories $(EXECUTABLE)

directories:
	@mkdir -p $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR) $(OUT_DIR)

# --- Static library -----------------------------------------------------
# Each module is compiled to a .o, then archived with ar into one .a
# file. Linking against $(STATIC_LIB) later pulls in only the object
# files the linker actually needs to resolve symbols used by main.c.
$(STATIC_LIB): $(LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/*.h | directories
	$(CC) $(CFLAGS) -c $< -o $@

# --- Executable ----------------------------------------------------------
# main.c is compiled on its own and linked against the static library,
# demonstrating library linkage rather than compiling every module
# directly into the executable.
$(EXECUTABLE): $(MAIN_OBJECT) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(MAIN_OBJECT) -L$(LIB_DIR) -lschedgen -o $@

# --- Run -------------------------------------------------------------
# Runs the executable against the two required input files, producing
# the text and CSV reports under output/.
run: all
	./$(EXECUTABLE) $(CLASS_SCHEDULE_FILE) $(FACULTY_INFO_FILE) \
		$(REPORT_TXT) $(REPORT_CSV)

# --- Import from Excel -------------------------------------------------
# Python converts the department spreadsheet into the two text files the
# C program reads. Run by hand (needs: python3 -m pip install openpyxl);
# the generated files are already included, so `make run` does not need it.
SCHEDULE_XLSX := $(DATA_DIR)/CTEC_Fall_2026_Schedule.xlsx
import:
	$(PYTHON) $(SCRIPTS_DIR)/import_xlsx.py $(SCHEDULE_XLSX) $(DATA_DIR)

# --- Prolog layer ----------------------------------------------------
# Python reads the same two input files and writes Prolog facts; the
# rules in prolog/rules.pl answer questions about them.
# Re-generated whenever either input file or the script changes.
$(FACTS_PL): $(CLASS_SCHEDULE_FILE) $(FACULTY_INFO_FILE) $(SCRIPTS_DIR)/gen_facts.py
	$(PYTHON) $(SCRIPTS_DIR)/gen_facts.py $(CLASS_SCHEDULE_FILE) $(FACULTY_INFO_FILE) $@

facts: $(FACTS_PL)

query: $(FACTS_PL)
	$(PYTHON) $(SCRIPTS_DIR)/query.py

verify: run $(FACTS_PL)
	$(PYTHON) $(SCRIPTS_DIR)/verify.py $(REPORT_CSV)

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

distclean: clean
	rm -f $(REPORT_TXT) $(REPORT_CSV) $(FACTS_PL)
