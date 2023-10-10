/**
 * @file PelicanoControl.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header del archivo principal que expone las funciones del validador Pelicano
 * @version 1.1
 * @date 2023-05-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef PELICANOCONTROL
#define PELICANOCONTROL

#include <stdio.h>
#include <string>
#include <iostream>
#include "StateMachine.hpp"
#include "ValidatorPelicano.hpp"

namespace PelicanoControl{

    using namespace PelicanoStateMachine;
    using namespace ValidatorPelicano;

    struct Response_t{
        int StatusCode;
        std::string Message;
    };

    struct CoinError_t{
        int StatusCode;
        int Event;
        int Coin;
        std::string Message;
        int Remaining;
    };

    struct CoinLost_t{
        int CoinCinc;
        int CoinCien;
        int CoinDosc;
        int CoinQuin;
        int CoinMil;
    };

    struct TestStatus_t{
        std::string Version;
        int Device;
        int ErrorType; 
        int ErrorCode;
        std::string Message; 
        std::string AditionalInfo;
        int Priority; 
    };

    class GlobalVariables {
        public:
            PelicanoClass PelicanoObject;
            PelicanoClass* PelicanoPointer;
            PelicanoSMClass SMObject;
            GlobalVariables() : PelicanoObject(), PelicanoPointer(&PelicanoObject), SMObject(PelicanoPointer) {}
    };

    class PelicanoControlClass{
        public:
            
            //READ ONLY
            int PortO;
            unsigned long InsertedCoins;

            //WRITE ONLY
            int WarnToCritical;
            int MaxCritical;
            std::string Path;
            int LogLvl;
            int MaximumPorts;

            GlobalVariables Globals;
            
            PelicanoControlClass();
            ~PelicanoControlClass();
            void InitLog();
            Response_t Connect();
            Response_t CheckDevice();
            Response_t StartReader();
            CoinError_t GetCoin();
            CoinLost_t GetLostCoins();
            Response_t ModifyChannels(int InhibitMask1,int InhibitMask2);
            Response_t StopReader();
            Response_t ResetDevice();
            Response_t CleanDevice();
            Response_t GetInsertedCoins();
            TestStatus_t TestStatus();
            Response_t CheckCodes(int Check);
    };
}

#endif /* PELICANOCONTROL */