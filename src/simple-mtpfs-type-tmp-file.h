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

#ifndef SMTPFS_TYPE_TMP_FILE_H
#define SMTPFS_TYPE_TMP_FILE_H

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

    const std::string pathDevice() const { return m_path_device; }
    const std::string pathTmp() const { return m_path_tmp; }
    int fileDescriptor() const { return m_file_desc; }
    bool isModified() const { return m_modified; }

    void setModified(bool modified = true) { m_modified = modified; }

    TypeTmpFile &operator =(const TypeTmpFile &rhs);
    bool operator ==(const TypeTmpFile &rhs) const
    {
        return m_file_desc == rhs.m_file_desc;
    }
    bool operator ==(int desc) const
    {
        return m_file_desc == desc;
    }
    bool operator <(const TypeTmpFile &rhs) const
    {
        return m_file_desc < rhs.m_file_desc;
    }
    bool operator <(int desc) const
    {
        return m_file_desc < desc;
    }


private:
    std::string m_path_device;
    std::string m_path_tmp;
    int m_file_desc;
    bool m_modified;
};

#endif // SMTPFS_TYPE_TMP_FILE_H
