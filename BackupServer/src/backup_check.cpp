#include "backup_check.h"
#include "backup_impl.h"
namespace backup {
std::pair<Status, std::vector<CheckResult>>
backup::BackupCheck::CheckBackupFile(
    const std::string &backup_path,
    const std::string &password)
{
    std::vector<CheckResult> result;
    BackUp *back = new BackUpImpl();
    auto [s, node] = back->GetFileList(backup_path, password);
    if (s.code != OK) { return {s, result}; }
    // 递归检查node
    for (auto &pair : node->children_) {
        auto &nd = pair.second;
        CheckBackupFile(result, nd);
    }
    return {{OK, ""}, result};
}

void BackupCheck::CheckBackupFile(
    std::vector<CheckResult> &result,
    std::shared_ptr<FileNode> root)
{
    auto &meta = root->meta_;
    CheckResult rt;
    // 1. Check自身
    CheckFileMeta(rt, meta);
    bool isDelete = rt.type == CheckType::DELETE;
    if (rt.type != CheckType::OK) result.emplace_back(std::move(rt));
    // 2. CheckChild
    // 注意：如果文件夹被删除了，那么就没必要继续下去了
    if (!meta.is_directory || isDelete) { return; }
    // check meta下面有的文件
    for (auto &pair : root->children_) {
        auto &node = pair.second;
        CheckBackupFile(result, node);
    }

    // check meta下没有的文件
    if (!meta.is_partly_check) {
        auto files = GetFilesFromDir(meta.origin_path);
        for (auto &path : files) {
            if (!root->children_.contains(path.FileName())) {
                result.emplace_back(MakeResult(
                    CheckType::ADD,
                    path.GetFileType(),
                    (Path(meta.origin_path) / path).ToString(),
                    meta.pack_path));
            }
        }
    }
}

CheckResult BackupCheck::MakeResult(
    CheckType type,
    FileType filetype,
    const std::string &origin_path,
    const std::string &backup_path,
    const std::string &details)
{
    switch (type) {
    case CheckType::ADD:
        return {
            type,
            origin_path,
            backup_path,
            "源路径下新增" + GetFileTypeTag(filetype)};
    case CheckType::DELETE:
        return {
            type,
            origin_path,
            backup_path,
            "源路径下" + GetFileTypeTag(filetype) + "被删除"};
    case CheckType::META_UPDATE:
        return {
            type,
            origin_path,
            backup_path,
            "源文件元信息发生了修改：" + details};
    case CheckType::DATA_UPDATE:
        return {type, origin_path, backup_path, "源文件发生了修改"};
    case CheckType::OTHER:
        return {type, origin_path, backup_path, details};
    default:
        throw Status{ERROR, "未知错误"};
    }
}

void BackupCheck::CheckFileMeta(CheckResult &result, const FileMetadata &meta)
{
    std::string origin_path = meta.origin_path;
    ErrorCode code = Access(origin_path, READ);
    if (code == NOT_EXIST) {
        result = MakeResult(
            CheckType::DELETE,
            static_cast<FileType>(meta.type),
            origin_path,
            meta.pack_path);
        return;
    }
    if (code == NO_PERMISSION) {
        result = MakeResult(
            CheckType::OTHER,
            static_cast<FileType>(meta.type),
            origin_path,
            meta.pack_path,
            "没有对源文件的访问权限");
        return;
    }

    // 检查元信息
    FileMetadata meta_origin;
    meta_origin.SetFromPath(origin_path);
    std::string details;
    CheckType type = CheckType::OK;
    // 时间
    if (meta_origin.access_time != meta.access_time) {
        type = CheckType::META_UPDATE;
        details = "访问时间变更：" + FormatTime(meta_origin.access_time) +
                  " : " + FormatTime(meta.access_time) + ";";
    }
    if (meta.mod_time != meta_origin.mod_time) {
        type = CheckType::META_UPDATE;
        if (!details.empty()) details += '\n';
        details += "修改时间变更：" + FormatTime(meta_origin.mod_time) + " : " +
                   FormatTime(meta.mod_time) + ";";
    }
    // 用户
    if (meta.uid != meta_origin.uid) {
        type = CheckType::META_UPDATE;
        if (!details.empty()) details += '\n';
        details += "所属用户变更：" + UidToString(meta_origin.uid) + " : " +
                   UidToString(meta.uid);
    }
    if (meta.gid != meta_origin.gid) {
        type = CheckType::META_UPDATE;
        if (!details.empty()) details += '\n';
        details += "所属用户组变更：" + GidToString(meta_origin.gid) + " : " +
                   GidToString(meta.gid);
    }
    // 权限
    if (meta.permissions != meta_origin.permissions) {
        type = CheckType::META_UPDATE;
        if (!details.empty()) details += '\n';
        details +=
            "权限变更：" +
            FormatPermission(
                meta_origin.permissions,
                static_cast<FileType>(meta_origin.type)) +
            " : " +
            FormatPermission(
                meta_origin.permissions, static_cast<FileType>(meta.type));
    }
    // 内容
    if (meta.type == static_cast<int>(FileType::REG))
        if (::memcmp(meta.hash, meta_origin.hash, SHA256_SIZE) != 0) {
            if (type != CheckType::OK) {
                type = CheckType::UPDATE;
            } else {
                type = CheckType::DATA_UPDATE;
            }
            if (!details.empty()) details += '\n';
            details += "文件内容发生变更";
        }
    result.type = type;
    if (type != CheckType::OK) {
        result.origin_path = origin_path;
        result.backup_path = meta.pack_path;
        result.detail = details;
    }
}
std::string GetCheckTypeTag(CheckType type)
{
    switch (type) {
    case CheckType::ADD:
        return "新增文件";
    case CheckType::DELETE:
        return "文件删除";
    case CheckType::META_UPDATE:
        return "元信息被修改";
    case CheckType::DATA_UPDATE:
        return "文件内容修改";
    case CheckType::UPDATE:
        return "元信息和内容被修改";
    case CheckType::OTHER:
        return "其他";
    default:
        break;
    }
    return std::string();
}
} // namespace backup
