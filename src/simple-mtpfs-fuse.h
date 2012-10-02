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

#ifndef SMTPFS_FUSE_H
#define SMTPFS_FUSE_H

#include <memory>
#include <string>
extern "C" {
#  include <fuse.h>
}
#include "simple-mtpfs-mtp-device.h"
#include "simple-mtpfs-tmp-files-pool.h"
#include "simple-mtpfs-type-tmp-file.h"

class SMTPFileSystem
{
private:
    struct SMTPFileSystemOptions {
    public:
        enum { KEY_VERSION,
               KEY_HELP,
               KEY_LIST_DEVICES,
               KEY_DEVICE,
               KEY_ENABLE_MOVE};

        int m_good;
        int m_help;
        int m_version;
        int m_verbose;
        int m_enable_move;
        int m_list_devices;
        int m_device;
        char *m_tmp_dir;

        SMTPFileSystemOptions():
            m_good(false),
            m_help(false),
            m_version(false),
            m_verbose(false),
            m_enable_move(false),
            m_list_devices(false),
            m_device(1),
            m_tmp_dir(NULL) {}
        ~SMTPFileSystemOptions() { free(static_cast<void*>(m_tmp_dir)); }

        static int opt_proc(void *data, const char *arg, int key,
            struct fuse_args *outargs);
    };

    SMTPFileSystem();

public:
    ~SMTPFileSystem();

    static SMTPFileSystem *instance();

    bool parseOptions(int argc, char **argv);
    void printHelp() const;
    void printVersion() const;
    bool listDevices() { return m_device.listDevices(); }

    bool exec();
    bool isGood() const { return m_options.m_good; }
    bool isHelp() const { return m_options.m_help; }
    bool isVersion() const { return m_options.m_version; }
    bool isListDevices() const { return m_options.m_list_devices; }

    int getattr(const char *path, struct stat *buf);
    int mknod(const char *path, mode_t mode, dev_t dev);
    int mkdir(const char *path, mode_t mode);
    int unlink(const char *path);
    int rmdir(const char *path);
    int rename(const char *path, const char *newpath);
    int chmod(const char *path, mode_t mode);
    int chown(const char *path, uid_t uid, gid_t gid);
    int truncate(const char *path, off_t new_size);
    int utime(const char *path, struct utimbuf *ubuf);
    int open(const char *path, struct fuse_file_info *file_info);
    int read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *file_info);
    int write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *file_info);
    int statfs(const char *path, struct statvfs *stat_info);
    int flush(const char *path, struct fuse_file_info *file_info);
    int release(const char *path, struct fuse_file_info *file_info);
    int fsync(const char *path, int datasync, struct fuse_file_info *fi);
    int opendir(const char *path, struct fuse_file_info *file_info);
    int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *file_info);
    int releasedir(const char *path, struct fuse_file_info *file_info);
    int fsyncdir(const char *path, int datasync, struct fuse_file_info *file_info);
    int ftruncate(const char *path, off_t offset, struct fuse_file_info *file_info);
    int fgetattr(const char *path, struct stat *buf, struct fuse_file_info *file_info);
    void* init(struct fuse_conn_info *conn);
    int create(const char *path, mode_t mode, fuse_file_info *file_info);

private:
    static char *expandTmpDir(char *tmp);
    static bool removeDir(const std::string &dirname);

    bool removeTmpDir();

    static std::unique_ptr<SMTPFileSystem> s_instance;
    struct fuse_args m_args;
    struct fuse_operations m_fuse_operations;
    TmpFilesPool m_tmp_files_pool;
    SMTPFileSystemOptions m_options;
    MTPDevice m_device;
};

std::string smtpfs_dirname(const std::string &path);
std::string smtpfs_basename(const std::string &path);

#endif // SMTPFS_FUSE_H
