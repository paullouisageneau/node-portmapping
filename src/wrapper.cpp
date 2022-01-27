#include "wrapper.hpp"
#include "mapping.hpp"

Napi::Object Wrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    exports.Set("init", Napi::Function::New(env, &Wrapper::init));
    exports.Set("cleanup", Napi::Function::New(env, &Wrapper::cleanup));
    exports.Set("createMapping", Napi::Function::New(env, &Wrapper::createMapping));
    exports.Set("getLocalAddress", Napi::Function::New(env, &Wrapper::getLocalAddress));
    exports.Set("getDummyTlsCertificate",
                Napi::Function::New(env, &Wrapper::getDummyTlsCertificate));
    exports.Set("getDummyTlsHost", Napi::Function::New(env, &Wrapper::getDummyTlsHost));

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
    config.dummytls_domain = NULL;

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

    Napi::Value dummyTlsDomain = param.Get("dummyTlsDomain");
    if (!dummyTlsDomain.IsUndefined() && !dummyTlsDomain.IsNull()) {
        if (!dummyTlsDomain.IsString()) {
            Napi::TypeError::New(env, "dummyTlsDomain must be a string")
                .ThrowAsJavaScriptException();
            return;
        }

        std::string str = dummyTlsDomain.As<Napi::String>().Utf8Value();
        config.dummytls_domain = str.c_str();
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

Napi::Value Wrapper::getDummyTlsCertificate(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "getDummyTlsCertificate expects an address as string")
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::string str = info[0].As<Napi::String>().Utf8Value();

    plum_dummytls_cert_type_t type;
    if (str == "Cert")
        type = PLUM_DUMMYTLS_PEM_CERT;
    else if (str == "Chain")
        type = PLUM_DUMMYTLS_PEM_CHAIN;
    else if (str == "FullChain")
        type = PLUM_DUMMYTLS_PEM_FULLCHAIN;
    else if (str == "PrivKey")
        type = PLUM_DUMMYTLS_PEM_PRIVKEY;
    else {
        Napi::TypeError::New(env, "certificate type is invalid").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    const size_t bufferSize = 8192;
    char buffer[bufferSize];
    if (plum_get_dummytls_certificate(type, buffer, bufferSize) < 0)
        return env.Undefined();

    return Napi::String::New(env, buffer);
}

Napi::Value Wrapper::getDummyTlsHost(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "getDummyTlsHost expects an address as string")
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::string address = info[0].As<Napi::String>().Utf8Value();

    char buffer[PLUM_MAX_HOST_LEN];
    if (plum_get_dummytls_host(address.c_str(), buffer, PLUM_MAX_HOST_LEN) < 0)
        return env.Undefined();

    return Napi::String::New(env, buffer);
}
