/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2012-2015, Peter Hatina <phatina@gmail.com>
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
#include <algorithm>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdlib>
extern "C" {
#  include <unistd.h>
#  include <sys/types.h>
#  define _DARWIN_USE_64_BIT_INODE
#  include <sys/stat.h>
}
#include "simple-mtpfs-fuse.h"
#include "simple-mtpfs-libmtp.h"
#include "simple-mtpfs-log.h"
#include "simple-mtpfs-mtp-device.h"
#include "simple-mtpfs-util.h"

uint32_t MTPDevice::s_root_node = ~0;

MTPDevice::MTPDevice():
    m_device(nullptr),
    m_capabilities(),
    m_device_mutex(),
    m_root_dir(),
    m_move_enabled(false)
{
    StreamHelper::off();
    LIBMTP_Init();
    StreamHelper::on();
}

MTPDevice::~MTPDevice()
{
    disconnect();
}

bool MTPDevice::connect(LIBMTP_raw_device_t *dev)
{
    if (m_device) {
        logerr("Already connected.\n");
        return true;
    }

    // Do not output LIBMTP debug stuff
    StreamHelper::off();
    m_device = LIBMTP_Open_Raw_Device_Uncached(dev);
    StreamHelper::on();

    if (!m_device) {
        LIBMTP_Dump_Errorstack(m_device);
        return false;
    }

    if (!enumStorages())
        return false;

    // Retrieve capabilities.
    m_capabilities = MTPDevice::getCapabilities(*this);

    logmsg("Connected.\n");
    return true;
}

bool MTPDevice::connect(int dev_no)
{
    if (m_device) {
        logerr("Already connected.\n");
        return true;
    }

    int raw_devices_cnt;
    LIBMTP_raw_device_t *raw_devices;

    // Do not output LIBMTP debug stuff
    StreamHelper::off();
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(
        &raw_devices, &raw_devices_cnt);
    StreamHelper::on();

    if (dev_no < 0 || dev_no >= raw_devices_cnt) {
        logerr("Can not connect to device no. ", dev_no + 1, ".\n");
        free(static_cast<void*>(raw_devices));
        return false;
    }

    if (err != LIBMTP_ERROR_NONE) {
        switch(err) {
        case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
            logerr("No raw devices found.\n");
            break;
        case LIBMTP_ERROR_CONNECTING:
            logerr("There has been an error connecting. Exiting.\n");
            break;
        case LIBMTP_ERROR_MEMORY_ALLOCATION:
            logerr("Encountered a Memory Allocation Error. Exiting.\n");
            break;
        case LIBMTP_ERROR_GENERAL:
            logerr("General error occured. Exiting.\n");
            break;
        case LIBMTP_ERROR_USB_LAYER:
            logerr("USB Layer error occured. Exiting.\n");
            break;
        default:
            break;
        }
        return false;
    }

    LIBMTP_raw_device_t *raw_device = &raw_devices[dev_no];

#ifdef HAVE_LIBUSB1
    // Try to reset USB device, so we don't wait until LIBMTP times out.
    // We do this every time we are about to mount a device, but better
    // connect on first try, than wait for 60s timeout.
    smtpfs_reset_device(raw_device);
#endif // HAVE_LIBUSB1

    // Do not output LIBMTP debug stuff
    StreamHelper::off();
    m_device = LIBMTP_Open_Raw_Device_Uncached(raw_device);
    StreamHelper::on();
    free(static_cast<void*>(raw_devices));

    if (!m_device) {
        LIBMTP_Dump_Errorstack(m_device);
        return false;
    }

    if (!enumStorages())
        return false;

    // Retrieve capabilities.
    m_capabilities = MTPDevice::getCapabilities(*this);

    logmsg("Connected.\n");
    return true;
}

#ifdef HAVE_LIBUSB1
bool MTPDevice::connect(const std::string &dev_file)
{
    if (m_device) {
        logerr("Already connected.\n");
        return true;
    }

    LIBMTP_raw_device_t *raw_device = smtpfs_raw_device_new(dev_file);
    if (!raw_device) {
        logerr("Can not open such device '", dev_file, "'.\n");
        return false;
    }

#ifdef HAVE_LIBUSB1
    // Try to reset USB device, so we don't wait until LIBMTP times out.
    // We do this every time we are about to mount a device, but better
    // connect on first try, than wait for 60s timeout.
    smtpfs_reset_device(raw_device);
#endif // HAVE_LIBUSB1

    bool rval = connect(raw_device);

    // TODO:  Smart pointer with alloc, free hooks.
    smtpfs_raw_device_free(raw_device);

    return rval;
}
#endif

void MTPDevice::disconnect()
{
    if (!m_device)
        return;

    LIBMTP_Release_Device(m_device);
    m_device = nullptr;
    logmsg("Disconnected.\n");
}

uint64_t MTPDevice::storageTotalSize() const
{
    uint64_t total = 0;
    for (LIBMTP_devicestorage_t *s = m_device->storage; s; s = s->next)
        total += s->MaxCapacity;
    return total;
}

uint64_t MTPDevice::storageFreeSize() const
{
    uint64_t free = 0;
    for (LIBMTP_devicestorage_t *s = m_device->storage; s; s = s->next)
        free += s->FreeSpaceInBytes;
    return free;
}

bool MTPDevice::enumStorages()
{
    criticalEnter();
    LIBMTP_Clear_Errorstack(m_device);
    if (LIBMTP_Get_Storage(m_device, LIBMTP_STORAGE_SORTBY_NOTSORTED) < 0) {
        std::cerr << "Could not retrieve device storage.\n";
        std::cerr << "For android phones make sure the screen is unlocked.\n";
        logerr("Could not retrieve device storage. Exiting.\n");
        LIBMTP_Dump_Errorstack(m_device);
        LIBMTP_Clear_Errorstack(m_device);
        return false;
    }
    criticalLeave();
    return true;
}

const TypeDir *MTPDevice::dirFetchContent(std::string path)
{
    if (!m_root_dir.isFetched()) {
        for (LIBMTP_devicestorage_t *s = m_device->storage; s; s = s->next) {
            m_root_dir.addDir(TypeDir(s_root_node, 0, s->id,
                std::string(s->StorageDescription)));
            m_root_dir.setFetched();
        }
    }

    if (m_root_dir.dirCount() == 1)
        path = '/' + m_root_dir.dirs().begin()->name() + path;

    if (path == "/")
        return &m_root_dir;

    std::string member;
    std::istringstream ss(path);
    TypeDir *dir = &m_root_dir;
    while (std::getline(ss, member, '/')) {
        if (member.empty())
            continue;

        const TypeDir *tmp = dir->dir(member);
        if (!tmp && !dir->isFetched()) {
            criticalEnter();
            LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(
                m_device, dir->storageid(), dir->id());
            criticalLeave();
            for (LIBMTP_file_t *f = content; f; f = f->next) {
                if (f->filetype == LIBMTP_FILETYPE_FOLDER)
                    dir->addDir(TypeDir(f));
                else
                    dir->addFile(TypeFile(f));
            }
            LIBMTP_Free_Files_And_Folders(&content);
            dir->setFetched();
            tmp = dir->dir(member);
        }

        if (!tmp)
            return nullptr;
        dir = const_cast<TypeDir*>(tmp);
    }

    if (dir->isFetched())
        return dir;

    criticalEnter();
    dir->setFetched();
    LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(
        m_device, dir->storageid(), dir->id());
    criticalLeave();
    for (LIBMTP_file_t *f = content; f; f = f->next) {
        if (f->filetype == LIBMTP_FILETYPE_FOLDER)
            dir->addDir(TypeDir(f));
        else
            dir->addFile(TypeFile(f));
    }
    LIBMTP_Free_Files_And_Folders(&content);
    return dir;
}

int MTPDevice::dirCreateNew(const std::string &path)
{
    const std::string tmp_basename(smtpfs_basename(path));
    const std::string tmp_dirname(smtpfs_dirname(path));
    const TypeDir *dir_parent = dirFetchContent(tmp_dirname);
    if (!dir_parent || dir_parent->id() == 0) {
        logerr("Can not remove directory '", path, "'.\n");
        return -EINVAL;
    }
    char *c_name = strdup(tmp_basename.c_str());
    criticalEnter();
    uint32_t new_id = LIBMTP_Create_Folder(m_device, c_name,
        dir_parent->id(), dir_parent->storageid());
    criticalLeave();
    if (new_id == 0) {
        logerr("Could not create directory '", path, "'.\n");
        LIBMTP_Dump_Errorstack(m_device);
        LIBMTP_Clear_Errorstack(m_device);
    } else {
        const_cast<TypeDir*>(dir_parent)->addDir(TypeDir(new_id, dir_parent->id(),
            dir_parent->storageid(), tmp_basename));
        logmsg("Directory '", path, "' created.\n");
    }
    free(static_cast<void*>(c_name));
    return new_id != 0 ? 0 : -EINVAL;
}

int MTPDevice::dirRemove(const std::string &path)
{
    const std::string tmp_basename(smtpfs_basename(path));
    const std::string tmp_dirname(smtpfs_dirname(path));
    const TypeDir *dir_parent = dirFetchContent(tmp_dirname);
    const TypeDir *dir_to_remove = dir_parent ? dir_parent->dir(tmp_basename) : nullptr;
    if (!dir_parent || !dir_to_remove || dir_parent->id() == 0) {
        logerr("No such directory '", path, "' to remove.\n");
        return -ENOENT;
    }
    if (!dir_to_remove->isEmpty())
        return -ENOTEMPTY;
    criticalEnter();
    int rval = LIBMTP_Delete_Object(m_device, dir_to_remove->id());
    criticalLeave();
    if (rval != 0){
        logerr("Could not remove the directory '", path, "'.\n");
        LIBMTP_Dump_Errorstack(m_device);
        LIBMTP_Clear_Errorstack(m_device);
        return -EINVAL;
    }
    const_cast<TypeDir*>(dir_parent)->removeDir(*dir_to_remove);
    logmsg("Folder '", path, "' removed.\n");
    return 0;
}

int MTPDevice::dirRename(const std::string &oldpath, const std::string &newpath)
{
    const std::string tmp_old_basename(smtpfs_basename(oldpath));
    const std::string tmp_old_dirname(smtpfs_dirname(oldpath));
    const std::string tmp_new_basename(smtpfs_basename(newpath));
    const std::string tmp_new_dirname(smtpfs_dirname(newpath));
    const TypeDir *dir_parent = dirFetchContent(tmp_old_dirname);
    const TypeDir *dir_to_rename = dir_parent ? dir_parent->dir(tmp_old_basename) : nullptr;
    if (!dir_parent || !dir_to_rename || dir_parent->id() == 0) {
        logerr("Can not rename '", tmp_old_basename, "' to '",
            tmp_new_basename, "'.\n");
        return -EINVAL;
    }
    if (tmp_old_dirname != tmp_new_dirname) {
        logerr("Can not move '", oldpath, "' to '", newpath, "'.\n");
        return -EINVAL;
    }

    LIBMTP_folder_t *folder = dir_to_rename->toLIBMTPFolder();
    criticalEnter();
    int ret = LIBMTP_Set_Folder_Name(m_device, folder, tmp_new_basename.c_str());
    criticalLeave();
    free(static_cast<void*>(folder->name));
    free(static_cast<void*>(folder));
    if (ret != 0) {
        logerr("Could not rename '", oldpath, "' to '",  tmp_new_basename, "'.\n");
        LIBMTP_Dump_Errorstack(m_device);
        LIBMTP_Clear_Errorstack(m_device);
        return -EINVAL;
    }
    const_cast<TypeDir*>(dir_to_rename)->setName(tmp_new_basename);
    logmsg("Directory '", oldpath, "' renamed to '", tmp_new_basename, "'.\n");
    return 0;
}

int MTPDevice::rename(const std::string &oldpath, const std::string &newpath)
{
#ifndef SMTPFS_MOVE_BY_SET_OBJECT_PROPERTY
    const std::string tmp_old_basename(smtpfs_basename(oldpath));
    const std::string tmp_old_dirname(smtpfs_dirname(oldpath));
    const std::string tmp_new_dirname(smtpfs_dirname(newpath));
    if (tmp_old_dirname != tmp_new_dirname)
        return -EINVAL;

    const TypeDir *dir_parent = dirFetchContent(tmp_old_dirname);
    if (!dir_parent || dir_parent->id() == 0)
        return -EINVAL;
    const TypeDir *dir_to_rename = dir_parent->dir(tmp_old_basename);
    if (dir_to_rename)
        return dirRename(oldpath, newpath);
    else
        return fileRename(oldpath, newpath);
#else
    const std::string tmp_old_basename(smtpfs_basename(oldpath));
    const std::string tmp_old_dirname(smtpfs_dirname(oldpath));
    const std::string tmp_new_basename(smtpfs_basename(newpath));
    const std::string tmp_new_dirname(smtpfs_dirname(newpath));
    const TypeDir *dir_old_parent = dirFetchContent(tmp_old_dirname);
    const TypeDir *dir_new_parent = dirFetchContent(tmp_new_dirname);
    const TypeDir *dir_to_rename = dir_old_parent ? dir_old_parent->dir(tmp_old_basename) : nullptr;
    const TypeFile *file_to_rename = dir_old_parent ? dir_old_parent->file(tmp_old_basename) : nullptr;

    logdebug("dir_to_rename:    ", dir_to_rename, "\n");
    logdebug("file_to_rename:   ", file_to_rename, "\n");

    if (!dir_old_parent || !dir_new_parent || dir_old_parent->id() == 0)
        return -EINVAL;

    const TypeBasic *object_to_rename =  dir_to_rename ?
        static_cast<const TypeBasic*>(dir_to_rename) :
        static_cast<const TypeBasic*>(file_to_rename);

    logdebug("object_to_rename: ", object_to_rename, "\n");
    logdebug("object_to_rename->id(): ", object_to_rename->id(), "\n");

    if (!object_to_rename) {
        logerr("No such file or directory to rename/move!\n");
        return -ENOENT;
    }

    if (tmp_old_dirname != tmp_new_dirname) {
        criticalEnter();
        int rval = LIBMTP_Set_Object_u32(m_device, object_to_rename->id(),
            LIBMTP_PROPERTY_ParentObject, dir_new_parent->id());
        criticalLeave();
        if (rval != 0) {
            logerr("Could not move '", oldpath, "' to '", newpath, "'.\n");
            LIBMTP_Dump_Errorstack(m_device);
            LIBMTP_Clear_Errorstack(m_device);
            return -EINVAL;
        }
        const_cast<TypeBasic*>(object_to_rename)->setParent(dir_new_parent->id());
    }
    if (tmp_old_basename != tmp_new_basename) {
        criticalEnter();
        int rval = LIBMTP_Set_Object_String(m_device, object_to_rename->id(),
            LIBMTP_PROPERTY_Name, tmp_new_basename.c_str());
        criticalLeave();
        if (rval != 0) {
            logerr("Could not rename '", oldpath, "' to '", newpath, "'.\n");
            LIBMTP_Dump_Errorstack(m_device);
            LIBMTP_Clear_Errorstack(m_device);
            return -EINVAL;
        }
    }
    return 0;
#endif
}

int MTPDevice::filePull(const std::string &src, const std::string &dst)
{
    const std::string src_basename(smtpfs_basename(src));
    const std::string src_dirname(smtpfs_dirname(src));
    const TypeDir *dir_parent = dirFetchContent(src_dirname);
    const TypeFile *file_to_fetch = dir_parent ? dir_parent->file(src_basename) : nullptr;
    if (!dir_parent) {
        logerr("Can not fetch '", src, "'.\n");
        return -EINVAL;
    }
    if (!file_to_fetch) {
        logerr("No such file '", src, "'.\n");
        return -ENOENT;
    }
    if (file_to_fetch->size() == 0) {
        int fd = ::creat(dst.c_str(), S_IRUSR | S_IWUSR);
        ::close(fd);
    } else {
        logmsg("Started fetching '", src, "'.\n");
        criticalEnter();
        int rval = LIBMTP_Get_File_To_File(m_device, file_to_fetch->id(),
            dst.c_str(), nullptr, nullptr);
        criticalLeave();
        if (rval != 0) {
            logerr("Could not fetch file '", src, "'.\n");
            LIBMTP_Dump_Errorstack(m_device);
            LIBMTP_Clear_Errorstack(m_device);
            return -ENOENT;
        }
    }
    logmsg("File fetched '", src, "'.\n");
    return 0;
}

int MTPDevice::filePush(const std::string &src, const std::string &dst)
{
    const std::string dst_basename(smtpfs_basename(dst));
    const std::string dst_dirname(smtpfs_dirname(dst));
    const TypeDir *dir_parent = dirFetchContent(dst_dirname);
    const TypeFile *file_to_remove = dir_parent ? dir_parent->file(dst_basename) : nullptr;
    if (dir_parent && file_to_remove) {
        criticalEnter();
        int rval = LIBMTP_Delete_Object(m_device, file_to_remove->id());
        criticalLeave();
        if (rval != 0) {
            logerr("Can not upload '", src, "' to '", dst, "'.\n");
            return -EINVAL;
        }
    }

    struct stat file_stat;
    stat(src.c_str(), &file_stat);
    TypeFile file_to_upload(0, dir_parent->id(), dir_parent->storageid(),
        dst_basename, static_cast<uint64_t>(file_stat.st_size), 0);
    LIBMTP_file_t *f = file_to_upload.toLIBMTPFile();
    if (file_stat.st_size)
        logmsg("Started uploading '", dst, "'.\n");
    criticalEnter();
    int rval = LIBMTP_Send_File_From_File(m_device, src.c_str(), f, nullptr, nullptr);
    criticalLeave();
    if (rval != 0) {
        logerr("Could not upload file '", src, "'.\n");
        LIBMTP_Dump_Errorstack(m_device);
        LIBMTP_Clear_Errorstack(m_device);
        rval = -EINVAL;
    } else {
        file_to_upload.setId(f->item_id);
        file_to_upload.setParent(f->parent_id);
        file_to_upload.setStorage(f->storage_id);
        file_to_upload.setName(std::string(f->filename));
        file_to_upload.setModificationDate(file_stat.st_mtime);
        if (file_to_remove)
            const_cast<TypeDir*>(dir_parent)->replaceFile(*file_to_remove, file_to_upload);
        else
            const_cast<TypeDir*>(dir_parent)->addFile(file_to_upload);
    }
    free(static_cast<void*>(f->filename));
    free(static_cast<void*>(f));
    logmsg("File '", dst, (file_stat.st_size ? " uploaded" : " created"), ".\n");
    return rval;
}

int MTPDevice::fileRemove(const std::string &path)
{
    const std::string tmp_basename(smtpfs_basename(path));
    const std::string tmp_dirname(smtpfs_dirname(path));
    const TypeDir *dir_parent = dirFetchContent(tmp_dirname);
    const TypeFile *file_to_remove = dir_parent ? dir_parent->file(tmp_basename) : nullptr;
    if (!dir_parent || !file_to_remove) {
        logerr("No such file '", path, "' to remove.\n");
        return -ENOENT;
    }
    criticalEnter();
    int rval = LIBMTP_Delete_Object(m_device, file_to_remove->id());
    criticalLeave();
    if (rval != 0) {
        logerr("Could not remove the directory '", path, "'.\n");
        return -EINVAL;
    }
    const_cast<TypeDir*>(dir_parent)->removeFile(*file_to_remove);
    logmsg("File '", path, "' removed.\n");
    return 0;
}

int MTPDevice::fileRename(const std::string &oldpath, const std::string &newpath)
{
    const std::string tmp_old_basename(smtpfs_basename(oldpath));
    const std::string tmp_old_dirname(smtpfs_dirname(oldpath));
    const std::string tmp_new_basename(smtpfs_basename(newpath));
    const std::string tmp_new_dirname(smtpfs_dirname(newpath));
    const TypeDir *dir_parent = dirFetchContent(tmp_old_dirname);
    const TypeFile *file_to_rename = dir_parent ? dir_parent->file(tmp_old_basename) : nullptr;
    if (!dir_parent || !file_to_rename || tmp_old_dirname != tmp_new_dirname) {
        logerr("Can not rename '", oldpath, "' to '", tmp_new_basename, "'.\n");
        return -EINVAL;
    }

    LIBMTP_file_t *file = file_to_rename->toLIBMTPFile();
    criticalEnter();
    int rval = LIBMTP_Set_File_Name(m_device, file, tmp_new_basename.c_str());
    criticalLeave();
    free(static_cast<void*>(file->filename));
    free(static_cast<void*>(file));
    if (rval > 0) {
        logerr("Could not rename '", oldpath, "' to '", newpath, "'.\n");
        LIBMTP_Dump_Errorstack(m_device);
        LIBMTP_Clear_Errorstack(m_device);
        return -EINVAL;
    }
    const_cast<TypeFile*>(file_to_rename)->setName(tmp_new_basename);
    logmsg("File '", oldpath, "' renamed to '", tmp_new_basename, "'.\n");
    return 0;
}

MTPDevice::Capabilities MTPDevice::getCapabilities() const
{
    return m_capabilities;
}

MTPDevice::Capabilities MTPDevice::getCapabilities(const MTPDevice &device)
{
    MTPDevice::Capabilities capabilities;

#ifdef HAVE_LIBMTP_CHECK_CAPABILITY
    if (device.m_device) {
        capabilities.setCanGetPartialObject(
            static_cast<bool>(
                LIBMTP_Check_Capability(
                    device.m_device,
                    LIBMTP_DEVICECAP_GetPartialObject)));
        capabilities.setCanSendPartialobject(
            static_cast<bool>(
                LIBMTP_Check_Capability(
                    device.m_device,
                    LIBMTP_DEVICECAP_SendPartialObject)));
        capabilities.setCanEditObjects(
            static_cast<bool>(
                LIBMTP_Check_Capability(
                    device.m_device,
                    LIBMTP_DEVICECAP_EditObjects)));
    }
#endif

    return capabilities;
}

bool MTPDevice::listDevices(bool verbose)
{
    int raw_devices_cnt;
    LIBMTP_raw_device_t *raw_devices;

    // Do not output LIBMTP debug stuff
    StreamHelper::off();
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(
        &raw_devices, &raw_devices_cnt);
    StreamHelper::on();

    if (err != 0) {
        if (err == LIBMTP_ERROR_NO_DEVICE_ATTACHED)
            std::cerr << "No raw devices found.\n";
        return false;
    }

    for (int i = 0; i < raw_devices_cnt; ++i) {
        std::cout << i + 1 << ": "
            << (raw_devices[i].device_entry.vendor ? raw_devices[i].device_entry.vendor : "Unknown vendor ")
            << (raw_devices[i].device_entry.product ? raw_devices[i].device_entry.product : "Unknown product")
            << std::endl;
#ifdef HAVE_LIBMTP_CHECK_CAPABILITY
            MTPDevice dev;
            if (verbose) {
                if (!dev.connect(&raw_devices[i]))
                    return false;

                const MTPDevice::Capabilities &cap = dev.getCapabilities();
                std::cout << "  - can get  partial object: " << (cap.canGetPartialObject() ? "yes" : "no") << std::endl;
                std::cout << "  - can send partial object: " << (cap.canSendPartialObject() ? "yes" : "no") << std::endl;
                std::cout << "  - can edit objects       : " << (cap.canEditObjects() ? "yes" : "no") << std::endl;
                dev.disconnect();
            }
#endif
    }
    free(static_cast<void*>(raw_devices));

    return true;
}
