/*Nithya Gopalakrishnan
UID: 119327937
ngopala1*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calendar.h"
#include "event.h"

int init_calendar(const char *name, int days,
                  int (*comp_func) (const void *ptr1, const void *ptr2),
                  void (*free_info_func) (void *ptr), Calendar **calendar) {

    if (!name || days < 1 || !calendar) {
        return FAILURE;
    }

    *calendar = malloc(sizeof(Calendar));

    if (!*calendar) {
        return FAILURE;
    }

    (*calendar)->name = malloc(strlen(name) + 1);

    if (!(*calendar)->name) {
        free(*calendar);
        return FAILURE;
    }

    strcpy((*calendar)->name, name);

    (*calendar)->events = calloc(days, sizeof(Event *));

    if (!(*calendar)->events) {
        free((*calendar)->name);
        free(*calendar);
        return FAILURE;
    }

    (*calendar)->days = days;
    (*calendar)->total_events = 0;
    (*calendar)->comp_func = comp_func;
    (*calendar)->free_info_func = free_info_func;

    return SUCCESS;
}

int print_calendar(Calendar *calendar, FILE *output_stream, int print_all) {
    Event *current;
    int i;

    if (calendar == NULL || output_stream == NULL) {
        return FAILURE;
    }

    if (print_all) {
        fprintf(output_stream, "Calendar's Name: \"%s\"\n", calendar->name);
        fprintf(output_stream, "Days: %d\n", calendar->days);
        fprintf(output_stream, "Total Events: %d\n", calendar->total_events);
    }

    fprintf(output_stream, "\n**** Events ****\n");
    for (i = 0; i < calendar->days; i++) {
        if(calendar->total_events != 0){
            fprintf(output_stream, "Day %d\n", i + 1);
        }
        
        current = calendar->events[i];
        while (current != NULL) {
            fprintf(output_stream, "Event's Name: \"%s\", Start_time: %d, Duration: %d\n",
                    current->name, current->start_time, current->duration_minutes);
            current = current->next;
        }
    }

    return SUCCESS;
}

/*need to check each name to see if event alr exists*/
int add_event(Calendar *calendar, const char *name, int start_time, int duration_minutes, void *info, int day) {
    Event **event_list, *new_event, *current, *prev, *found_event;

    if (!calendar || !name || start_time < 0 || start_time > 2400 || duration_minutes <= 0 || day < 1 || day > calendar->days || find_event(calendar, name, &found_event) == SUCCESS) {
        return FAILURE;
    }

    new_event = malloc(sizeof(Event));

    if (!new_event) {
        return FAILURE;
    }

    new_event->name = malloc(strlen(name) + 1);

    if (!new_event->name) {
        free(new_event);
        return FAILURE;
    }
 
    strcpy(new_event->name, name);

    new_event->start_time = start_time;
    new_event->duration_minutes = duration_minutes;
    new_event->info = info;
    new_event->next = NULL;

    event_list = &(calendar->events[day - 1]);
    current = *event_list;
    prev = NULL;

    while (current && calendar->comp_func(new_event, current) > 0) {
        prev = current;
        current = current->next;
    }

    if (current && strcmp(current->name, name) == 0) {
        free(new_event->name);
        free(new_event);
        return FAILURE;
    }

    new_event->next = current;
    if (prev) {
        prev->next = new_event;
    } else {
        *event_list = new_event;
    }

    calendar->total_events++;
    return SUCCESS;
}

int find_event(Calendar *calendar, const char *name, Event **event){
    int i;
    Event *current;

    if (!calendar || !name) {
        return FAILURE;
    }

    for (i = 0; i < calendar->days; ++i) {
        current = calendar->events[i];
        while (current) {
            if (strcmp(current->name, name) == 0) {
                if (event) {
                    *event = current;
                }
                return SUCCESS;
            }
            current = current->next;
        }
    }

    return FAILURE;
}

int find_event_in_day(Calendar *calendar, const char *name, int day,
                      Event **event){
    Event *current;

    if (!calendar || !name || day < 1 || day > calendar->days) {
        return FAILURE;
    }

    current = calendar->events[day - 1];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (event) {
                *event = current;
            }
            return SUCCESS;
        }
        current = current->next;
    }

    return FAILURE;
}

int remove_event(Calendar *calendar, const char *name){
    Event **current, *prev, *to_delete;
    int i;

    if (!calendar || !name) {
        return FAILURE;
    }

    for (i = 0; i < calendar->days; ++i) {
        current = &(calendar->events[i]);
        prev = NULL;

        while (*current) {
            if (strcmp((*current)->name, name) == 0) {
                to_delete = *current;
                if (prev) {
                    prev->next = (*current)->next;
                } else {
                    calendar->events[i] = (*current)->next;
                }

                if (calendar->free_info_func && to_delete->info) {
                    calendar->free_info_func(to_delete->info);
                }

                free(to_delete->name);
                free(to_delete);
                calendar->total_events--;

                return SUCCESS;
            }
            prev = *current;
            current = &((*current)->next);
        }
    }

    return FAILURE;
}

void *get_event_info(Calendar *calendar, const char *name){
    Event *event = NULL;

    if (find_event(calendar, name, &event) == SUCCESS) {
        return event->info;
    }

    return NULL;
}

int clear_calendar(Calendar *calendar) {
    int i;

    if (!calendar) {
        return FAILURE;
    }

    for (i = 0; i < calendar->days; ++i) {
        clear_day(calendar, i + 1);
    }

    return SUCCESS;
}

int clear_day(Calendar *calendar, int day) {
    Event *current, *to_delete;

    if (!calendar || day < 1 || day > calendar->days) {
        return FAILURE;
    }

    current = calendar->events[day - 1];
    while (current) {
        to_delete = current;
        current = current->next;

        if (calendar->free_info_func && to_delete->info) {
            calendar->free_info_func(to_delete->info);
        }

        free(to_delete->name);
        free(to_delete);
    }

    calendar->events[day - 1] = NULL;
    calendar->total_events = 0;

    return SUCCESS;
}

int destroy_calendar(Calendar *calendar) {
    if (!calendar) {
        return FAILURE;
    }

    clear_calendar(calendar);

    free(calendar->events);
    free(calendar->name);
    free(calendar);

    return SUCCESS;
}