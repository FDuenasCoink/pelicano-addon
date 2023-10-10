#include <napi.h>
#include "pelicano/Pelicano.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Pelicano::Init(env, exports);
  return exports;
}

NODE_API_MODULE(oink_addons, InitAll);