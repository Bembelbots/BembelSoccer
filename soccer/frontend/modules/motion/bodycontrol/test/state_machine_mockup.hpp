#include <core/logger.h>
#include <bodycontrol/test/unittest.hpp>

#include <bodycontrol/utils/state_machine.h>

class BCSM {
    int state_machine(){
        //SHOT DBG << _state_machine_last_state << " -> " << _state_machine_state << EL;
        initial_state(initial,0){
            state_initialization{
                TEST << "initial init";
                counter = 2;
            }
            TEST << "initial_state initial";
            if (--counter < 0)
                goto state1;
        }
        state(state1,1){
            state_initialization {
                TEST << "state 1 init";
                counter = 3;
            }
            TEST << "state 1";
            if (counter-- == 0) goto istate1;
        }
        immediate_state(istate1,2){
            TEST << "immediate state 1";
            goto istate2;
        }
        immediate_state(istate2,3){
            TEST << "immediate state 2";
            goto state2;
        }
        state(state2,4){
            state_initialization {
                TEST << "state 2 init";
                counter = 2;
            }
            TEST << "state 2";
            if (counter-- == 0 ) goto initial;
        }
    }

    private:
        int counter;


    public:
        BCSM(){
            
        }
        ~BCSM(){}
        void step(){
            TEST << "returned: " << run_state_machine();
        }




    
};

#define UNDEF_STATE_MACHINE
#include <bodycontrol/utils/state_machine.h>
