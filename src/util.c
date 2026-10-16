/*
 * util.c
 *
 * Small, dependency-free helper functions shared by every module:
 * time-string conversion and whitespace trimming. Kept in their own
 * translation unit so they can be unit-tested and reused without
 * pulling in parsing or scheduling code.
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

int time_to_minutes(const char *hhmm)
{
    int h, m;
    if (sscanf(hhmm, "%d:%d", &h, &m) != 2)
        return -1;
    if (h < 0 || h > 23 || m < 0 || m > 59)
        return -1;
    return h * 60 + m;
}

void minutes_to_ampm(int minutes, char *buf, size_t buf_size)
{
    int h24 = (minutes / 60) % 24;
    int m   = minutes % 60;
    const char *suffix = (h24 < 12) ? "AM" : "PM";
    int h12 = h24 % 12;
    if (h12 == 0)
        h12 = 12;
    snprintf(buf, buf_size, "%d:%02d %s", h12, m, suffix);
}

char *trim(char *s)
{
    if (!s)
        return s;

    /* Skip leading whitespace. */
    while (*s && isspace((unsigned char)*s))
        s++;

    if (*s == '\0')
        return s;

    /* Trim trailing whitespace in place. */
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}
