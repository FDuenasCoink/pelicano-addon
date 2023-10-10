/**
 * @file PelicanoControl.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Archivo principal que expone las funciones del validador Pelicano
 * @version 1.1
 * @date 2023-05-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "PelicanoControl.hpp"

namespace PelicanoControl {

    using namespace PelicanoStateMachine;
    using namespace ValidatorPelicano;

    // --------------- EXTERNAL VARIABLES --------------------//

    // READ ONLY
    int PortO;
    unsigned long InsertedCoins;

    // WRITE ONLY
    int WarnToCritical;
    int MaxCritical;
    std::string Path;
    int LogLvl;
    int MaximumPorts;
    
    // --------------- INTERNAL VARIABLES --------------------//
    
    std::string VERSION = "1.1";
    std::string DEFAULTERROR = "DeafaultError";
    int Remaining;
    bool FlagCritical;
    bool FlagCritical2;
    int WarnCounter;
    int CriticalCounter;
    int CoinEventPrev;
    
    Response_t Response;
    
    PelicanoControlClass::PelicanoControlClass(){
        
        PortO = 0;
        CoinEventPrev = 0;
        WarnCounter = 0;
        CriticalCounter = 0;
        Remaining = 0;
        FlagCritical = false;
        FlagCritical2 = false;
        WarnToCritical = 10;
        MaxCritical = 4;
        Path = "logs/Pelicano.log";
        LogLvl = 1;
        InsertedCoins = 0;                
        MaximumPorts = 10;
        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;
    }

    PelicanoControlClass::~PelicanoControlClass(){}

    void PelicanoControlClass::InitLog(){
        Globals.PelicanoObject.LoggerLevel = LogLvl;
        Globals.PelicanoObject.InitLogger(Path);
        Globals.PelicanoObject.MaxPorts = MaximumPorts;
    }

    Response_t PelicanoControlClass::Connect() {
        
        int Connection = -1;
        int Check = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        //std::cout<<"Iniciando maquina de estados"<<std::endl;
        //Cambio de estado: [Cualquiera] ---> ST_IDLE
        Globals.SMObject.InitStateMachine();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        //Cambio de estado: ST_IDLE ---> ST_CONNECT
        Connection = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_ANY);
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (Connection == 0){
            
            PortO = Globals.PelicanoObject.PortO;
            //Cambio de estado: ST_CONNECT ---> ST_CHECK
            Check = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_SUCCESS_CONN);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

            Response = CheckCodes(Check);
        }
        else {
            //Cambio de estado: ST_CONNECT ---> ST_ERROR
            Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
            Response.StatusCode = 505;
            Response.Message = "Fallo en la conexion con el validador, puerto no encontrado";
        }
        return Response;
    }

    Response_t PelicanoControlClass::CheckDevice() {

        int Check = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        Check = Globals.SMObject.RunCheck();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        
        Response = CheckCodes(Check);

        return Response;
    }

    Response_t PelicanoControlClass::StartReader() {
        
        int Enable = -1;
        int Poll = -1;
        int Reset = -1;
        int Check = -1;
        int CleanBowl = -1;

        bool FlagReady = false;
        bool FlagBowlInBadState = false;
        bool FlagRepeat = true;
        WarnCounter = 0;
        CriticalCounter = 0;

        CoinEventPrev = 0;
        Remaining = 0;

        FlagCritical = false;
        FlagCritical2 = false;

        //Deberia entrar aca desde el estado ST_CHECK
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_CHECK") == 0){
            // Si la bandeja esta limpia y cerrada se activa bandera para que comience a inicar el polling
            if ((Globals.PelicanoObject.CoinPresent == false) & (Globals.PelicanoObject.TrashDoorOpen == false)){
                FlagReady = true;
            }
            // Si la bandeja no esta limpia o cerrada se cambia al estado CLEANBOWL para limpiarla y cerrarla
            else {
                //Cambio de estado: ST_CHECK ---> ST_CLEANBOWL
                CleanBowl = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_TRASH);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                if ((CleanBowl == 0) | (CleanBowl == 2)){
                    //Cambio de estado: ST_CLEANBOWL ---> ST_CHECK
                    Check = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_ANY);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                    Response = CheckCodes(Check);

                    if (Check == 0){
                        FlagReady = true;
                    }
                    else if(Check == 2){
                        FlagBowlInBadState = true;
                    }
                }
            }
        }
        else if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            //Se revisa que el evento este reiniciado y con buen estado en el bowl para poder comenzar a hacer polling correctamente
            if ((Globals.PelicanoObject.CoinEvent <= 1) & (Globals.PelicanoObject.CoinPresent == false) & (Globals.PelicanoObject.TrashDoorOpen == false)){
                FlagRepeat = false;
            }
            //Se debe cambiar de estado a reset cuando el evento no esta reiniciado o el bowl esta en mal estado y por ultimo se cambia al estado check
            else {
                //Cambio de estado: ST_POLLING ---> ST_CLEANBOWL
                CleanBowl = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_FINISH_POLL);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                if ((CleanBowl == 0) | (CleanBowl == 2)){
                    //Cambio de estado: ST_CLEANBOWL ---> ST_RESET
                    Reset = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_EMPTY);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                    if (Reset == 0){
                        //Cambio de estado: ST_RESET ---> ST_CHECK
                        Check = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_LOOP);
                        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                        if (Check == 0){
                            FlagReady = true;
                        }
                        else if(Check == 2){
                            FlagBowlInBadState = true;
                        }
                    }
                }
            }
        }
        else {
            //Para cualquier otro estado diferente a ST_CHECK o ST_POLLING se vuelve a reiniciar la maquina de estados, llevandola hasta ST_CHECK
            //Cambio de estado: [Cualquiera] ---> ST_CHECK
            Response = Connect();
            if (Response.StatusCode == 200){
                FlagReady = true;
            }    
        }

        if (FlagReady){
            //Si llega hasta este punto, debe estar en el estado ST_CHECK
            //Cambio de estado: ST_CHECK ---> ST_ENABLE
            Enable = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_CALL_POLLING);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
            if (Enable == 0){
                //Cambio de estado: ST_ENABLE ---> ST_POLLING
                Poll = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_READY);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                if (((Poll == 0) | (Poll == -2)) & (Globals.PelicanoObject.CoinEvent <= 1)){
                    Response.StatusCode = 201;
                    Response.Message = "Validador OK. Listo para iniciar a leer monedas";
                }
                else {
                    Response.StatusCode = 503;
                    Response.Message = "Fallo con el validador. No responde";
                }
            }
            else {
                if (Globals.PelicanoObject.CoinEvent > 1){
                    Response.StatusCode = 506;
                    Response.Message = "Fallo con el validador. Validador no reinicio aunque se intento reiniciar";
                }
                else {
                    Response.StatusCode = 503;
                    Response.Message = "Fallo con el validador. No responde";
                }
            }
        }
        else {
            if (FlagBowlInBadState == false){
                Response = CheckCodes(1);
            }
            else if (FlagRepeat) {
                Response = CheckCodes(2);
            }
            else {
                Response.StatusCode = 202;
                Response.Message = "Start reader corrio nuevamente. Listo para iniciar";
            }
        }
        return Response;
    }

    CoinError_t PelicanoControlClass::GetCoin() {
        
        CoinError_t ResponseCE;

        ResponseCE.StatusCode = 404;
        ResponseCE.Event = 0;
        ResponseCE.Coin = 0;
        ResponseCE.Message = DEFAULTERROR;
        ResponseCE.Remaining = 0;

        int Poll = -1;

        //Deberia entrar aca desde el estado ST_POLLING
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            //Cambio de estado: ST_POLLING ---> ST_POLLING
            Poll = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_POLL);
            
            if (Globals.PelicanoObject.CoinEvent != CoinEventPrev){
                //std::cout<<"[MAIN] Evento actual: "<<Globals.PelicanoObject.CoinEvent<<" Evento previo: "<<CoinEventPrev<<std::endl;
                Remaining = Globals.PelicanoObject.CoinEvent - CoinEventPrev;

                if (Poll == 0){

                    ResponseCE.StatusCode = 202;
                    ResponseCE.Event = Globals.PelicanoObject.CoinEvent;
                    ResponseCE.Coin = Globals.PelicanoObject.ActOCoin;
                    ResponseCE.Message = "Moneda detectada";

                    WarnCounter = 0;
                    CriticalCounter = 0;
                }
                else if (Poll == -2){

                    if (Globals.PelicanoObject.ErrorOCode == 1){

                        ResponseCE.StatusCode = 302;
                        ResponseCE.Event = Globals.PelicanoObject.CoinEvent;
                        ResponseCE.Coin = 0;
                        ResponseCE.Message = "Moneda rechazada";
                        WarnCounter++;
                    }
                    else{

                        if (Globals.PelicanoObject.ErrorOCritical == 1){
                            CriticalCounter++;
                        }

                        if ((Globals.PelicanoObject.ErrorOCode == 5) |
                           (Globals.PelicanoObject.ErrorOCode == 6) | (Globals.PelicanoObject.ErrorOCode == 9) | 
                           (Globals.PelicanoObject.ErrorOCode == 10) | (Globals.PelicanoObject.ErrorOCode == 20) | 
                           (Globals.PelicanoObject.ErrorOCode == 115) | (Globals.PelicanoObject.ErrorOCode == 116) |
                           (Globals.PelicanoObject.ErrorOCode == 119) | (Globals.PelicanoObject.ErrorOCode == 254)) {
                            WarnCounter++;
                        }
                        
                        if (Globals.PelicanoObject.ErrorOCode == 0){
                            CriticalCounter = 0;
                        }

                        if (CriticalCounter >= MaxCritical){
                            FlagCritical = true;
                        }

                        if (WarnCounter >= WarnToCritical){ 
                            FlagCritical2 = true;
                        }

                        std::string EC = std::to_string(Globals.PelicanoObject.ErrorOCode);
                        std::string WarnCounterStr = std::to_string(WarnCounter);
                        std::string CriticalCounterStr = std::to_string(CriticalCounter);
                        
                        ResponseCE.Event = Globals.PelicanoObject.CoinEvent;
                        ResponseCE.Coin = 0;

                        if (FlagCritical){
                            ResponseCE.StatusCode = 402;
                            ResponseCE.Message = "Codigo: " + EC + " Mensaje: " + Globals.PelicanoObject.ErrorOMsg + " CC: Full WC: " + WarnCounterStr;
                            FlagCritical2 = true;
                        }
                        else if(FlagCritical2){
                            ResponseCE.StatusCode = 403;
                            ResponseCE.Message = "Codigo: " + EC + " Mensaje: " + Globals.PelicanoObject.ErrorOMsg + " CC: " + CriticalCounterStr + " WC: Full";
                        }
                        else{
                            ResponseCE.StatusCode = 401;
                            ResponseCE.Message = "Codigo: " + EC + " Mensaje: " + Globals.PelicanoObject.ErrorOMsg + " CC: " + CriticalCounterStr + " WC: " + WarnCounterStr;
                        }
                    }
                }
                else{
                    ResponseCE.StatusCode = 503;
                    ResponseCE.Event = Globals.PelicanoObject.CoinEvent;
                    ResponseCE.Coin = 0;
                    ResponseCE.Message = "Fallo con el validador. No responde";
                    FlagCritical = true;
                }

                if (Remaining > 1){
                    ResponseCE.Remaining = Remaining;                            
                }

                CoinEventPrev = (Globals.PelicanoObject.CoinEvent == 255) ? 0 : Globals.PelicanoObject.CoinEvent;
                
            }
            else{
                ResponseCE.StatusCode = 303;
                ResponseCE.Event = CoinEventPrev;
                ResponseCE.Coin = 0;
                ResponseCE.Message = "No hay nueva informacion";
            }
        }
        else{
            ResponseCE.StatusCode = 507;
            ResponseCE.Message = "No se ha iniciado el lector (StartReader)";
        }
        return ResponseCE;
    }

    CoinLost_t PelicanoControlClass::GetLostCoins() {

        CoinLost_t ResponseLC;

        ResponseLC.CoinCinc = 0;
        ResponseLC.CoinCien = 0;
        ResponseLC.CoinDosc = 0;
        ResponseLC.CoinQuin = 0;
        ResponseLC.CoinMil = 0;

        if(strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            ResponseLC.CoinCinc = Globals.PelicanoObject.CoinCinc;
            ResponseLC.CoinCien = Globals.PelicanoObject.CoinCien;
            ResponseLC.CoinDosc = Globals.PelicanoObject.CoinDosc;
            ResponseLC.CoinQuin = Globals.PelicanoObject.CoinQuin;
            ResponseLC.CoinMil = Globals.PelicanoObject.CoinMil;
        }

        return ResponseLC;
    }
    
    Response_t PelicanoControlClass::ModifyChannels(int InhibitMask1,int InhibitMask2) {

        int Inhibit = Globals.PelicanoObject.ChangeInhibitChannels(InhibitMask1,InhibitMask2);

        if (Inhibit == 0){
            Response.StatusCode = 203;
            Response.Message = "Validador OK. Canales inhibidos correctamente";
        }
        else{
            Response.StatusCode = 508;
            Response.Message = "Fallo con el validador. No se pudieron inhibir los canales";
        }

        return Response;
    }

    Response_t PelicanoControlClass::StopReader() {
              
        //Deberia entrar aca desde el estado ST_POLLING

        int CleanBowl = -1;
        int Reset = -1;
        int Check = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if(strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            if ((FlagCritical) | (FlagCritical2)) {
                //Cambio de estado: ST_POLLING ---> ST_CLEANBOWL
                Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_FINISH_POLL);
                //Cambio de estado: ST_CLEANBOWL ---> ST_RESET
                Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_EMPTY);
                //Cambio de estado: ST_RESET ---> ST_ERROR
                Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_ERROR);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                Response.StatusCode = 509;
                Response.Message = "Fallo en el deposito. Hubo un error critico";
            }
            else {
                //Cambio de estado: ST_POLLING ---> ST_CLEANBOWL
                CleanBowl = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_FINISH_POLL);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                if (CleanBowl == 0){
                    //Cambio de estado: ST_CLEANBOWL ---> ST_RESET
                    Reset = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_EMPTY);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                    if (Reset == 0){
                        //Cambio de estado: ST_RESET ---> ST_CHECK
                        Check = Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_LOOP);
                        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                        Response = CheckCodes(Check);
                    }
                    else{
                        //Cambio de estado: ST_RESET ---> ST_ERROR
                        Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_ERROR);
                        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                        Response.StatusCode = 506;
                        Response.Message = "Fallo con el validador. Validador no reinicio aunque se intento reiniciar";
                    }
                }
                else{
                    //Cambio de estado: ST_CLEANBOWL ---> ST_ERROR
                    Globals.SMObject.StateMachineRun(PelicanoSMClass::EV_ERROR);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                    Response.StatusCode = 510;
                    Response.Message = "Fallo con el validador. Validador no limpio el bowl, aunque se intento limpiar";
                }
            }
        }
        else{
            Response.StatusCode = 405;
            Response.Message = "No se puede detener el lector porque no se ha iniciado";
        }

        return Response;
    }

    Response_t PelicanoControlClass::ResetDevice() {

        int Reset = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        Reset = Globals.SMObject.RunReset();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        
        if (Reset == 0){
            Response.StatusCode = 204;
            Response.Message = "Validador OK. Reset corrio exitosamente";
        }
        else {
            Response.StatusCode = 506;
            Response.Message = "Fallo con el validador. Validador no reinicio aunque se intento reiniciar";
        }

        return Response;
    }

    Response_t PelicanoControlClass::CleanDevice() {

        int Clean = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        Clean = Globals.SMObject.RunClean();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        
        if (Clean == 0){
            Response.StatusCode = 205;
            Response.Message = "Validador OK. Cleanbowl corrio exitosamente";
        }
        else {
            Response.StatusCode = 510;
            Response.Message = "Fallo con el validador. Validador no pudo limipar el bowl";
        }

        return Response;
    }

    Response_t PelicanoControlClass::GetInsertedCoins() {
        
        int Insert = Globals.PelicanoObject.GetCountCoins();
                
        if (Insert == 0){
            InsertedCoins = Globals.PelicanoObject.TotalInsertionCounter;
            std::string InsCoins = std::to_string(InsertedCoins);
            Response.StatusCode = 206;
            Response.Message = "Validador OK. Monedas insertadas: "+InsCoins;
        }
        else {
            Response.StatusCode = 511;
            Response.Message = "Fallo con el validador. GetInsertedCoins no pudo correr";
        }

        return Response;
    }

    TestStatus_t PelicanoControlClass::TestStatus() {

        TestStatus_t Status;
        
        Status.Version = VERSION;
        Status.Device = 3;

        if (Globals.PelicanoObject.FaultOCode != 0){
            Status.ErrorType = 0;
            Status.ErrorCode = Globals.PelicanoObject.FaultOCode;
            Status.Message = Globals.PelicanoObject.FaultOMsg;
            std::string EC = std::to_string(Globals.PelicanoObject.ErrorOCode);
            Status.AditionalInfo = "ErrorCode: " + EC + " ECMensaje: " + Globals.PelicanoObject.ErrorOMsg;
            Status.Priority = 1;
        }
        else{
            Status.ErrorType = 1;
            Status.ErrorCode = Globals.PelicanoObject.ErrorOCode;
            Status.Message = Globals.PelicanoObject.ErrorOMsg;
            
            Response = CheckDevice();

            if (Response.StatusCode == 301){
                Status.AditionalInfo = Response.Message;
                Status.Priority = 1;
            }
            else if (Response.StatusCode == 302){
                Status.AditionalInfo = Response.Message;
                Status.Priority = 1;
            }
            else {
                Status.AditionalInfo = "FaultCode: OK";
                Status.Priority = Globals.PelicanoObject.ErrorOCritical;
            }
        }

        return Status;
    }

    Response_t PelicanoControlClass::CheckCodes(int Check){

        if(Check == 0){
            Response.StatusCode = 200;
            Response.Message = "Validador OK. Todos los sensores reportan buen estado";
        }
        else if(Check == 2){
            if (Globals.PelicanoObject.CoinPresent){
                Response.StatusCode = 301;
                Response.Message = "Alerta en el validador. Hay algo presente en la bandeja";
            }
            else if(Globals.PelicanoObject.TrashDoorOpen){
                Response.StatusCode = 302;
                Response.Message = "Alerta en el validador. La puerta de la bandeja esta abierta";
            }
            else{
                Response.StatusCode = 504;
                Response.Message = "Fallo en el codigo del validador. Revisar codigo en C";
            }
        }
        else{
            if (Globals.PelicanoObject.LowerSensorBlocked){
                Response.StatusCode = 501;
                Response.Message = "Fallo con el validador. Sensor bajo esta bloqueado";
            }
            else if (Globals.PelicanoObject.UpperSensorBlocked){
                Response.StatusCode = 502;
                Response.Message = "Fallo con el validador. Sensor alto esta bloqueado";
            }
            else{
                Response.StatusCode = 503;
                Response.Message = "Fallo con el validador. No responde";
            }
        }
        return Response;
    }
}

