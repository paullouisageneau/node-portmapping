#ifndef CALLBACK_H
#define CALLBACK_H

#include <napi.h>

#include <functional>
#include <vector>

class Callback {
public:
    using arg_vector_t = std::vector<napi_value>;
    using arg_func_t = std::function<void(napi_env, arg_vector_t &)>;

    Callback(Napi::Function callback);
    ~Callback();

    Callback(const Callback &) = delete;
    Callback(Callback &&) = delete;

    Callback &operator=(const Callback &) = delete;
    Callback &operator=(Callback &&) = delete;

    void call(arg_func_t argFunc);

    class CancelException : public std::exception {
        const char *what() const throw();
    };

private:
    struct Data {
        arg_func_t argFunc;
    };
    using Context = std::nullptr_t;

    static void Func(Napi::Env env, Napi::Function callback, Context *context, Data *data);

    using tsfn_t = Napi::TypedThreadSafeFunction<Context, Data, Func>;
    tsfn_t tsfn;
};

#endif // CALLBACK_H
