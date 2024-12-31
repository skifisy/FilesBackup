#ifndef UTIL_H
#define UTIL_H

#include <ctime>
#include <QString>
#include <unistd.h>
#include <file_sys.h>
// 时间格式化显示
QString FormatTime(const time_t & t, const QString& format = "yyyy-MM-dd HH:mm");

// 权限格式化显示
QString FormatPermission(mode_t permission, backup::FileType type);

QString FormatFileSize(uint64_t size);

#endif // UTIL_H