#include <functional>

class Executable
{
public:
    Executable() : m_exec( [](){} ) {} // empty lambda prevents exceptions when empty m_exec is called

    template<typename Func, typename... Args>
    Executable(Func&& func, Args&&... args) : m_exec(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) {}

    Executable(Executable&& another) noexcept : m_exec(std::move(another.m_exec)) {}

    Executable& operator=(Executable&& another) noexcept
    {
        m_exec = std::move(another.m_exec);
    }

    void operator()() const
    {
        m_exec();
    }

    // this is move-only class
    Executable(const Executable&) = delete;
    Executable& operator=(const Executable&) = delete;

private:
    std::function<void()> m_exec;

};