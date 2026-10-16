% rules.pl
%
% Hand-written Prolog rules. They work on the facts that
% scripts/gen_facts.py generates from the two input files.
%
% Facts expected (see facts.pl):
%   faculty(Id, Name, Dept).
%   policy(Id, DaytimeEndMin, EveningLenMin, EveningCutoffMin).
%   course(Code, Title, LecturerId).
%   meets(Code, Day, StartMin, EndMin, Room).

weekday(monday). weekday(tuesday). weekday(wednesday).
weekday(thursday). weekday(friday).

campus_open(480).     % 8:00 AM
campus_close(1080).   % 6:00 PM

% ---- simple lookups --------------------------------------------------

teaches(Prof, Code)        :- course(Code, _, Prof).
teaches_name(Name, Code)   :- faculty(Prof, Name, _), teaches(Prof, Code).
class_on(Prof, Day, Code, Start, End, Room) :-
    teaches(Prof, Code),
    meets(Code, Day, Start, End, Room).
teaches_on(Prof, Day)      :- class_on(Prof, Day, _, _, _, _), !.

% ---- office-hours rules (same rule the C program uses) ---------------

% A class is "evening" if it starts at or after the professor's cutoff.
evening_class(Prof, Day, Code, Start) :-
    policy(Prof, _, _, Cutoff),
    class_on(Prof, Day, Code, Start, _, _),
    Start >= Cutoff.

daytime_class(Prof, Day, Code, End) :-
    policy(Prof, _, _, Cutoff),
    class_on(Prof, Day, Code, Start, End, _),
    Start < Cutoff.

clamp(X, Lo, Hi, R) :- R is max(Lo, min(Hi, X)).

% Block after the day's last-ending daytime class, up to the
% professor's daytime end time.
office_hours(Prof, Day, Start, End, Code) :-
    policy(Prof, DayEnd, _, _),
    weekday(Day),
    aggregate_all(max(E, C), daytime_class(Prof, Day, C, E), max(LastEnd, Code)),
    LastEnd < DayEnd,
    campus_open(Open), campus_close(Close),
    clamp(LastEnd, Open, Close, Start),
    clamp(DayEnd, Open, Close, End),
    End > Start.

% Block of the professor's chosen length right before the day's
% earliest evening class.
office_hours(Prof, Day, Start, End, Code) :-
    policy(Prof, _, Len, _),
    weekday(Day),
    aggregate_all(min(S, C), evening_class(Prof, Day, C, S), min(EveStart, Code)),
    campus_open(Open), campus_close(Close),
    Raw is EveStart - Len,
    clamp(Raw, Open, Close, Start),
    clamp(EveStart, Open, Close, End),
    End > Start.

% ---- readable questions ----------------------------------------------

office_hours_on(Day, Name, Start, End, Code) :-
    faculty(Prof, Name, _),
    office_hours(Prof, Day, Start, End, Code).

busy_at(Prof, Day, Time) :-
    class_on(Prof, Day, _, S, E, _),
    Time >= S, Time < E.

in_office_hours_at(Prof, Day, Time) :-
    office_hours(Prof, Day, S, E, _),
    Time >= S, Time < E.
