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

#ifndef SMTPFS_MTP_DEVICE_H
#define SMTPFS_MTP_DEVICE_H

#include <map>
#include <mutex>
#include <stack>
#include <vector>
extern "C" {
#  include <libmtp.h>
}
#include "simple-mtpfs-type-dir.h"
#include "simple-mtpfs-type-file.h"

class MTPDevice
{
public:
    MTPDevice();
    ~MTPDevice();

    bool connect(int dev_no = 0);
    void disconnect();

    bool listDevices();
    void enableMove(bool e = true) { m_move_enabled = e; }

    uint64_t storageTotalSize() const;
    uint64_t storageFreeSize() const;

    int dirCreateNew(const std::string &path);
    int dirRemove(const std::string &path);
    int dirRename(const std::string &oldpath, const std::string &newpath);
    const TypeDir *dirFetchContent(std::string path);
    const TypeDir *dirGetContent(const std::string &path);

    int rename(const std::string &oldpath, const std::string &newpath);

    int filePull(const std::string &src, const std::string &dst);
    int filePush(const std::string &src, const std::string &dst);
    int fileRemove(const std::string &path);
    int fileRename(const std::string &oldpath, const std::string &newpath);

    void criticalEnter() { m_device_mutex.lock(); }
    void criticalLeave() { m_device_mutex.unlock(); }

private:
    bool enumStorages();

private:
    LIBMTP_mtpdevice_t *m_device;
    std::mutex m_device_mutex;
    TypeDir m_root_dir;
    bool m_move_enabled;
    static uint32_t s_root_node;
};

#endif // SMTPFS_MTP_DEVICE_H
