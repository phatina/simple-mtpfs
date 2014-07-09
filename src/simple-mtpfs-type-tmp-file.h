/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2012-2014, Peter Hatina <phatina@gmail.com>
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

#ifndef SMTPFS_TYPE_TMP_FILE_H
#define SMTPFS_TYPE_TMP_FILE_H

#include <set>
#include <string>
#include "simple-mtpfs-type-file.h"
#include "simple-mtpfs-log.h"

class TypeTmpFile
{
public:
    TypeTmpFile();
    TypeTmpFile(const TypeTmpFile &copy);
    TypeTmpFile(const std::string &path_device, const std::string &path_tmp,
        int file_desc, bool modified = false);

    std::string pathDevice() const { return m_path_device; }
    std::string pathTmp() const { return m_path_tmp; }

    bool isModified() const { return m_modified; }
    void setModified(bool modified = true) { m_modified = modified; }

    std::set<int> fileDescriptors() const { return m_file_descriptors; }
    void addFileDescriptor(int fd) { m_file_descriptors.insert(fd); }
    bool hasFileDescriptor(int fd);
    void removeFileDescriptor(int fd);
    int refcnt() const { return m_file_descriptors.size(); }

    TypeTmpFile &operator =(const TypeTmpFile &rhs);

    bool operator ==(const TypeTmpFile &rhs) const
    {
        return m_path_device == rhs.m_path_device;
    }
    bool operator ==(const std::string &path) const
    {
        return m_path_device == path;
    }
    bool operator <(const TypeTmpFile &rhs) const
    {
        return m_path_device < rhs.m_path_device;
    }
    bool operator <(const std::string &path) const
    {
        return m_path_device < path;
    }

private:
    std::string m_path_device;
    std::string m_path_tmp;
    std::set<int> m_file_descriptors;
    bool m_modified;
};

#endif // SMTPFS_TYPE_TMP_FILE_H
