#include "mapping.hpp"

Napi::FunctionReference Mapping::constructor;

Napi::Object Mapping::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Mapping",
                                      {InstanceMethod("destroy", &Mapping::destroy),
                                       InstanceMethod("getInfo", &Mapping::getInfo)});

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Mapping", func);
    return exports;
}

Mapping::Mapping(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Mapping>(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Mapping expects an object").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object param = info[0].As<Napi::Object>();

    plum_mapping_t mappingInfo = {};
    mappingInfo.user_ptr = this;
    mappingInfo.protocol = PLUM_IP_PROTOCOL_TCP;

    Napi::Value internalPort = param.Get("internalPort");
    if (!internalPort.IsNumber()) {
        Napi::TypeError::New(env, "internalPort must be a number").ThrowAsJavaScriptException();
        return;
    }

    mappingInfo.internal_port = uint16_t(internalPort.As<Napi::Number>().Uint32Value());

    Napi::Value externalPort = param.Get("externalPort");
    if (!externalPort.IsUndefined() && !externalPort.IsNull()) {
        if (!externalPort.IsNumber()) {
            Napi::TypeError::New(env, "externalPort must be a number").ThrowAsJavaScriptException();
            return;
        }

        mappingInfo.external_port = uint16_t(externalPort.As<Napi::Number>().Uint32Value());
    }

    Napi::Value protocol = param.Get("protocol");
    if (!protocol.IsUndefined() && !protocol.IsNull()) {
        if (!protocol.IsString()) {
            Napi::TypeError::New(env, "protocol must be a string").ThrowAsJavaScriptException();
            return;
        }

        std::string str = protocol.As<Napi::String>().Utf8Value();
        if (str == "TCP")
            mappingInfo.protocol = PLUM_IP_PROTOCOL_TCP;
        else if (str == "UDP")
            mappingInfo.protocol = PLUM_IP_PROTOCOL_UDP;
        else {
            Napi::TypeError::New(env, "protocol is invalid").ThrowAsJavaScriptException();
            return;
        }
    }

    Napi::Value callback = param.Get("callback");
    if (!callback.IsUndefined() && !callback.IsNull()) {
        if (!callback.IsFunction()) {
            Napi::TypeError::New(env, "callback must be a function").ThrowAsJavaScriptException();
            return;
        }

        mCallback = std::make_unique<Callback>(callback.As<Napi::Function>());
    }

    mId = plum_create_mapping(&mappingInfo, CallbackFunc);
    if (mId < 0) {
        Napi::Error::New(env, "create failed").ThrowAsJavaScriptException();
        return;
    }
}

Mapping::~Mapping() {
    if (mId >= 0)
        plum_destroy_mapping(mId);
}

void Mapping::destroy(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (mId >= 0) {
        plum_destroy_mapping(mId);
        mId = -1;
    }
}

Napi::Value Mapping::getInfo(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (mId < 0) {
        Napi::Error::New(env, "mapping is destroyed").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    plum_state_t state;
    plum_mapping_t mappingInfo;
    if (plum_query_mapping(mId, &state, &mappingInfo) < 0) {
        Napi::Error::New(env, "mapping query failed").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    return Convert(env, state, mappingInfo);
}

namespace {

std::string to_string(plum_state_t state) {
    switch (state) {
    case PLUM_STATE_PENDING:
        return "Pending";
    case PLUM_STATE_SUCCESS:
        return "Success";
    case PLUM_STATE_FAILURE:
        return "Failure";
    case PLUM_STATE_DESTROYING:
        return "Destroying";
    default:
        return "Destroyed";
    }
}

} // namespace

Napi::Value Mapping::Convert(Napi::Env env, plum_state_t state, const plum_mapping_t &mappingInfo) {
    Napi::Object param = Napi::Object::New(env);
    param.Set("state", Napi::String::New(env, to_string(state)));
    param.Set("protocol",
              Napi::String::New(env, mappingInfo.protocol == PLUM_IP_PROTOCOL_UDP ? "UDP" : "TCP"));
    param.Set("internalPort", Napi::Number::New(env, mappingInfo.internal_port));

    if (mappingInfo.external_port != 0)
        param.Set("externalPort", Napi::Number::New(env, mappingInfo.external_port));

    if (*mappingInfo.external_host != '\0')
        param.Set("externalHost", Napi::String::New(env, mappingInfo.external_host));

    return param;
}

void Mapping::CallbackFunc(int id, plum_state_t state, const plum_mapping_t *mappingInfo) {
    Mapping *mapping = static_cast<Mapping *>(mappingInfo->user_ptr);
    if (!mapping)
        return;

    mapping->onCallback(state, mappingInfo);
}

void Mapping::onCallback(plum_state_t state, const plum_mapping_t *mappingInfo) {
    if (!mCallback)
        return;

    mCallback->call(
        [this, state, mappingInfo = *mappingInfo](Napi::Env env, std::vector<napi_value> &args) {
            if (mId < 0)
                throw Callback::CancelException();

            args.emplace_back(Convert(env, state, mappingInfo));
        });
}
