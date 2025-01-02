////
// @file error.cc
// @brief
// 实现错误码
//
// @author hz
// @email skyfishine@163.com
//

#include "error_code.h"
#include <netdb.h>
#include <cstring>

namespace backup {

const char *backup_error_category::name() const noexcept
{
    return "backup::backup_error_category";
}
std::string backup_error_category::message(int ev) const
{
    if (ev < backup_error_code::EEOF) { return strerror(ev); }
    switch (static_cast<backup_error_code>(ev)) {
    case backup_error_code::OK:
        return "Success";
    case backup_error_code::EEOF:
        return "Read eof";
    case backup_error_code::ERROR:
        return "error";
    default:
        return "unknown error";
    }
}

// 单件错误类型
const backup_error_category &singleton_bigant_error_category()
{
    static backup_error_category instance;
    return instance;
}

std::error_code backup_make_error_code(int e)
{
    return std::error_code(e, singleton_bigant_error_category());
}

std::error_code backup_make_error_code(backup_error_code e)
{
    return std::error_code(
        static_cast<int>(e), singleton_bigant_error_category());
}

} // namespace backup