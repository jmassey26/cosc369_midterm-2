#!/usr/bin/env python3
"""
verify.py

Cross-checks the two implementations of the office-hours rule: the C
program's output (output/report.csv) and the Prolog rules. Exits with
status 1 if they disagree.

Usage: python3 scripts/verify.py output/report.csv
"""
import csv
import sys
from pyswip import Prolog

DAYS = ["monday", "tuesday", "wednesday", "thursday", "friday"]


def clock(minutes):
    h, m = divmod(int(minutes), 60)
    return f"{(h % 12) or 12}:{m:02d} {'AM' if h < 12 else 'PM'}"


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <report.csv>", file=sys.stderr)
        sys.exit(1)

    from_c = set()
    with open(sys.argv[1], newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            if row["kind"] == "office_hours":
                from_c.add((row["faculty"], row["day"].lower(), row["start"], row["end"]))

    prolog = Prolog()
    prolog.consult("prolog/facts.pl")
    prolog.consult("prolog/rules.pl")
    from_prolog = set()
    for r in prolog.query("office_hours_on(Day, Name, S, E, _)"):
        from_prolog.add((str(r["Name"]), str(r["Day"]), clock(r["S"]), clock(r["E"])))

    print(f"C program:  {len(from_c)} office-hour blocks")
    print(f"Prolog:     {len(from_prolog)} office-hour blocks")
    if from_c == from_prolog:
        print("MATCH: both produce the same office hours.")
        return
    print("MISMATCH")
    for x in sorted(from_c - from_prolog):
        print("  only in C:     ", x)
    for x in sorted(from_prolog - from_c):
        print("  only in Prolog:", x)
    sys.exit(1)


if __name__ == "__main__":
    main()
