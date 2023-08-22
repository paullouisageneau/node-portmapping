#include "callback.hpp"

#include <stdexcept>

const char *Callback::CancelException::what() const throw() { return "Callback cancelled"; }

Callback::Callback(Napi::Function callback) {
    if (!callback.IsFunction())
        throw Napi::Error::New(callback.Env(), "Callback must be a function");

    Napi::Env env = callback.Env();

    receiver = Napi::Persistent(static_cast<Napi::Value>(Napi::Object::New(env)));
    tsfn = tsfn_t::New(env, std::move(callback), "Callback callback", 0, 1, &receiver);
}

Callback::~Callback() { tsfn.Abort(); }

void Callback::call(arg_func_t argFunc) {
    Data *data = new Data{std::move(argFunc)};
    if (tsfn.BlockingCall(data) != napi_ok) {
        delete data;
        throw std::runtime_error("Failed to call JavaScript callback");
    }
}

void Callback::Func(Napi::Env env, Napi::Function callback, Napi::Reference<Napi::Value> *context,
                    Data *data) {
    if (!data)
        return;

    arg_vector_t args;
    arg_func_t argFunc(std::move(data->argFunc));
    delete data;

    try {
        argFunc(env, args);
    } catch (CancelException &) {
        return;
    }

    if (env && callback)
        callback.Call(context->Value(), args);
}
