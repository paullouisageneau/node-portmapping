#include "wrapper.hpp"
#include "mapping.hpp"

Napi::Object Wrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    exports.Set("init", Napi::Function::New(env, &Wrapper::init));
    exports.Set("cleanup", Napi::Function::New(env, &Wrapper::cleanup));
    exports.Set("createMapping", Napi::Function::New(env, &Wrapper::createMapping));
    exports.Set("getLocalAddress", Napi::Function::New(env, &Wrapper::getLocalAddress));

    return exports;
}

void Wrapper::init(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    Napi::Object param;
    if (info.Length() < 1 || info[0].IsUndefined() || info[0].IsNull())
        param = Napi::Object::New(env);
    else if (info[0].IsObject())
        param = info[0].ToObject();
    else {
        Napi::TypeError::New(env, "init expects an optional object").ThrowAsJavaScriptException();
        return;
    }

    plum_config_t config = {};
    config.log_level = PLUM_LOG_LEVEL_NONE;
    config.log_callback = NULL;

    Napi::Value logLevel = param.Get("logLevel");
    if (!logLevel.IsUndefined() && !logLevel.IsNull()) {
        if (!logLevel.IsString()) {
            Napi::TypeError::New(env, "logLevel must be a string").ThrowAsJavaScriptException();
            return;
        }

        std::string str = logLevel.As<Napi::String>().Utf8Value();
        if (str == "Verbose")
            config.log_level = PLUM_LOG_LEVEL_VERBOSE;
        else if (str == "Debug")
            config.log_level = PLUM_LOG_LEVEL_DEBUG;
        else if (str == "Info")
            config.log_level = PLUM_LOG_LEVEL_INFO;
        else if (str == "Warning")
            config.log_level = PLUM_LOG_LEVEL_WARN;
        else if (str == "Error")
            config.log_level = PLUM_LOG_LEVEL_ERROR;
        else if (str == "Fatal")
            config.log_level = PLUM_LOG_LEVEL_FATAL;
        else {
            Napi::TypeError::New(env, "logLevel is invalid").ThrowAsJavaScriptException();
            return;
        }
    }

    if (plum_init(&config) < 0) {
        Napi::Error::New(env, "init failed").ThrowAsJavaScriptException();
        return;
    }
}

class CleanupWorker : public Napi::AsyncWorker {
public:
    CleanupWorker(Napi::Env &env, Napi::Promise::Deferred &deferred)
        : AsyncWorker(env), mDeferred(deferred) {}
    ~CleanupWorker() {}

    void Execute() override {
        if (plum_cleanup() < 0) {
            Napi::AsyncWorker::SetError("cleanup failed");
            return;
        }
    }

    void OnOK() override { mDeferred.Resolve(Env().Undefined()); }

    void OnError(const Napi::Error &error) override { mDeferred.Reject(error.Value()); }

private:
    Napi::Promise::Deferred &mDeferred;
};

Napi::Value Wrapper::cleanup(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    CleanupWorker *worker = new CleanupWorker(env, deferred);
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Wrapper::createMapping(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "create expects an object or number")
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }

    Napi::Object param;
    if (info[0].IsObject()) {
        param = info[0].As<Napi::Object>();
    } else if (info[0].IsNumber()) {
        param = Napi::Object::New(env);
        param.Set("internalPort", info[0].As<Napi::Number>());
    } else {
        Napi::TypeError::New(env, "create expects an object or number")
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }

    if (info.Length() >= 2) {
        if (!info[1].IsFunction()) {
            Napi::TypeError::New(env, "create expects an optional callback")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        param.Set("callback", info[1].As<Napi::Function>());
    }

    return Mapping::constructor.New({param});
}

Napi::Value Wrapper::getLocalAddress(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    char buffer[PLUM_MAX_ADDRESS_LEN];
    if (plum_get_local_address(buffer, PLUM_MAX_ADDRESS_LEN) < 0)
        return env.Undefined();

    return Napi::String::New(env, buffer);
}

