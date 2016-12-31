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

#ifndef SMTPFS_DRIVERBASE_H
#define SMTPFS_DRIVERBASE_H

extern "C" {
#  include <fuse/fuse.h>
}
#include "simple-mtpfs-mtp-device.h"

class DriverBase
{
public:
    DriverBase();
    virtual ~DriverBase();

    virtual int getattr(const char *path, struct stat *buf);
    virtual int mknod(const char *path, mode_t mode, dev_t dev);
    virtual int mkdir(const char *path, mode_t mode);
    virtual int unlink(const char *path);

    virtual int rmdir(const char *path);
    virtual int rename(const char *path, const char *newpath);
    virtual int chmod(const char *path, mode_t mode);
    virtual int chown(const char *path, uid_t uid, gid_t gid);
    virtual int truncate(const char *path, off_t new_size);
    virtual int utime(const char *path, struct utimbuf *ubuf);
    virtual int open(const char *path, struct fuse_file_info *file_info);
    virtual int read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *file_info);
    virtual int write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *file_info);
    virtual int statfs(const char *path, struct statvfs *stat_info);
    virtual int flush(const char *path, struct fuse_file_info *file_info);
    virtual int release(const char *path, struct fuse_file_info *file_info);
    virtual int fsync(const char *path, int datasync, struct fuse_file_info *fi);
    virtual int opendir(const char *path, struct fuse_file_info *file_info);
    virtual int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *file_info);
    virtual int releasedir(const char *path, struct fuse_file_info *file_info);
    virtual int fsyncdir(const char *path, int datasync, struct fuse_file_info *file_info);
    virtual int ftruncate(const char *path, off_t offset, struct fuse_file_info *file_info);
    virtual void* init(struct fuse_conn_info *conn);
    virtual int create(const char *path, mode_t mode, fuse_file_info *file_info);

private:
    static bool removeDir(const std::string &dirname);

    MTPDevice m_device;
};

#endif // SMTPFS_DRIVERBASE_H
