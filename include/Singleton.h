#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
#include <iostream>
#include <mutex>

template <class T>
class Singleton
{
public:
    static std::shared_ptr<T> getInstance()
    {
        static std::once_flag s_flag;
        std::call_once(s_flag,
                       [&]()
                       {
                           m_instance = std::shared_ptr<T>(new T);
                       });
        return m_instance;
    }
    ~Singleton() {};

protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton &operator=(const Singleton<T> &) = delete;
    static std::shared_ptr<T> m_instance;
};

template <class T>
std::shared_ptr<T> Singleton<T>::m_instance = nullptr;
#endif // SINGLETON_H