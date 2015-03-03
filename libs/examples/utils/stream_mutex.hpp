#pragma once

#include <iostream>
#include <mutex>

//! この提案を参考にした http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3395.html

namespace hwm {

namespace detail { namespace stream_mutex {

template<class Stream>
struct stream_mutex;

template<class Stream>
struct stream_guard
{
    typedef Stream base_type;
    typedef Stream stream_type;
    typedef stream_guard<Stream> this_type;
    typedef stream_mutex<Stream> owner_type;

public:
    explicit
    stream_guard(owner_type &owner)
        :   stream_(&owner.bypass())
        ,   lock_(owner.get_mutex())
    {}

    ~stream_guard()
    {}

    stream_guard(this_type const &) = delete;
    this_type & operator=(this_type const &) = delete;

    stream_guard(this_type &&rhs)
        :   stream_(rhs.stream_)
        ,   lock_(std::move(rhs.lock_))
    {
        rhs.stream_ = nullptr;
    }

    stream_guard & operator=(this_type &&rhs)
    {
        stream_ = rhs.stream_;
        rhs.stream_ = nullptr;
        lock_ = std::move(rhs.lock_);
        return *this;
    }

    stream_type & bypass() const { return *stream_; }

private:
    stream_type * stream_;
    std::unique_lock<std::mutex> lock_;
};

template<class Stream, class T>
stream_guard<Stream> const &
    operator<<(stream_guard<Stream> const &os, T &&arg)
{
    os.bypass() << std::forward<T>(arg);
    return os;
}

template<class Stream>
stream_guard<Stream> const &
    operator<<(
        stream_guard<Stream> const &os,
        Stream & (*manip)(Stream &)
        )
{
    os.bypass() << manip;
    return os;
}

template<class Stream>
struct stream_mutex
{
    typedef Stream stream_type;
    typedef stream_guard<stream_type> guard_type;

    stream_mutex(stream_type &stream)
        :   stream_(&stream)
    {}

    guard_type hold() {
        return guard_type(*this);
    }

    std::mutex & get_mutex() { return mtx_; }
    stream_type & bypass() { return *stream_; }

private:
    std::mutex mtx_;
    stream_type *stream_;
};

template <class Stream, class T>
stream_guard<Stream>
    operator<<(stream_mutex<Stream> &mtx, T &&arg)
{
    auto guard = mtx.hold();
    guard << std::forward<T>(arg);
    return guard;
}

template <class Stream>
stream_guard<Stream>
    operator<<(
        stream_mutex<Stream> &mtx,
        Stream & (*manip)(Stream &)
        )
{
    auto guard = mtx.hold();
    guard << manip;
    return guard;
}

template<class Stream>
stream_mutex<Stream> make_stream_mutex(Stream &stream)
{
    return stream_mutex<Stream>(stream);
}

#if defined(_MSC_VER)
    __declspec(selectany) std::ios_base::Init _init;
    __declspec(selectany) stream_mutex<std::istream> mcin { std::cin };
    __declspec(selectany) stream_mutex<std::ostream> mcout { std::cout };
    __declspec(selectany) stream_mutex<std::ostream> mcerr { std::cerr };
    __declspec(selectany) stream_mutex<std::ostream> mclog { std::clog };
    __declspec(selectany) stream_mutex<std::wistream> mwcin { std::wcin };
    __declspec(selectany) stream_mutex<std::wostream> mwcout { std::wcout };
    __declspec(selectany) stream_mutex<std::wostream> mwcerr { std::wcerr };
    __declspec(selectany) stream_mutex<std::wostream> mwclog { std::wclog };
#else
    __attribute__((weak)) std::ios_base::Init _init;
    __attribute__((weak)) stream_mutex<std::istream> mcin { std::cin };
    __attribute__((weak)) stream_mutex<std::ostream> mcout { std::cout };
    __attribute__((weak)) stream_mutex<std::ostream> mcerr { std::cerr };
    __attribute__((weak)) stream_mutex<std::ostream> mclog { std::clog };
    __attribute__((weak)) stream_mutex<std::wistream> mwcin { std::wcin };
    __attribute__((weak)) stream_mutex<std::wostream> mwcout { std::wcout };
    __attribute__((weak)) stream_mutex<std::wostream> mwcerr { std::wcerr };
    __attribute__((weak)) stream_mutex<std::wostream> mwclog { std::wclog };
#endif

}} // ::detail::stream_mutex


using detail::stream_mutex::stream_guard;
using detail::stream_mutex::stream_mutex;
using detail::stream_mutex::make_stream_mutex;

using detail::stream_mutex::mcin;
using detail::stream_mutex::mcout;
using detail::stream_mutex::mcerr;
using detail::stream_mutex::mclog;
using detail::stream_mutex::mwcin;
using detail::stream_mutex::mwcout;
using detail::stream_mutex::mwcerr;
using detail::stream_mutex::mwclog;

} // ::hwm

