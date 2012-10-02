/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2012, Peter Hatina <phatina@gmail.com>
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

#include <config.h>
#include <iostream>
extern "C" {
#  include <dirent.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <fuse_opt.h>
#  include <libgen.h>
#  include <unistd.h>
#  include <stddef.h>
#  include <stdlib.h>
#  include <sys/stat.h>
#  include <sys/types.h>
}
#include "simple-mtpfs-fuse.h"
#include "simple-mtpfs-log.h"

std::string smtpfs_dirname(const std::string &path)
{
    char *str = strdup(path.c_str());
    std::string result(dirname(str));
    free(static_cast<void*>(str));
    return result;
}

std::string smtpfs_basename(const std::string &path)
{
    char *str = strdup(path.c_str());
    std::string result(basename(str));
    free(static_cast<void*>(str));
    return result;
}

int wrap_getattr(const char *path, struct stat *statbuf)
{
    return SMTPFileSystem::instance()->getattr(path, statbuf);
}

int wrap_mknod(const char *path, mode_t mode, dev_t dev)
{
    return SMTPFileSystem::instance()->mknod(path, mode, dev);
}

int wrap_mkdir(const char *path, mode_t mode)
{
    return SMTPFileSystem::instance()->mkdir(path, mode);
}

int wrap_unlink(const char *path)
{
    return SMTPFileSystem::instance()->unlink(path);
}

int wrap_rmdir(const char *path)
{
    return SMTPFileSystem::instance()->rmdir(path);
}

int wrap_rename(const char *path, const char *newpath)
{
    return SMTPFileSystem::instance()->rename(path, newpath);
}

int wrap_chmod(const char *path, mode_t mode)
{
    return SMTPFileSystem::instance()->chmod(path, mode);
}

int wrap_chown(const char *path, uid_t uid, gid_t gid)
{
    return SMTPFileSystem::instance()->chown(path, uid, gid);
}

int wrap_truncate(const char *path, off_t new_size)
{
    return SMTPFileSystem::instance()->truncate(path, new_size);
}

int wrap_utime(const char *path, struct utimbuf *ubuf)
{
    return SMTPFileSystem::instance()->utime(path, ubuf);
}

int wrap_open(const char *path, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->open(path, file_info);
}

int wrap_read(const char *path, char *buf, size_t size, off_t offset,
    struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->read(path, buf, size, offset, file_info);
}

int wrap_write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->write(path, buf, size, offset, file_info);
}

int wrap_statfs(const char *path, struct statvfs *stat_info)
{
    return SMTPFileSystem::instance()->statfs(path, stat_info);
}

int wrap_flush(const char *path, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->flush(path, file_info);
}

int wrap_release(const char *path, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->release(path, file_info);
}

int wrap_fsync(const char *path, int datasync, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->fsync(path, datasync, file_info);
}

int wrap_opendir(const char *path, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->opendir(path, file_info);
}

int wrap_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->readdir(path, buf, filler,
        offset, file_info);
}

int wrap_releasedir(const char *path, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->releasedir(path, file_info);
}

int wrap_fsyncdir(const char *path, int datasync, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->fsyncdir(path, datasync, file_info);
}

void* wrap_init(struct fuse_conn_info *conn)
{
    return SMTPFileSystem::instance()->init(conn);
}

int wrap_create(const char *path, mode_t mode, fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->create(path, mode, file_info);
}

int wrap_ftruncate(const char *path, off_t offset, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->ftruncate(path, offset, file_info);
}

int wrap_fgetattr(const char *path, struct stat *buf, struct fuse_file_info *file_info)
{
    return SMTPFileSystem::instance()->fgetattr(path, buf, file_info);
}

// -----------------------------------------------------------------------------

std::unique_ptr<SMTPFileSystem> SMTPFileSystem::s_instance;

SMTPFileSystem *SMTPFileSystem::instance()
{
    if (!s_instance.get())
        s_instance.reset(new SMTPFileSystem());
    return s_instance.get();
}

SMTPFileSystem::SMTPFileSystem():
    m_args(),
    m_tmp_files_pool(),
    m_options(),
    m_device()
{
    m_fuse_operations.getattr = wrap_getattr;
    m_fuse_operations.readlink = nullptr;
    m_fuse_operations.getdir = nullptr;
    m_fuse_operations.mknod = wrap_mknod;
    m_fuse_operations.mkdir = wrap_mkdir;
    m_fuse_operations.unlink = wrap_unlink;
    m_fuse_operations.rmdir = wrap_rmdir;
    m_fuse_operations.symlink = nullptr;
    m_fuse_operations.rename = wrap_rename;
    m_fuse_operations.link = nullptr;
    m_fuse_operations.chmod = wrap_chmod;
    m_fuse_operations.chown = wrap_chown;
    m_fuse_operations.truncate = wrap_truncate;
    m_fuse_operations.utime = wrap_utime;
    m_fuse_operations.open = wrap_open;
    m_fuse_operations.read = wrap_read;
    m_fuse_operations.write = wrap_write;
    m_fuse_operations.statfs = wrap_statfs;
    m_fuse_operations.flush = wrap_flush;
    m_fuse_operations.release = wrap_release;
    m_fuse_operations.fsync = wrap_fsync;
    m_fuse_operations.setxattr = nullptr;
    m_fuse_operations.getxattr = nullptr;
    m_fuse_operations.listxattr = nullptr;
    m_fuse_operations.removexattr = nullptr;
    m_fuse_operations.opendir = wrap_opendir;
    m_fuse_operations.readdir = wrap_readdir;
    m_fuse_operations.releasedir = wrap_releasedir;
    m_fuse_operations.fsyncdir = wrap_fsyncdir;
    m_fuse_operations.init = wrap_init;
    m_fuse_operations.destroy = nullptr;
    m_fuse_operations.access = nullptr;
    m_fuse_operations.create = wrap_create;
    m_fuse_operations.ftruncate = wrap_ftruncate;
    m_fuse_operations.fgetattr = wrap_fgetattr;
}

SMTPFileSystem::~SMTPFileSystem()
{
    fuse_opt_free_args(&m_args);
    removeTmpDir();
}

bool SMTPFileSystem::parseOptions(int argc, char **argv)
{
    #define SMTPFS_OPT_KEY(t, p, v) { t, offsetof(SMTPFileSystemOptions, p), v }

    static struct fuse_opt smtpfs_opts[] = {
        SMTPFS_OPT_KEY("enable-move", m_enable_move, 1),
        SMTPFS_OPT_KEY("tmp-dir=%s", m_tmp_dir, 0),
        SMTPFS_OPT_KEY("--device %i", m_device, 0),
        SMTPFS_OPT_KEY("-l", m_list_devices, 1),
        SMTPFS_OPT_KEY("--list-devices", m_list_devices, 1),
        SMTPFS_OPT_KEY("-v", m_verbose, 1),
        SMTPFS_OPT_KEY("--verbose", m_verbose, 1),
        SMTPFS_OPT_KEY("-V", m_version, 1),
        SMTPFS_OPT_KEY("--version", m_version, 1),
        SMTPFS_OPT_KEY("-h", m_help, 1),
        SMTPFS_OPT_KEY("--help", m_help, 1),
        FUSE_OPT_END
    };

    if (argc < 2) {
        m_options.m_good = false;
        return false;
    }

    fuse_opt_free_args(&m_args);
    m_args = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&m_args, &m_options, smtpfs_opts, nullptr) == -1) {
        m_options.m_good = false;
        return false;
    }

    fuse_opt_add_arg(&m_args, "-s");

    if (m_options.m_version || m_options.m_help || m_options.m_list_devices) {
        m_options.m_good = true;
        return true;
    }

    if (--m_options.m_device < 0) {
        m_options.m_good = false;
        return false;
    }

    if (m_options.m_tmp_dir)
        removeTmpDir();
    m_options.m_tmp_dir = expandTmpDir(m_options.m_tmp_dir);
    if (!m_options.m_tmp_dir) {
        m_options.m_good = false;
        return false;
    }

    m_tmp_files_pool.setTmpDir(m_options.m_tmp_dir);
    ::mkdir(static_cast<const char*>(m_options.m_tmp_dir), 0700);

    if (m_options.m_verbose) {
        Logger::setGlobalVerbose();
        fuse_opt_add_arg(&m_args, "-f");
    }

    m_options.m_good = true;
    return true;
}

void SMTPFileSystem::printHelp() const
{
    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    struct fuse_operations tmp_operations;
    memset(&tmp_operations, 0, sizeof(tmp_operations));
    std::cout << "usage: " << m_args.argv[0] << " mountpoint [options]\n\n"
        << "general options:\n"
        << "    -o opt,[opt...]        mount options\n"
        << "    -h   --help            print help\n"
        << "    -V   --version         print version\n\n"
        << "simple-mtpfs options:\n"
        << "    -l   --list-devices    print available devices\n"
        << "         --device          select a device number to mount\n"
        << "    -o enable-move         enable the move operations\n"
        << "    -o tmp-dir=PATH        define a temporary directory for data storage\n\n";
    fuse_opt_add_arg(&args, m_args.argv[0]);
    fuse_opt_add_arg(&args, "-ho");
    fuse_main(args.argc, args.argv, &tmp_operations, nullptr);
    fuse_opt_free_args(&args);
    std::cout << "\nReport bugs to <" << PACKAGE_BUGREPORT << ">.\n";
}

void SMTPFileSystem::printVersion() const
{
    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    struct fuse_operations tmp_operations;
    memset(&tmp_operations, 0, sizeof(tmp_operations));
    fuse_opt_add_arg(&args, m_args.argv[0]);
    fuse_opt_add_arg(&args, "--version");
    std::cout << "simple-mtpfs version " << VERSION << "\n";
    fuse_main(args.argc, args.argv, &tmp_operations, nullptr);
    fuse_opt_free_args(&args);
}

bool SMTPFileSystem::exec()
{
    if (!m_options.m_good)
        return false;

    if (m_options.m_list_devices) {
        m_device.listDevices();
        return true;
    }

    if (m_options.m_version || m_options.m_help)
        return true;

    if (!m_device.connect(m_options.m_device))
        return false;
    m_device.enableMove(m_options.m_enable_move);
    if (fuse_main(m_args.argc, m_args.argv, &m_fuse_operations, nullptr) > 0)
        return false;
    m_device.disconnect();
    return true;
}

void* SMTPFileSystem::init(struct fuse_conn_info *conn)
{
    return nullptr;
}

int SMTPFileSystem::getattr(const char *path, struct stat *buf)
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
        if (!content)
            return -ENOENT;
        if (content->dir(tmp_file)) {
            const TypeDir *dir = content->dir(tmp_file);
            buf->st_ino = dir->id();
            buf->st_mode = S_IFDIR | 0775;
            buf->st_nlink = 2;
        } else if (content->file(tmp_file)) {
            const TypeFile *file = content->file(tmp_file);
            buf->st_ino = file->id();
            buf->st_size = file->size();
            buf->st_blocks = (file->size() / 512) + (file->size() % 512 > 0 ? 1 : 0);
            buf->st_nlink = 1;
            buf->st_mode = S_IFREG | 0664;
            buf->st_mtime = file->modificationDate();
            buf->st_ctime = buf->st_mtime;
            buf->st_atime = buf->st_mtime;
        } else {
            return -ENOENT;
        }
    }
    return 0;
}

int SMTPFileSystem::mknod(const char *path, mode_t mode, dev_t dev)
{
    if (!S_ISREG(mode))
        return -EINVAL;
    std::string tmp_path = m_tmp_files_pool.makeTmpPath(std::string(path));
    int rval = ::open(tmp_path.c_str(), O_CREAT | O_WRONLY, mode);
    if (rval < 0)
        return -errno;
    rval = close(rval);
    if (rval < 0)
        return -errno;
    m_device.filePush(tmp_path, std::string(path));
    ::unlink(tmp_path.c_str());
    return rval;
}

int SMTPFileSystem::mkdir(const char *path, mode_t mode)
{
    return m_device.dirCreateNew(std::string(path));
}

int SMTPFileSystem::unlink(const char *path)
{
    return m_device.fileRemove(std::string(path));
}

int SMTPFileSystem::rmdir(const char *path)
{
    return m_device.dirRemove(std::string(path));
}

int SMTPFileSystem::rename(const char *path, const char *newpath)
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
        return rval;
    rval = m_device.filePush(tmp_file, std::string(newpath));
    if (rval != 0)
        return rval;
    rval = m_device.fileRemove(std::string(path));
    if (rval != 0)
        return rval;
    return 0;
}

int SMTPFileSystem::chmod(const char *path, mode_t mode)
{
    return 0;
}

int SMTPFileSystem::chown(const char *path, uid_t uid, gid_t gid)
{
    return 0;
}

int SMTPFileSystem::truncate(const char *path, off_t new_size)
{
    const std::string tmp_path = m_tmp_files_pool.makeTmpPath(std::string(path));
    int rval = m_device.filePull(std::string(path), tmp_path);
    if (rval != 0)
        return rval;
    rval = ::truncate(tmp_path.c_str(), new_size);
    if (rval != 0)
        return -errno;
    rval = m_device.fileRemove(std::string(path));
    if (rval != 0)
        return rval;
    return m_device.filePush(tmp_path, std::string(path));
}

int SMTPFileSystem::utime(const char *path, struct utimbuf *ubuf)
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

int SMTPFileSystem::create(const char *path, mode_t mode, fuse_file_info *file_info)
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

int SMTPFileSystem::open(const char *path, struct fuse_file_info *file_info)
{
    if (file_info->flags & O_WRONLY)
        file_info->flags |= O_TRUNC;

    std::string tmp_file = m_tmp_files_pool.makeTmpPath(std::string(path));
    int rval = m_device.filePull(std::string(path), tmp_file);
    if (rval != 0)
        return rval;
    int fd = ::open(tmp_file.c_str(), file_info->flags);
    if (fd < 0)
        return -errno;
    file_info->fh = fd;
    m_tmp_files_pool.addFile(TypeTmpFile(std::string(path), tmp_file, fd));
    return 0;
}

int SMTPFileSystem::read(const char *path, char *buf, size_t size,
    off_t offset, struct fuse_file_info *file_info)
{
    int rval = ::pread(file_info->fh, buf, size, offset);
    if (rval < 0)
        return -errno;
    return rval;
}

int SMTPFileSystem::write(const char *path, const char *buf, size_t size,
    off_t offset, struct fuse_file_info *file_info)
{
    int rval = ::pwrite(file_info->fh, buf, size, offset);
    if (rval < 0)
        return -errno;
    const TypeTmpFile *tmp_file = m_tmp_files_pool.getFile(static_cast<int>(file_info->fh));
    if (!tmp_file)
        return -EINVAL;
    const_cast<TypeTmpFile*>(tmp_file)->setModified();
    return rval;
}

int SMTPFileSystem::release(const char *path, struct fuse_file_info *file_info)
{
    int rval = ::close(file_info->fh);
    if (rval < 0)
        return rval;

    if (std::string(path) == std::string("-"))
        return 0;

    const TypeTmpFile *tmp_file = m_tmp_files_pool.getFile(static_cast<int>(file_info->fh));
    const bool modif = tmp_file->isModified();
    const std::string tmp_path = tmp_file->pathTmp();
    m_tmp_files_pool.removeFile(tmp_file->fileDescriptor());
    if (modif) {
        int push_rval = m_device.filePush(tmp_path, std::string(path));
        if (push_rval != 0)
            return push_rval;
        ::unlink(tmp_path.c_str());
    }
    return rval;
}

int SMTPFileSystem::statfs(const char *path, struct statvfs *stat_info)
{
    uint64_t bs = 1024;
    stat_info->f_bsize = static_cast<unsigned long>(bs);
    stat_info->f_blocks = m_device.storageTotalSize() / bs;
    stat_info->f_bavail = m_device.storageFreeSize() / bs;
    stat_info->f_bfree = stat_info->f_bavail;
    return 0;
}

int SMTPFileSystem::flush(const char *path, struct fuse_file_info *file_info)
{
    return 0;
}

int SMTPFileSystem::fsync(const char *path, int datasync,
    struct fuse_file_info *fi)
{
    return datasync ? ::fdatasync(fi->fh) : ::fsync(fi->fh);
}

int SMTPFileSystem::opendir(const char *path, struct fuse_file_info *file_info)
{
    const TypeDir *content = m_device.dirFetchContent(std::string(path));
    if (!content)
        return -ENOENT;
    return 0;
}

int SMTPFileSystem::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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
        st.st_mode = S_IFREG | 0664;
        filler(buf, f.name().c_str(), &st, 0);
    }
    return 0;
}

int SMTPFileSystem::releasedir(const char *path, struct fuse_file_info *file_info)
{
    return 0;
}

int SMTPFileSystem::fsyncdir(const char *path, int datasync,
    struct fuse_file_info *file_info)
{
    return 0;
}

int SMTPFileSystem::ftruncate(const char *path, off_t offset,
    struct fuse_file_info *file_info)
{
    const TypeTmpFile *tmp_file = m_tmp_files_pool.getFile(file_info->fh);
    if (::ftruncate(file_info->fh, offset) != 0)
        return -errno;
    const_cast<TypeTmpFile*>(tmp_file)->setModified();
    return 0;
}

int SMTPFileSystem::fgetattr(const char *path, struct stat *buf, fuse_file_info *file_info)
{
    if (::fstat(file_info->fh, buf) != 0)
        return -errno;
    return 0;
}

char *SMTPFileSystem::expandTmpDir(char *tmp)
{
    if (!tmp) {
        tmp = strdup("/tmp/simple-mtpfs-XXXXXX");
    } else {
        std::string tmp_dir(tmp);
        auto it = tmp_dir.find('~');
        for (; it != std::string::npos; it = tmp_dir.find('~'))
            tmp_dir.replace(it, 1, getenv("HOME"));
        tmp_dir += "XXXXXX";
        free(static_cast<void*>(tmp));
        tmp = strdup(tmp_dir.c_str());
        if (!tmp)
            return nullptr;
    }
    return mktemp(tmp);
}

bool SMTPFileSystem::removeDir(const std::string &dirname)
{
    DIR *dir;
    struct dirent *entry;
    std::string path;

    dir = ::opendir(dirname.c_str());
    if (dir == nullptr)
        return false;

    while ((entry = ::readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            path = dirname + "/" + entry->d_name;
            if (entry->d_type == DT_DIR)
                return removeDir(path);
            ::unlink(path.c_str());
        }
    }
    ::closedir(dir);
    ::remove(dirname.c_str());
    return true;
}

bool SMTPFileSystem::removeTmpDir()
{
    if (m_options.m_tmp_dir)
        return removeDir(std::string(m_options.m_tmp_dir));
    return false;
}
