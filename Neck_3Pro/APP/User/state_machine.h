#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#include <stddef.h>
#include <stdbool.h>

struct event
{
    int type;
    void *data;
};

struct state;

struct transition
{
    int event_type;
    void *condition;
    bool (*guard)(void *condition, struct event *event);
    void (*action)(void *state_current_data, struct event *event,
         void *state_new_data);
    struct state *state_next;
};

struct state
{
    struct state *state_parent;
    struct state *state_entry;  
    struct transition *transitions;
    size_t transition_nums;
    void *data;
    void (*action_entry)(void *state_data, struct event *event);
    void (*action_exit)(void *state_data, struct event *event);
};

struct state_machine
{
    struct state *state_current;
    struct state *state_previous;
    struct state *state_error;
};

int statem_init(struct state_machine *state_machine, struct state *state_init, struct state *state_error);

enum statem_handle_event_return_vals
{
    STATEM_ERR_ARG = -2,
    STATEM_ERR_STATE_RECHED,
    STATEM_STATE_CHANGED,
    STATEM_STATE_LOOPSELF,
    STATEM_STATE_NOCHANGE,
    STATEM_FINAL_STATE_RECHED,
};

int statem_handle_event(struct state_machine *state_machine, struct event *event);
struct state *statem_state_current(struct state_machine *state_machine);
struct state *statem_state_previous(struct state_machine *state_machine);
int statem_stopped(struct state_machine *state_machine);

#endif 


