#ifndef WRAPPER_H
#define WRAPPER_H

#include <iostream>
#include <string>

#include <napi.h>

#include "plum/plum.h"

class Wrapper {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    static void init(const Napi::CallbackInfo &info);
    static Napi::Value cleanup(const Napi::CallbackInfo &info);

    static Napi::Value createMapping(const Napi::CallbackInfo &info);

    static Napi::Value getLocalAddress(const Napi::CallbackInfo &info);
};

#endif // WRAPPER_H
