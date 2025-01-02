////
// @file error.h
// @brief
// 定义错误码
// bigant错误码从1024开始，0-255保留给系统。
//
// @author niexw
// @email niexiaowen@uestc.edu.cn
//
#pragma once
#include <errno.h>
#include <system_error>
#include <exception>

namespace backup {

// 错误码
enum backup_error_code : int
{
    OK = 0,
    EEOF = 1024,
    ERROR,
    NO_PERMISSION,
    NOT_EXIST,
    ENCRYPTED,      // 文件已加密
    FORMAT_ERROR,   // 文件格式错误
    PASSWORD_ERROR, // 密码错误
    EMPTY_FILENAME, // 文件名为空
    EMPTY_FILEPATH, // 文件路径为空
    UNABLE_HASH,    // 无法计算哈希
};

// 错误类型
class backup_error_category : public std::error_category
{
  public:
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

// 错误类型单件
const backup_error_category &singleton_backup_error_category();

// 辅助函数
std::error_code backup_make_error_code(int e);
std::error_code backup_make_error_code(backup_error_code e);

using ErrorCode = backup_error_code;
// typedef backup_error_code ErrorCode;

// 对外提供的接口都应该包含 code + msg两者
struct Status : std::exception
{
    Status(int code, const std::string &s)
        : code(code)
        , msg(s)
    {}
    Status(int code, std::string &&s) noexcept
        : code(code)
        , msg(std::move(s))
    {}

    const char *what() const noexcept override { return msg.c_str(); }

    int code;
    std::string msg;
};

} // namespace backup