#include "state_machine.h"

static void go_to_state_error(struct state_machine *state_machine, struct event *const event);
static struct transition *get_transition(struct state_machine *state_machine, struct state *state, struct event *const event);

int statem_init(struct state_machine *fsm, struct state *state_init, struct state *state_error)
{
    if ( !fsm )
    {
        return -1;
    }

    fsm->state_current = state_init;
    fsm->state_previous = NULL;
    fsm->state_error = state_error;
      
    return 0;      
}

int statem_handle_event(struct state_machine *fsm, struct event *event)
{
    if ( !fsm || !event )
    {
        return STATEM_ERR_ARG;
    }

    if ( !fsm->state_current )
    {
        go_to_state_error( fsm, event );
        return STATEM_ERR_STATE_RECHED;
    }

    if ( (!fsm->state_current->transition_nums) && (!fsm->state_current->state_parent))
    {
        return STATEM_STATE_NOCHANGE;
    }

    struct state *state_next = fsm->state_current;
    
    do {
        struct transition *transition = get_transition( fsm, state_next, event );

        if ( !transition )
        {
            state_next = state_next->state_parent;
            continue;
        }

        if ( !transition->state_next )
        {
            go_to_state_error( fsm, event );
            return STATEM_ERR_STATE_RECHED;
        }

        state_next = transition->state_next;

        while ( state_next->state_entry )
        {
            state_next = state_next->state_entry;
        }

        if ( state_next != fsm->state_current && fsm->state_current->action_exit )
        {
            fsm->state_current->action_exit( fsm->state_current->data, event );
        }

        if ( transition->action )
        {
            transition->action( fsm->state_current->data, event, state_next->data );
        }

        fsm->state_previous = fsm->state_current;

        if ( state_next != fsm->state_current && state_next->action_entry )
        {
            state_next->action_entry( state_next->data, event );
        }

        fsm->state_current = state_next;

        if ( fsm->state_current == fsm->state_previous )
        {
            return STATEM_STATE_LOOPSELF;
        }

        if ( fsm->state_current == fsm->state_error )
        {
            return STATEM_ERR_STATE_RECHED;
        }

        if ((!fsm->state_current->transition_nums) && (!fsm->state_current->state_parent))
        {
            return STATEM_FINAL_STATE_RECHED;
        }

        return STATEM_STATE_CHANGED;
    } while ( state_next );

    return STATEM_STATE_NOCHANGE;
}

struct state *statem_state_current(struct state_machine *fsm)
{
    if ( !fsm )
    {
        return NULL;
    }

    return fsm->state_current;
}

struct state *statem_state_previous(struct state_machine *fsm)
{
    if ( !fsm )
    {
        return NULL;
    }

    return fsm->state_previous;
}

static void go_to_state_error(struct state_machine *fsm,
      struct event *const event)
{
    fsm->state_previous = fsm->state_current;
    fsm->state_current = fsm->state_error;

    if ( fsm->state_current && fsm->state_current->action_entry )
    {
        fsm->state_current->action_entry(fsm->state_current->data, event);
    }
}

static struct transition *get_transition(struct state_machine *fsm, struct state *state, struct event *const event)
{
    size_t i;

    if( !state )
    {
        return NULL;
    }

    for ( i = 0; i < state->transition_nums; ++i )
    {
        struct transition *t = &state->transitions[ i ];

        if ( t->event_type == event->type )
        {
            if ( !t->guard )
            {
                return t;
            }
            else if ( t->guard( t->condition, event ) )
            {
                return t;
            }
        }
    }

    return NULL;
}

int statem_stopped(struct state_machine *state_machine)
{
    if ( !state_machine )
    {
        return -1;
    }

    return (state_machine->state_current->transition_nums == 0);
}
