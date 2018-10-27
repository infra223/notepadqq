#ifndef SCOPEGUARD_H
#define SCOPEGUARD_H

#include <utility>

// ScopeGuard is a utility class that takes a function object and executes it
// when the ScopeGuard object reaches the end of its lifetime.
template <typename F>
class ScopeGuard {
public:
    ScopeGuard( F func ) : func( std::move(func) ) {}
    ~ScopeGuard( ) { if(active) func( ); }

    // Triggers the scope guard's content and disables it
    void trigger() { active = false; func(); }

    // Disables the scope guard.
    void dismiss() { active = false; }

    ScopeGuard() = delete;
    ScopeGuard(ScopeGuard&& other) : func(std::move(other.func)) {}
    ScopeGuard& operator=(const ScopeGuard& other) = delete;
private:
    bool active = true;
    F func;
};

namespace detail {
struct defer_dummy { };
template<typename F>
ScopeGuard<F> operator+( defer_dummy, F&& f )
{
    return ScopeGuard<F>(std::forward<F>(f));
}
}

#define DEFER auto anonScopeGuard ## __LINE__ = detail::defer_dummy() + [&]()
#define SCOPE_GUARD(name) auto name = detail::defer_dummy() + [&]()

#endif // SCOPEGUARD_H
