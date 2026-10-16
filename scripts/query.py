#!/usr/bin/env python3
"""
query.py

Loads the generated facts and the hand-written rules into Prolog (via
pyswip) and answers questions about the schedule.

Usage:
    python3 scripts/query.py                  # run the sample questions
    python3 scripts/query.py "teaches(P, 'ctec 402.170')"   # run any Prolog query
"""
import sys
from pyswip import Prolog

FACTS = "prolog/facts.pl"
RULES = "prolog/rules.pl"


def clock(minutes):
    h, m = divmod(int(minutes), 60)
    return f"{(h % 12) or 12}:{m:02d} {'AM' if h < 12 else 'PM'}"


def load():
    prolog = Prolog()
    prolog.consult(FACTS)
    prolog.consult(RULES)
    return prolog


def sample_questions(prolog):
    print("Q: Who teaches CTEC 402.170?")
    for r in prolog.query("teaches_name(Name, 'ctec 402.170')"):
        print(f"   {r['Name']}")

    print("\nQ: What does Pierrot Chery teach?")
    for r in prolog.query("teaches_name('Pierrot Chery', Code)"):
        print(f"   {str(r["Code"]).upper()}")

    print("\nQ: Which office hours are on Wednesday?")
    for r in prolog.query("office_hours_on(wednesday, Name, S, E, Code)"):
        print(f"   {r['Name']}: {clock(r['S'])} - {clock(r['E'])} ({str(r["Code"]).upper()})")

    print("\nQ: Who has office hours on Friday?")
    results = list(prolog.query("office_hours_on(friday, Name, _, _, _)"))
    for r in results:
        print(f"   {r['Name']}")
    if not results:
        print("   Nobody")

    print("\nQ: Which professors have no class on Friday?")
    for r in prolog.query("faculty(P, Name, _), \\+ teaches_on(P, friday)"):
        print(f"   {r['Name']}")

    print("\nQ: Is Pierrot Chery in office hours on Tuesday at 3:00 PM?")
    ok = bool(list(prolog.query("in_office_hours_at(chery, tuesday, 900)")))
    print(f"   {'Yes' if ok else 'No'}")


def main():
    prolog = load()
    if len(sys.argv) > 1:
        for r in prolog.query(sys.argv[1]):
            print(r)
    else:
        sample_questions(prolog)


if __name__ == "__main__":
    main()
