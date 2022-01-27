#ifndef MAPPING_H
#define MAPPING_H

#include "callback.hpp"

#include <iostream>
#include <memory>
#include <string>

#include "plum/plum.h"

class Mapping : public Napi::ObjectWrap<Mapping> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    Mapping(const Napi::CallbackInfo &info);
    ~Mapping();

    void destroy(const Napi::CallbackInfo &info);

    Napi::Value getInfo(const Napi::CallbackInfo &info);

private:
    Napi::Value Convert(Napi::Env env, plum_state_t state, const plum_mapping_t &info);
    static void CallbackFunc(int id, plum_state_t state, const plum_mapping_t *info);
    void onCallback(plum_state_t state, const plum_mapping_t *mapping);

    int mId;
    std::unique_ptr<Callback> mCallback;
};

#endif // MAPPING_H
