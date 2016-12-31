/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2012-2016, Peter Hatina <phatina@gmail.com>
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
#include <string>
#include <vector>
extern "C" {
#  include <libmtp.h>
}
#include "simple-mtpfs-type-dir.h"
#include "simple-mtpfs-type-file.h"

class MTPDevice
{
public:
    class Capabilities
    {
    public:
        Capabilities()
            : m_get_partial_object(false)
            , m_send_partial_object(false)
            , m_edit_objects(false)
        {
        }

        void setCanGetPartialObject(bool b) { m_get_partial_object = b; }
        void setCanSendPartialobject(bool b) { m_send_partial_object = b; }
        void setCanEditObjects(bool b) { m_edit_objects = b; }

        bool canGetPartialObject()  const { return m_get_partial_object; }
        bool canSendPartialObject() const { return m_send_partial_object; }
        bool canEditObjects() const { return m_edit_objects; }

    private:
        bool m_get_partial_object;
        bool m_send_partial_object;
        bool m_edit_objects;
    };

    // -------------------------------------------------------------------------

    MTPDevice();
    ~MTPDevice();

    bool connect(LIBMTP_raw_device_t *dev);
    bool connect(int dev_no = 0);
    bool connect(const std::string &dev_file);
    void disconnect();

    void enableMove(bool e = true) { m_move_enabled = e; }

    uint64_t storageTotalSize() const;
    uint64_t storageFreeSize() const;

    int dirCreateNew(const std::string &path);
    int dirRemove(const std::string &path);
    int dirRename(const std::string &oldpath, const std::string &newpath);
    const TypeDir *dirFetchContent(std::string path);

    int rename(const std::string &oldpath, const std::string &newpath);

    int filePull(const std::string &src, const std::string &dst);
    int filePush(const std::string &src, const std::string &dst);
    int fileRemove(const std::string &path);
    int fileRename(const std::string &oldpath, const std::string &newpath);

    int fileGetPartial();
    int filePutPartial();

    Capabilities getCapabilities() const;

    static bool listDevices(bool verbose, const std::string &dev_file);

private:
    void criticalEnter() { m_device_mutex.lock(); }
    void criticalLeave() { m_device_mutex.unlock(); }

    bool enumStorages();

    static Capabilities getCapabilities(const MTPDevice &device);
    bool connect_priv(int dev_no, const std::string &dev_file);

private:
    LIBMTP_mtpdevice_t *m_device;
    Capabilities m_capabilities;
    std::mutex m_device_mutex;
    TypeDir m_root_dir;
    bool m_move_enabled;
    static uint32_t s_root_node;
};

#endif // SMTPFS_MTP_DEVICE_H
