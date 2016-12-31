/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2016, Peter Hatina <phatina@gmail.com>
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License as
*   published by the Free Software Foundation; either version 2 of
*   the License, or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program. If not, see <http://www.gnu.org/licenses/>.
* ***** END LICENSE BLOCK ***** */

// Own
#include "simple-mtpfs-driver-base.h"
#include "simple-mtpfs-util.h"

DriverBase::DriverBase()
    : m_device()
{
}

DriverBase::~DriverBase()
{
}

int DriverBase::getattr(const char *path, struct stat *buf)
{
    memset(buf, 0, sizeof(struct stat));
    struct fuse_context *fc = fuse_get_context();
    buf->st_uid = fc->uid;
    buf->st_gid = fc->gid;
    if (path == std::string("/")) {
        buf->st_mode = S_IFDIR | 0775;
        buf->st_nlink = 2;
        return 0;
    } else {
        std::string tmp_path(smtpfs_dirname(path));
        std::string tmp_file(smtpfs_basename(path));
        const TypeDir *content = m_device.dirFetchContent(tmp_path);
        if (!content) {
            return -ENOENT;
        }

        if (content->dir(tmp_file)) {
            const TypeDir *dir = content->dir(tmp_file);
            buf->st_ino = dir->id();
            buf->st_mode = S_IFDIR | 0775;
            buf->st_nlink = 2;
            buf->st_mtime = dir->modificationDate();
        } else if (content->file(tmp_file)) {
            const TypeFile *file = content->file(tmp_file);
            buf->st_ino = file->id();
            buf->st_size = file->size();
            buf->st_blocks = (file->size() / 512) + (file->size() % 512 > 0 ? 1 : 0);
            buf->st_nlink = 1;
            buf->st_mode = S_IFREG | 0644;
            buf->st_mtime = file->modificationDate();
            buf->st_ctime = buf->st_mtime;
            buf->st_atime = buf->st_mtime;
        } else {
            return -ENOENT;
        }
    }

    return 0;
}

int DriverBase::mkdir(const char *path, mode_t mode)
{
    return m_device.dirCreateNew(std::string(path));
}

int DriverBase::unlink(const char *path)
{
    return m_device.fileRemove(std::string(path));
}

int DriverBase::rmdir(const char *path)
{
    return m_device.dirRemove(std::string(path));
}

int DriverBase::chmod(const char *path, mode_t mode)
{
    return 0;
}

int DriverBase::chown(const char *path, uid_t uid, gid_t gid)
{
    return 0;
}

int DriverBase::utime(const char *path, struct utimbuf *ubuf)
{
    std::string tmp_basename(smtpfs_basename(std::string(path)));
    std::string tmp_dirname(smtpfs_dirname(std::string(path)));

    const TypeDir *parent = m_device.dirFetchContent(tmp_dirname);
    if (!parent)
        return -ENOENT;

    const TypeFile *file = parent->file(tmp_basename);
    if (!file)
        return -ENOENT;

    const_cast<TypeFile*>(file)->setModificationDate(ubuf->modtime);

    return 0;
}

int DriverBase::statfs(const char *path, struct statvfs *stat_info)
{
    uint64_t bs = 1024;
    // XXX: linux coreutils still use bsize member to calculate free space
    stat_info->f_bsize = static_cast<unsigned long>(bs);
    stat_info->f_frsize = static_cast<unsigned long>(bs);
    stat_info->f_blocks = m_device.storageTotalSize() / bs;
    stat_info->f_bavail = m_device.storageFreeSize() / bs;
    stat_info->f_bfree = stat_info->f_bavail;
    return 0;
}

int DriverBase::flush(const char *path, struct fuse_file_info *file_info)
{
    return 0;
}

int DriverBase::opendir(const char *path, struct fuse_file_info *file_info)
{
    const TypeDir *content = m_device.dirFetchContent(std::string(path));
    if (!content)
        return -ENOENT;

    return 0;
}

int DriverBase::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *file_info)
{
    const TypeDir *content = m_device.dirFetchContent(std::string(path));
    if (!content)
        return -ENOENT;

    const std::set<TypeDir> dirs = content->dirs();
    const std::set<TypeFile> files = content->files();

    for (const TypeDir &d : dirs) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = d.id();
        st.st_mode = S_IFDIR | 0775;
        filler(buf, d.name().c_str(), &st, 0);
    }

    for (const TypeFile &f : files) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = f.id();
        st.st_mode = S_IFREG | 0644;
        filler(buf, f.name().c_str(), &st, 0);
    }

    return 0;
}

int DriverBase::releasedir(const char *path, struct fuse_file_info *file_info)
{
    return 0;
}

int DriverBase::fsyncdir(const char *path, int datasync,
    struct fuse_file_info *file_info)
{
    return 0;
}
