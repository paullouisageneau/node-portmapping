#include "mapping.hpp"
#include "wrapper.hpp"

#include <napi.h>

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    Wrapper::Init(env, exports);
    Mapping::Init(env, exports);

    return exports;
}

NODE_API_MODULE(nodePortMapping, InitAll)
