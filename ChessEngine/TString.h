#pragma once
#include <string>

namespace std
{
#ifdef _UNICODE
    typedef std::wstring tstring;
#else
    typedef std::string tstring;
#endif
}
