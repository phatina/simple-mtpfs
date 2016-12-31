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
#include "simple-mtpfs-tmpdir-driver.h"
#include "simple-mtpfs-util.h"

TmpDirDriver::TmpDirDriver()
    : DriverBase{}
    , m_tmp_files_pool{}
{
}

TmpDirDriver::~TmpDirDriver()
{
}

int TmpDirDriver::mknod(const char *path, mode_t mode, dev_t dev)
{
    if (!S_ISREG(mode))
        return -EINVAL;

    std::string tmp_path = m_tmp_files_pool.makeTmpPath(std::string(path));
    int rval = ::open(tmp_path.c_str(), O_CREAT | O_WRONLY, mode);
    if (rval < 0)
        return -errno;

    rval = ::close(rval);
    if (rval < 0)
        return -errno;

    m_device.filePush(tmp_path, std::string(path));
    ::unlink(tmp_path.c_str());
    return 0;
}

int TmpDirDriver::rename(const char *path, const char *newpath)
{
    const std::string tmp_old_dirname(smtpfs_dirname(std::string(path)));
    const std::string tmp_new_dirname(smtpfs_dirname(std::string(newpath)));
    if (tmp_old_dirname == tmp_new_dirname)
        return m_device.rename(std::string(path), std::string(newpath));

    if (!m_options.m_enable_move)
        return -EPERM;

    const std::string tmp_file = m_tmp_files_pool.makeTmpPath(std::string(newpath));
    int rval = m_device.filePull(std::string(path), tmp_file);
    if (rval != 0)
        return -rval;

    rval = m_device.filePush(tmp_file, std::string(newpath));
    if (rval != 0)
        return -rval;

    rval = m_device.fileRemove(std::string(path));
    if (rval != 0)
        return -rval;

    return 0;
}

int TmpDirDriver::truncate(const char *path, off_t new_size)
{
    const std::string tmp_path = m_tmp_files_pool.makeTmpPath(std::string(path));
    int rval = m_device.filePull(std::string(path), tmp_path);
    if (rval != 0) {
        ::unlink(tmp_path.c_str());
        return -rval;
    }

    rval = ::truncate(tmp_path.c_str(), new_size);
    if (rval != 0) {
        int errno_tmp = errno;
        ::unlink(tmp_path.c_str());
        return -errno_tmp;
    }

    rval = m_device.fileRemove(std::string(path));
    if (rval != 0) {
        ::unlink(tmp_path.c_str());
        return -rval;
    }

    rval = m_device.filePush(tmp_path, std::string(path));
    ::unlink(tmp_path.c_str());

    if (rval != 0)
        return -rval;

    return 0;
}

int TmpDirDriver::create(const char *path, mode_t mode, fuse_file_info *file_info)
{
    const std::string tmp_path = m_tmp_files_pool.makeTmpPath(std::string(path));

    int rval = ::creat(tmp_path.c_str(), mode);
    if (rval < 0)
        return -errno;

    file_info->fh = rval;
    m_tmp_files_pool.addFile(TypeTmpFile(std::string(path), tmp_path, rval, true));
    m_device.filePush(tmp_path, std::string(path));

    return 0;
}

int TmpDirDriver::open(const char *path, struct fuse_file_info *file_info)
{
    if (file_info->flags & O_WRONLY)
        file_info->flags |= O_TRUNC;

    const std::string std_path(path);

    TypeTmpFile *tmp_file = const_cast<TypeTmpFile*>(
        m_tmp_files_pool.getFile(std_path));

    std::string tmp_path;
    if (tmp_file) {
        tmp_path = tmp_file->pathTmp();
    } else {
        tmp_path = m_tmp_files_pool.makeTmpPath(std_path);

        int rval = m_device.filePull(std_path, tmp_path);
        if (rval != 0)
            return -rval;
    }

    int fd = ::open(tmp_path.c_str(), file_info->flags);
    if (fd < 0) {
        ::unlink(tmp_path.c_str());
        return -errno;
    }

    file_info->fh = fd;

    if (tmp_file)
        tmp_file->addFileDescriptor(fd);
    else
        m_tmp_files_pool.addFile(TypeTmpFile(std_path, tmp_path, fd));

    return 0;
}

int TmpDirDriver::read(const char *path, char *buf, size_t size,
    off_t offset, struct fuse_file_info *file_info)
{
    int rval = ::pread(file_info->fh, buf, size, offset);
    if (rval < 0)
        return -errno;
    return rval;
}

int TmpDirDriver::write(const char *path, const char *buf, size_t size,
    off_t offset, struct fuse_file_info *file_info)
{
    const TypeTmpFile *tmp_file = m_tmp_files_pool.getFile(std::string(path));
    if (!tmp_file)
        return -EINVAL;

    int rval = ::pwrite(file_info->fh, buf, size, offset);
    if (rval < 0)
        return -errno;

    const_cast<TypeTmpFile*>(tmp_file)->setModified();
    return rval;
}

int TmpDirDriver::release(const char *path, struct fuse_file_info *file_info)
{
    int rval = ::close(file_info->fh);
    if (rval < 0)
        return -errno;

    const std::string std_path(path);
    if (std_path == std::string("-"))
        return 0;

    TypeTmpFile *tmp_file = const_cast<TypeTmpFile*>(
        m_tmp_files_pool.getFile(std_path));
    tmp_file->removeFileDescriptor(file_info->fh);
    if (tmp_file->refcnt() != 0)
        return 0;

    const bool modif = tmp_file->isModified();
    const std::string tmp_path = tmp_file->pathTmp();
    m_tmp_files_pool.removeFile(std_path);
    if (modif) {
        rval = m_device.filePush(tmp_path, std_path);
        if (rval != 0) {
            ::unlink(tmp_path.c_str());
            return -rval;
        }
    }

    ::unlink(tmp_path.c_str());

    return 0;
}

int TmpDirDriver::fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    int rval = -1;
#ifdef HAVE_FDATASYNC
    if (datasync)
        rval = ::fdatasync(fi->fh);
    else
#else
    rval = ::fsync(fi->fh);
#endif
    if (rval != 0)
        return -errno;
    return 0;
}

std::unique_ptr<DriverBase> makeTmpDirDriver()
{
    return std::make_unique<TmpDirDriver>();
}

