#include "util.h"
#include <sys/stat.h>
#include <QDateTime>
QString FormatTime(const time_t &t, const QString &format)
{
    // 将 time_t 转换为 QDateTime 对象
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(t);

    // 格式化为字符串
    QString formattedTime = dateTime.toString(format);
    return formattedTime;
}

QString FormatPermission(mode_t permission, backup::FileType type)
{
    QString per_s;
    switch (type) {
    case backup::FileType::REG:
        per_s += '-';
        break;
    case backup::FileType::DIR:
        per_s += 'd';
        break;
    case backup::FileType::FIFO:
        per_s += 'p';
        break;
    case backup::FileType::FLNK:
        per_s += 'l';
        break;
    default:
        per_s += '?';
        break;
    }
    // 用户权限
    per_s += (permission & S_IRUSR) ? 'r' : '-';
    per_s += (permission & S_IWUSR) ? 'w' : '-';
    per_s += (permission & S_IXUSR) ? 'x' : '-';

    // 组权限
    per_s += (permission & S_IRGRP) ? 'r' : '-';
    per_s += (permission & S_IWGRP) ? 'w' : '-';
    per_s += (permission & S_IXGRP) ? 'x' : '-';

    // 其他权限
    per_s += (permission & S_IROTH) ? 'r' : '-';
    per_s += (permission & S_IWOTH) ? 'w' : '-';
    per_s += (permission & S_IXOTH) ? 'x' : '-';
    return per_s;
}

QString FormatFileSize(uint64_t size) {
    if(size < 1024) {
        return QString::number(size) + 'B';
    } else if(size < 1024 * 1024) {
        return QString::number(size / 1024.0, 'f', 2) + "KB";
    } else if(size < 1024 * 1024 * 1024) {
        return QString::number(size / (1024.0 * 1024), 'f', 2) + "MB";
    } else {
        return QString::number(size / (1024.0 * 1024 * 1024), 'f', 2) + "GB";
    }
}
