/**
 *  State machine for bodycontrol modules.
 *  This header defines macros that collide with Cabsl and
 *  can easily cause other problems as they are pretty usual identifiers.
 *  This can easily lead to nasty errors. For this reason, if you include this
 *  header in a submodule, ALWAYS INCLUDE THIS HEADER LAST and 
 *  undef all macros by pasting the following code
 *  at the end of your submodule header:
 *      #define STATE_MACHINE_UNDEF
 *      #include <bodycontrol/utils/state_machine.h>
 */
#pragma once

#define BC_STATE_MACHINE(...) \
    _state_machine_return_value; \
    private: \
    int _state_machine_state = 0; \
    int _state_machine_last_state = -1; \
    int _state_machine_bak_state = -1; \
    void reset_state_machine() { \
        _state_machine_state = 0; \
        _state_machine_last_state = -1; \
    } \
    void _state_machine_internals(__VA_ARGS__)

#define BC_RUN_STATE_MACHINE(...) \
    ( _state_machine_bak_state = _state_machine_state, \
      _state_machine_internals(__VA_ARGS__), \
      _state_machine_last_state = _state_machine_bak_state, \
      _state_machine_return_value \
    )

#define BC_INITIAL_STATE(name, returnvalue) \
    if (false) { \
    initial_state: \
    name: \
        _state_machine_state = 0; \
        return; \
    } \
    if (_state_machine_state <= 0) { \
        _state_machine_return_value = returnvalue; \
    } \
    if (_state_machine_state <= 0)

#define _BC_STATE(name, line, is_immediate_state, returnvalue) \
    if (false) { \
        goto initial_state; \
    name: \
        _state_machine_state = line; \
        if (not is_immediate_state) return; \
    } \
    if (_state_machine_state == line) { \
        _state_machine_return_value = returnvalue; \
    } \
    if (_state_machine_state == line)


#define BC_STATE(name,returnvalue) _BC_STATE(name, __LINE__, false, returnvalue)
#define BC_IMMEDIATE_STATE(name,returnvalue) _BC_STATE(name, __LINE__, true, returnvalue)

#define BC_STATE_INITIALIZATION if (_state_machine_last_state != _state_machine_state)