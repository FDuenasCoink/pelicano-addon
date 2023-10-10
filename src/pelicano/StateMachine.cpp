/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de la maquina de estados
 * @version 1.1
 * @date 2023-05-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "StateMachine.hpp"

namespace PelicanoStateMachine{

    using namespace ValidatorPelicano;

    PelicanoClass *PelicanoObject;

    PelicanoSMClass::PelicanoSMClass(ValidatorPelicano::PelicanoClass *_PelicanoClass_p){
        PelicanoObject = _PelicanoClass_p;
        SM.CurrState = PelicanoSMClass::ST_IDLE;
    };

    struct StateFunctionRow_t{
        const char * name;
        int (PelicanoClass::*func)(void);
    };

    static StateFunctionRow_t StateFunctionValidatorPelicano[] = {
            // NAME         // FUNC
        { "ST_IDLE",       &PelicanoClass::StIdle },      
        { "ST_CONNECT",    &PelicanoClass::StConnect },      
        { "ST_CHECK",      &PelicanoClass::StCheck },     
        { "ST_ENABLE",     &PelicanoClass::StEnable },
        { "ST_POLLING",    &PelicanoClass::StPolling },
        { "ST_CLEANBOWL",  &PelicanoClass::StCleanBowl }, 
        { "ST_RESET",      &PelicanoClass::StReset }, 
        { "ST_ERROR",      &PelicanoClass::StError }, 
    };

    struct StateTransitionRow_t{
        PelicanoSMClass::State_t CurrState;
        PelicanoSMClass::Event_t Event;
        PelicanoSMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { PelicanoSMClass::ST_IDLE,          PelicanoSMClass::EV_ANY,             PelicanoSMClass::ST_CONNECT},

        { PelicanoSMClass::ST_CONNECT,       PelicanoSMClass::EV_SUCCESS_CONN,    PelicanoSMClass::ST_CHECK},
        { PelicanoSMClass::ST_CONNECT,       PelicanoSMClass::EV_ERROR,           PelicanoSMClass::ST_ERROR},

        { PelicanoSMClass::ST_CHECK,         PelicanoSMClass::EV_CALL_POLLING,    PelicanoSMClass::ST_ENABLE},
        { PelicanoSMClass::ST_CHECK,         PelicanoSMClass::EV_CHECK,           PelicanoSMClass::ST_CHECK},
        { PelicanoSMClass::ST_CHECK,         PelicanoSMClass::EV_TRASH,           PelicanoSMClass::ST_CLEANBOWL},
        { PelicanoSMClass::ST_CHECK,         PelicanoSMClass::EV_ERROR,           PelicanoSMClass::ST_ERROR},

        { PelicanoSMClass::ST_ENABLE,        PelicanoSMClass::EV_READY,           PelicanoSMClass::ST_POLLING},
        { PelicanoSMClass::ST_ENABLE,        PelicanoSMClass::EV_ERROR,           PelicanoSMClass::ST_ERROR},
        
        { PelicanoSMClass::ST_POLLING,       PelicanoSMClass::EV_FINISH_POLL,     PelicanoSMClass::ST_CLEANBOWL},
        { PelicanoSMClass::ST_POLLING,       PelicanoSMClass::EV_POLL,            PelicanoSMClass::ST_POLLING},
        { PelicanoSMClass::ST_POLLING,       PelicanoSMClass::EV_ERROR,           PelicanoSMClass::ST_ERROR},

        { PelicanoSMClass::ST_CLEANBOWL,     PelicanoSMClass::EV_EMPTY,           PelicanoSMClass::ST_RESET},
        { PelicanoSMClass::ST_CLEANBOWL,     PelicanoSMClass::EV_ERROR,           PelicanoSMClass::ST_ERROR},
        { PelicanoSMClass::ST_CLEANBOWL,     PelicanoSMClass::EV_ANY,             PelicanoSMClass::ST_CHECK},
        { PelicanoSMClass::ST_CLEANBOWL,     PelicanoSMClass::EV_FINISH_POLL,     PelicanoSMClass::ST_CLEANBOWL},

        { PelicanoSMClass::ST_RESET,         PelicanoSMClass::EV_LOOP,            PelicanoSMClass::ST_CHECK},
        { PelicanoSMClass::ST_RESET,         PelicanoSMClass::EV_ANY,             PelicanoSMClass::ST_RESET},
        { PelicanoSMClass::ST_RESET,         PelicanoSMClass::EV_ERROR,           PelicanoSMClass::ST_ERROR},
        { PelicanoSMClass::ST_ERROR,         PelicanoSMClass::EV_ANY,             PelicanoSMClass::ST_IDLE},
    };

    void PelicanoSMClass::InitStateMachine() {
        SM.CurrState = PelicanoSMClass::ST_IDLE; 
        (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
    }

    int PelicanoSMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = PelicanoSMClass::ST_CHECK; 
        Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int PelicanoSMClass::RunReset() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = PelicanoSMClass::ST_RESET; 
        Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int PelicanoSMClass::RunClean() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = PelicanoSMClass::ST_CLEANBOWL; 
        Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }
    
    
    int PelicanoSMClass::StateMachineRun(Event_t Event) {
        int Response = -1;
        for(long unsigned int i = 0; i < sizeof(StateTransition)/sizeof(StateTransition[0]); i++) {
            if(StateTransition[i].CurrState == SM.CurrState) {
                if(StateTransition[i].Event == Event) {
                    SM.CurrState =  StateTransition[i].NextState;
                    Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
                    return Response;
                    break;
                }
            }
        }
        return Response;
    }
    
    const char * PelicanoSMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionValidatorPelicano[State].name;
    }
};