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

#include <system_error>

namespace backup {

// 错误码
enum backup_error_code : int
{
    OK = 0,
    EEOF,
    ERROR,
    NO_PERMISSION,
    NOT_EXIST,
    ENCRYPTED,  // 文件已加密
    FORMAT_ERROR,  // 文件格式错误 
    PASSWORD_ERROR,  // 密码错误
    EMPTY_FILENAME,  // 文件名为空
    EMPTY_FILEPATH,  // 文件路径为空
    
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

// 对外提供的接口都应该包含 code + msg两者
struct Status
{
    int code;
    std::string msg;
};

} // namespace backup