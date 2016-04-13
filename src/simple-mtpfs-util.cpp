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

#include <config.h>
#include <cstdio>
#include <cstring>
#ifdef HAVE_LIBUSB1
#  include <iomanip>
#  include <sstream>
#endif // HAVE_LIBUSB1
extern "C" {
#  include <dirent.h>
#  include <libgen.h>
#  include <limits.h>
#  include <stdlib.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
}
#ifdef HAVE_LIBUSB1
#  include <climits>
extern "C" {
#  include <libmtp.h>
#  include <libusb.h>
}
#endif // HAVE_LIBUSB1
#include "simple-mtpfs-log.h"
#include "simple-mtpfs-util.h"

const std::string devnull = "/dev/null";

bool StreamHelper::s_enabled = false;
int StreamHelper::s_stdout = -1;
int StreamHelper::s_stderr = -1;

void StreamHelper::on()
{
    if (!s_enabled)
        return;

    freopen(devnull.c_str(), "w", stdout);
    freopen(devnull.c_str(), "w", stderr);
    dup2(s_stdout, fileno(stdout));
    dup2(s_stderr, fileno(stderr));
    close(s_stdout);
    close(s_stderr);
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    s_enabled = false;
}

void StreamHelper::off()
{
    if (s_enabled)
        return;

    fflush(stdout);
    fflush(stderr);
    s_stdout = dup(fileno(stdout));
    s_stderr = dup(fileno(stderr));
    freopen(devnull.c_str(), "w", stdout);
    freopen(devnull.c_str(), "w", stderr);

    s_enabled = true;
}

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

std::string smtpfs_realpath(const std::string &path)
{
    char buf[PATH_MAX + 1];
    char *real_path = realpath(path.c_str(), buf);
    return std::string(real_path ? buf : "");
}

std::string smtpfs_get_tmpdir()
{
    const char *c_tmp = getenv("TMP");
    std::string tmp_dir;
    if (c_tmp) {
        tmp_dir = smtpfs_realpath(c_tmp);
    } else {
        c_tmp = getenv("TMPDIR");
        if (!c_tmp)
            c_tmp = TMPDIR;
        tmp_dir = smtpfs_realpath(c_tmp);
    }

    tmp_dir += "/simple-mtpfs-XXXXXX";
    char *c_tmp_dir = ::mktemp(::strdup(tmp_dir.c_str()));

    tmp_dir.assign(c_tmp_dir);
    ::free(static_cast<void*>(c_tmp_dir));

    return tmp_dir;
}

bool smtpfs_create_dir(const std::string &dirname)
{
    return ::mkdir(dirname.c_str(), S_IRWXU) == 0;
}

bool smtpfs_remove_dir(const std::string &dirname)
{
    DIR *dir;
    struct dirent *entry;
    std::string path;

    dir = ::opendir(dirname.c_str());
    if (!dir)
        return false;

    while ((entry = ::readdir(dir))) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            path = dirname + "/" + entry->d_name;
            if (entry->d_type == DT_DIR)
                return smtpfs_remove_dir(path);
            ::unlink(path.c_str());
        }
    }
    ::closedir(dir);
    ::remove(dirname.c_str());
    return true;
}

bool smtpfs_usb_devpath(const std::string &path, uint8_t *bnum, uint8_t *dnum)
{
    unsigned int bus, dev;
#ifdef USB_DEVPATH
    std::string realpath(smtpfs_realpath(path));
    if (realpath.empty() ||
        sscanf(realpath.c_str(), USB_DEVPATH, &bus, &dev) != 2)
#endif
        if (sscanf(path.c_str(), "%u/%u", &bus, &dev) != 2)
            return false;

    if (bus > 255 || dev > 255)
        return false;

    *bnum = bus;
    *dnum = dev;
    return true;
}

#ifdef HAVE_LIBUSB1
LIBMTP_raw_device_t *smtpfs_raw_device_new_priv(libusb_device *usb_device)
{
    if (!usb_device)
        return nullptr;

    LIBMTP_raw_device_t *device = static_cast<LIBMTP_raw_device_t*>(
        malloc(sizeof(LIBMTP_raw_device_t)));

    if (!device)
        return nullptr;

    struct libusb_device_descriptor desc;
    int err = libusb_get_device_descriptor(usb_device, &desc);
    if (err != LIBUSB_SUCCESS) {
        free(static_cast<void*>(device));
        return nullptr;
    }

    device->device_entry.vendor = nullptr;  // TODO: vendor string
    device->device_entry.vendor_id = desc.idVendor;
    device->device_entry.product = nullptr; // TODO: product string
    device->device_entry.product_id = desc.idProduct;
    device->device_entry.device_flags = 0;

    device->bus_location = static_cast<uint32_t>(libusb_get_bus_number(usb_device));
    device->devnum = libusb_get_device_address(usb_device);

    return device;
}

LIBMTP_raw_device_t *smtpfs_raw_device_new(const std::string &path)
{
    uint8_t bnum, dnum;
    if (!smtpfs_usb_devpath(path, &bnum, &dnum))
        return nullptr;

    if (libusb_init(NULL) != 0)
        return nullptr;

    libusb_device **dev_list;
    ssize_t num_devs = libusb_get_device_list(NULL, &dev_list);
    if (!num_devs) {
        libusb_exit(NULL);
        return nullptr;
    }

    libusb_device *dev = nullptr;
    for (auto i = 0; i < num_devs; ++i) {
        dev = dev_list[i];
        if (bnum == libusb_get_bus_number(dev_list[i]) &&
            dnum == libusb_get_device_address(dev_list[i]))
            break;
        dev = nullptr;
    }

    LIBMTP_raw_device_t *raw_device = smtpfs_raw_device_new_priv(dev);

    libusb_free_device_list(dev_list, 0);
    libusb_exit(NULL);

    return raw_device;
}

bool smtpfs_reset_device(LIBMTP_raw_device_t *device)
{
    if (libusb_init(NULL) != 0)
        return false;

    libusb_device **dev_list;
    ssize_t num_devs = libusb_get_device_list(NULL, &dev_list);
    if (!num_devs) {
        libusb_exit(NULL);
        return false;
    }

    libusb_device_handle *dev_handle = nullptr;
    for (auto i = 0; i < num_devs; ++i) {
        uint8_t bnum = libusb_get_bus_number(dev_list[i]);
        uint8_t dnum = libusb_get_device_address(dev_list[i]);

        if (static_cast<uint32_t>(bnum) == device->bus_location &&
            dnum == device->devnum)
        {
            libusb_open(dev_list[i], &dev_handle);
            libusb_reset_device(dev_handle);
            libusb_close(dev_handle);
            break;
        }
    }

    libusb_free_device_list(dev_list, 0);
    libusb_exit(NULL);

    return true;
}

void smtpfs_raw_device_free(LIBMTP_raw_device_t *device)
{
    if (!device)
        return;

    free(static_cast<void*>(device->device_entry.vendor));
    free(static_cast<void*>(device->device_entry.product));
    free(static_cast<void*>(device));
}
#endif // HAVE_LIBUSB1

bool smtpfs_check_dir(const std::string &path)
{
    struct stat buf;
    if (::stat(path.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode)
        && ::access(path.c_str(), R_OK | W_OK | X_OK) == 0)
    {
        return true;
    }

    return false;
}
