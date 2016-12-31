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

#ifndef SMTPFS_TMPDIR_DRIVER_H
#define SMTPFS_TMPDIR_DRIVER_H

// STL
#include <memory>

// Own
#include "simple-mtpfs-driver-base.h"
#include "simple-mtpfs-mtp-device.h"
#include "simple-mtpfs-tmp-files-pool.h"

class TmpDirDriver: public DriverBase
{
public:
    TmpDirDriver();
    virtual ~TmpDirDriver();

    virtual int mknod(const char *path, mode_t mode, dev_t dev) override;
    virtual int rename(const char *path, const char *newpath) override;
    virtual int truncate(const char *path, off_t new_size) override;
    virtual int create(const char *path, mode_t mode, fuse_file_info *file_info) override;
    virtual int open(const char *path, struct fuse_file_info *file_info) override;
    virtual int read(const char *path, char *buf, size_t size,
        off_t offset, struct fuse_file_info *file_info) override;
    virtual int write(const char *path, const char *buf, size_t size,
        off_t offset, struct fuse_file_info *file_info) override;
    virtual int release(const char *path, struct fuse_file_info *file_info) override;
    virtual int fsync(const char *path, int datasync, struct fuse_file_info *fi) override;

private:
    bool createTmpDir();
    bool removeTmpDir();

    TmpFilesPool m_tmp_files_pool;
};

std::unique_ptr<DriverBase> makeTmpDirDriver();

#endif // SMTPFS_TMPDIR_DRIVER_H
