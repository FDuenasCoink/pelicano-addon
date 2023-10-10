#include "Pelicano.hpp"

std::thread nativeThreadPelicano;
Napi::ThreadSafeFunction tsfnPelicano;
static bool isRunningPelicano = true;
static bool threadEndedPelicano = true;

Napi::FunctionReference Pelicano::constructor;

Napi::Object Pelicano::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "Peliacno", {
    InstanceMethod("connect", &Pelicano::Connect),
    InstanceMethod("checkDevice", &Pelicano::CheckDevice),
    InstanceMethod("startReader", &Pelicano::StartReader),
    InstanceMethod("getCoin", &Pelicano::GetCoin),
    InstanceMethod("getLostCoins", &Pelicano::GetLostCoins),
    InstanceMethod("modifyChannels", &Pelicano::ModifyChannels),
    InstanceMethod("stopReader", &Pelicano::StopReader),
    InstanceMethod("resetDevice", &Pelicano::ResetDevice),
    InstanceMethod("testStatus", &Pelicano::TestStatus),
    InstanceMethod("cleanDevice", &Pelicano::CleanDevice),
    InstanceMethod("onCoin", &Pelicano::OnCoin),
    InstanceMethod("getInsertedCoins", &Pelicano::GetInsertedCoins),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Pelicano", func);
  return exports;
}

Pelicano::Pelicano(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Pelicano>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length(); 
  if (length != 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }

  Napi::Object params = info[0].As<Napi::Object>();
  if (
    !params.Has("warnToCritical") || 
    !params.Has("maxCritical") ||
    !params.Has("maximumPorts") ||
    !params.Has("logPath") ||
    !params.Has("logLevel")
  ) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  
  Napi::Number WarnToCritical = params.Get("warnToCritical").ToNumber();
  Napi::Number MaxCritical = params.Get("maxCritical").ToNumber();
  Napi::Number MaximumPorts = params.Get("maximumPorts").ToNumber();
  Napi::Number LogLvl = params.Get("logLevel").ToNumber();
  Napi::String LogFilePath = params.Get("logPath").ToString();

  this->pelicanoControl_ = new PelicanoControlClass();
  this->pelicanoControl_->WarnToCritical = WarnToCritical.Int32Value();
  this->pelicanoControl_->MaxCritical = MaxCritical.Int32Value();
  this->pelicanoControl_->MaximumPorts = MaximumPorts.Int32Value();
  this->pelicanoControl_->LogLvl = LogLvl.Uint32Value();
  this->pelicanoControl_->Path = LogFilePath.Utf8Value();

  this->pelicanoControl_->InitLog();
}

Napi::Value Pelicano::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->pelicanoControl_->Connect();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::CheckDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->pelicanoControl_->CheckDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::StartReader(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->pelicanoControl_->StartReader();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::GetCoin(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  CoinError_t response = this->pelicanoControl_->GetCoin();
  Napi::Object object = Napi::Object::New(env);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  object["event"] = Napi::Number::New(env, response.Event);
  object["coin"] = Napi::Number::New(env, response.Coin);
  object["message"] = Napi::String::New(env, response.Message);
  object["remaining"] = Napi::Number::New(env, response.Remaining);
  return object;
}

Napi::Value Pelicano::GetLostCoins(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  CoinLost_t response = this->pelicanoControl_->GetLostCoins();
  Napi::Object object = Napi::Object::New(env);
  object["50"] = Napi::Number::New(env, response.CoinCinc);
  object["100"] = Napi::Number::New(env, response.CoinCien);
  object["200"] = Napi::Number::New(env, response.CoinDosc);
  object["500"] = Napi::Number::New(env, response.CoinQuin);
  object["1000"] = Napi::Number::New(env, response.CoinMil);
  return object;
}

Napi::Value Pelicano::ModifyChannels(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
     Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  Napi::Number InhibitMask1 = info[0].As<Napi::Number>();
  Napi::Number InhibitMask2 = info[1].As<Napi::Number>();

  Response_t response = this->pelicanoControl_->ModifyChannels(InhibitMask1.Int32Value(), InhibitMask2.Int32Value());
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::StopReader(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  isRunningPelicano = false;
  Response_t response = this->pelicanoControl_->StopReader();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::ResetDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->pelicanoControl_->ResetDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::TestStatus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  TestStatus_t response = this->pelicanoControl_->TestStatus();
  Napi::Object object = Napi::Object::New(env);
  object["version"] = Napi::String::New(env, response.Version);
  object["device"] = Napi::Number::New(env, response.Device);
  object["errorType"] = Napi::Number::New(env, response.ErrorType);
  object["errorCode"] = Napi::Number::New(env, response.ErrorCode);
  object["message"] = Napi::String::New(env, response.Message);
  object["aditionalInfo"] = Napi::String::New(env, response.AditionalInfo);
  object["priority"] = Napi::Number::New(env, response.Priority);
  return object;
}

Napi::Value Pelicano::CleanDevice(const Napi::CallbackInfo &info) { 
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->pelicanoControl_->CleanDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Pelicano::GetInsertedCoins(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->pelicanoControl_->GetInsertedCoins();
  long insertedCoins = this->pelicanoControl_->InsertedCoins;
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  object["insertedCoins"] = Napi::Number::New(env, insertedCoins);
  return object;
}

Napi::Value Pelicano::OnCoin(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsFunction()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  
  Napi::Function napiFunction = info[0].As<Napi::Function>();
  tsfnPelicano = Napi::ThreadSafeFunction::New(
    env, 
    napiFunction, 
    "Callback", 
    0, 
    1,
    []( Napi::Env ) {
      nativeThreadPelicano.join();
    });

  nativeThreadPelicano = std::thread ( [this] {
    auto callback = [](Napi::Env env, Napi::Function jsCallback, CoinError_t* coin) {
      Napi::Object object = Napi::Object::New(env);
      object["statusCode"] = Napi::Number::New(env, coin->StatusCode);
      object["event"] = Napi::Number::New(env, coin->Event);
      object["coin"] = Napi::Number::New(env, coin->Coin);
      object["message"] = Napi::String::New(env, coin->Message);
      object["remaining"] = Napi::Number::New(env, coin->Remaining);
      jsCallback.Call({object});
      delete coin;
    };
    isRunningPelicano = true;
    threadEndedPelicano = false;
    while (isRunningPelicano) {
      std::this_thread::sleep_for( std::chrono::milliseconds(10));
      CoinError_t response = this->pelicanoControl_->GetCoin();
      if (response.StatusCode == 303) continue;
      CoinError_t *value = new CoinError_t(response);
      napi_status status = tsfnPelicano.BlockingCall(value, callback);
      if ( status != napi_ok ) break;
    }
    threadEndedPelicano = true;
    tsfnPelicano.Release();
  });

  auto finishFn = [] (const Napi::CallbackInfo& info) {
    isRunningPelicano = false;
    while (!threadEndedPelicano);
    std::this_thread::sleep_for( std::chrono::milliseconds(50));
    return;
  };

  return Napi::Function::New(env, finishFn);
}