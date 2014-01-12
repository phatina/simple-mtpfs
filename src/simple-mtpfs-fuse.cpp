/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2012-2013, Peter Hatina <phatina@gmail.com>
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
#  include <errno.h>
#  include <fuse/fuse_opt.h>
#  include <unistd.h>
#  include <stddef.h>
}
#include "simple-mtpfs-fuse.h"
#include "simple-mtpfs-log.h"
#include "simple-mtpfs-util.h"

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

// -----------------------------------------------------------------------------

SMTPFileSystem::SMTPFileSystemOptions::SMTPFileSystemOptions()
    : m_good(false)
    , m_help(false)
    , m_version(false)
    , m_verbose(false)
    , m_enable_move(false)
    , m_list_devices(false)
    , m_device_no(1)
#ifdef HAVE_LIBUSB1
    , m_device_file(nullptr)
#endif // HAVE_LIBUSB1
    , m_mount_point(nullptr)
{
}

SMTPFileSystem::SMTPFileSystemOptions::~SMTPFileSystemOptions()
{
#ifdef HAVE_LIBUSB1
    free(static_cast<void*>(m_device_file));
#endif // HAVE_LIBUSB1
    free(static_cast<void*>(m_mount_point));
}

// -----------------------------------------------------------------------------

int SMTPFileSystem::SMTPFileSystemOptions::opt_proc(void *data, const char *arg, int key,
    struct fuse_args *outargs)
{
    SMTPFileSystemOptions *options = static_cast<SMTPFileSystemOptions*>(data);

    if (key == FUSE_OPT_KEY_NONOPT) {
#ifdef HAVE_LIBUSB1
        if (options->m_mount_point && !options->m_device_file) {
            options->m_device_file = options->m_mount_point;
            options->m_mount_point = nullptr;
        } else if (options->m_mount_point && options->m_device_file) {
            // Unknown positional argument supplied
            return -1;
        }
#else
        if (options->m_mount_point) {
            // Unknown positional argument supplied
            return -1;
        }
#endif //HAVE_LIBUSB1

        fuse_opt_add_opt(&options->m_mount_point, arg);
        return 0;
    }
    return 1;
}

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
    m_fuse_operations.fgetattr = nullptr;
}

SMTPFileSystem::~SMTPFileSystem()
{
    fuse_opt_free_args(&m_args);
}

bool SMTPFileSystem::parseOptions(int argc, char **argv)
{
    #define SMTPFS_OPT_KEY(t, p, v) { t, offsetof(SMTPFileSystemOptions, p), v }

    static struct fuse_opt smtpfs_opts[] = {
        SMTPFS_OPT_KEY("enable-move", m_enable_move, 1),
        SMTPFS_OPT_KEY("--device %i", m_device_no, 0),
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
        logerr("Wrong usage.\n");
        m_options.m_good = false;
        return false;
    }

    fuse_opt_free_args(&m_args);
    m_args = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&m_args, &m_options, smtpfs_opts, SMTPFileSystemOptions::opt_proc) == -1) {
        m_options.m_good = false;
        return false;
    }

    if (m_options.m_version || m_options.m_help || m_options.m_list_devices) {
        m_options.m_good = true;
        return true;
    }

    if (!m_options.m_mount_point) {
        logerr("Mount point missing.\n");
        m_options.m_good = false;
        return false;
    }

    fuse_opt_add_arg(&m_args, m_options.m_mount_point);
    fuse_opt_add_arg(&m_args, "-s");

    if (m_options.m_verbose) {
        Logger::setGlobalVerbose();
        fuse_opt_add_arg(&m_args, "-f");
    }

    --m_options.m_device_no;

#ifdef HAVE_LIBUSB1
    // device file and -- device are mutually exclusive, fail if both set
    if (m_options.m_device_no && m_options.m_device_file) {
        m_options.m_good = false;
        return false;
    }
#endif // HAVE_LIBUSB1

    m_options.m_good = true;
    return true;
}

void SMTPFileSystem::printHelp() const
{
    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    struct fuse_operations tmp_operations;
    memset(&tmp_operations, 0, sizeof(tmp_operations));
    std::cerr << "usage: " << smtpfs_basename(m_args.argv[0])
#ifdef HAVE_LIBUSB1
              << " <source>"
#endif // HAVE_LIBUSB1
              << " mountpoint [options]\n\n"
        << "general options:\n"
        << "    -o opt,[opt...]        mount options\n"
        << "    -h   --help            print help\n"
        << "    -V   --version         print version\n\n"
        << "simple-mtpfs options:\n"
        << "    -v   --verbose         verbose output, implies -f\n"
        << "    -l   --list-devices    print available devices\n"
        << "         --device          select a device number to mount\n"
        << "    -o enable-move         enable the move operations\n\n";
    fuse_opt_add_arg(&args, m_args.argv[0]);
    fuse_opt_add_arg(&args, "-ho");
    fuse_main(args.argc, args.argv, &tmp_operations, nullptr);
    fuse_opt_free_args(&args);
    std::cerr << "\nReport bugs to <" << PACKAGE_BUGREPORT << ">.\n";
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

    if (m_options.m_version || m_options.m_help)
        return true;

    if (!smtpfs_check_dir(m_options.m_mount_point)) {
        logerr("Can not mount the device to '", m_options.m_mount_point, "'.\n");
        return false;
    }

    if (!m_tmp_files_pool.createTmpDir()) {
        logerr("Can not create a temporary directory.\n");
        return false;
    }

#ifdef HAVE_LIBUSB1
    if (m_options.m_device_file) {
        // Try to use device file first, if provided
        if (!m_device.connect(m_options.m_device_file))
            return false;
    } else
#endif // HAVE_LIBUSB1
    {
        // Connect to MTP device by order number, if no device file supplied
        if (!m_device.connect(m_options.m_device_no))
            return false;
    }
    m_device.enableMove(m_options.m_enable_move);
    if (fuse_main(m_args.argc, m_args.argv, &m_fuse_operations, nullptr) > 0) {
        return false;
    }
    m_device.disconnect();

    m_tmp_files_pool.removeTmpDir();

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
        if (!content) {
            return -ENOENT;
        }

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

int SMTPFileSystem::mknod(const char *path, mode_t mode, dev_t dev)
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
        return -rval;

    rval = m_device.filePush(tmp_file, std::string(newpath));
    if (rval != 0)
        return -rval;

    rval = m_device.fileRemove(std::string(path));
    if (rval != 0)
        return -rval;

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
    const TypeTmpFile *tmp_file = m_tmp_files_pool.getFile(std::string(path));
    if (!tmp_file)
        return -EINVAL;

    int rval = ::pwrite(file_info->fh, buf, size, offset);
    if (rval < 0)
        return -errno;

    const_cast<TypeTmpFile*>(tmp_file)->setModified();
    return rval;
}

int SMTPFileSystem::release(const char *path, struct fuse_file_info *file_info)
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

int SMTPFileSystem::statfs(const char *path, struct statvfs *stat_info)
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

int SMTPFileSystem::flush(const char *path, struct fuse_file_info *file_info)
{
    return 0;
}

int SMTPFileSystem::fsync(const char *path, int datasync,
    struct fuse_file_info *fi)
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
        st.st_mode = S_IFREG | 0644;
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
    const TypeTmpFile *tmp_file = m_tmp_files_pool.getFile(std::string(path));
    if (::ftruncate(file_info->fh, offset) != 0)
        return -errno;
    const_cast<TypeTmpFile*>(tmp_file)->setModified();
    return 0;
}
