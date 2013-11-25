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

#ifndef SIMPLE_MTPFS_UTIL
#define SIMPLE_MTPFS_UTIL

#include <config.h>
#include <string>

#ifdef HAVE_LIBUSB1
#  include <libmtp.h>
#endif // HAVE_LIBUSB1

class StreamHelper
{
public:
    StreamHelper() = delete;

    static void on();
    static void off();

private:
    static bool s_enabled;
    static int s_stdout;
    static int s_stderr;
};

std::string smtpfs_dirname(const std::string &path);
std::string smtpfs_basename(const std::string &path);
std::string smtpfs_realpath(const std::string &path);
std::string smtpfs_get_tmpdir();

bool smtpfs_create_dir(const std::string &dirname);
bool smtpfs_remove_dir(const std::string &dirname);
bool smtpfs_check_dir(const std::string &path);

#ifdef HAVE_LIBUSB1
LIBMTP_raw_device_t *smtpfs_raw_device_new(const std::string &path);
void smtpfs_raw_device_free(LIBMTP_raw_device_t *device);
#endif // HAVE_LIBUSB1

#endif // SIMPLE_MTPFS_UTIL
