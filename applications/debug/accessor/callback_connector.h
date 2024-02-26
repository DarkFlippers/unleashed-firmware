#ifndef CALLBACKCONNECTOR_H
#define CALLBACKCONNECTOR_H

#ifdef __cplusplus
#include <functional>
namespace cbc {
namespace Details {

template <std::size_t Tag, typename T, typename Ret, typename... Args>
class FuncMemberWrapper {
public:
    FuncMemberWrapper() = delete;
    using member_fun_t = Ret (T::*)(Args...);
    using const_member_fun_t = Ret (T::*)(Args...) const;
    static auto instantiate(T* t, member_fun_t ptr) {
        obj = t;
        member = ptr;
        return MetaCall;
    }

    static auto instantiate(T* t, const_member_fun_t ptr) {
        obj = t;
        const_member = ptr;
        return ConstMetaCall;
    }

private:
    static auto MetaCall(Args... args) {
        return (*obj.*member)(args...);
    }
    static auto ConstMetaCall(Args... args) {
        return (*obj.*const_member)(args...);
    }
    static T* obj;
    static member_fun_t member;
    static const_member_fun_t const_member;
};
template <std::size_t Tag, typename T, typename Ret, typename... Args>
T* FuncMemberWrapper<Tag, T, Ret, Args...>::obj{};

template <std::size_t Tag, typename T, typename Ret, typename... Args>
typename FuncMemberWrapper<Tag, T, Ret, Args...>::member_fun_t
    FuncMemberWrapper<Tag, T, Ret, Args...>::member{};

template <std::size_t Tag, typename T, typename Ret, typename... Args>
typename FuncMemberWrapper<Tag, T, Ret, Args...>::const_member_fun_t
    FuncMemberWrapper<Tag, T, Ret, Args...>::const_member{};

template <typename Functor, typename Ret, typename... Args>
struct FunctorWrapper {
public:
    static std::function<Ret(Args...)> functor;
    static auto instatiate(Functor fn) {
        functor = std::move(fn);
        return MetaCall;
    }

private:
    static auto MetaCall(Args... args) {
        return functor(args...);
    }
};

template <typename Functor, typename Ret, typename... Args>
std::function<Ret(Args...)> FunctorWrapper<Functor, Ret, Args...>::functor;

template <typename Functor, typename Ret, typename T, typename... Args>
auto deducer(Functor obj, Ret (T::*)(Args...) const) {
    return FunctorWrapper<Functor, Ret, Args...>::instatiate(std::move(obj));
}

template <typename Functor, typename Ret, typename T, typename... Args>
auto deducer(Functor obj, Ret (T::*)(Args...)) {
    return FunctorWrapper<Functor, Ret, Args...>::instatiate(std::move(obj));
}

template <std::size_t tag, typename T, typename Ret, typename... Args>
auto const_instantiate(T* t, Ret (T::*ptr)(Args...) const) {
    return FuncMemberWrapper<tag, T, Ret, Args...>::instantiate(t, ptr);
}

template <std::size_t tag, typename T, typename Func>
auto const_instantiate(T* t, Func ptr) {
    return const_instantiate(t, ptr);
}

} //end of Details scope

template <std::size_t tag = 0, typename T, typename Ret, typename... Args>
auto obtain_connector(T* t, Ret (T::*ptr)(Args...)) {
    return Details::FuncMemberWrapper<tag, T, Ret, Args...>::instantiate(t, ptr);
}

template <std::size_t tag = 0, typename T, typename Ret, typename... Args>
auto obtain_connector(T* t, Ret (T::*ptr)(Args...) const) {
    return Details::FuncMemberWrapper<tag, T, Ret, Args...>::instantiate(t, ptr);
}

template <typename Functor>
auto obtain_connector(Functor functor) {
    return Details::deducer(std::move(functor), &Functor::operator());
}
} //end of cbc scope

#endif // __cplusplus
#endif // CALLBACKCONNECTOR_H
