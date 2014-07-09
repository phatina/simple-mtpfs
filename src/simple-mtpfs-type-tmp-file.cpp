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

#include <config.h>
#include <algorithm>
#include "simple-mtpfs-type-tmp-file.h"

TypeTmpFile::TypeTmpFile():
    m_path_device(),
    m_path_tmp(),
    m_file_descriptors(),
    m_modified(false)
{
}

TypeTmpFile::TypeTmpFile(const std::string &path_device,
        const std::string &path_tmp, int file_desc,
        bool modified):
    m_path_device(path_device),
    m_path_tmp(path_tmp),
    m_modified(modified)
{
    m_file_descriptors.insert(file_desc);
}

TypeTmpFile::TypeTmpFile(const TypeTmpFile &copy):
    m_path_device(copy.m_path_device),
    m_path_tmp(copy.m_path_tmp),
    m_file_descriptors(copy.m_file_descriptors),
    m_modified(copy.m_modified)
{
}

bool TypeTmpFile::hasFileDescriptor(int fd)
{
    auto it = std::find(m_file_descriptors.begin(),
        m_file_descriptors.end(), fd);
    return it != m_file_descriptors.end();
}

void TypeTmpFile::removeFileDescriptor(int fd)
{
    auto it = std::find(m_file_descriptors.begin(),
        m_file_descriptors.end(), fd);
    if (it == m_file_descriptors.end())
        return;
    m_file_descriptors.erase(it);
}

TypeTmpFile &TypeTmpFile::operator =(const TypeTmpFile &rhs)
{
    m_path_device = rhs.m_path_device;
    m_path_tmp = rhs.m_path_tmp;
    m_file_descriptors = rhs.m_file_descriptors;
    m_modified = rhs.m_modified;
    return *this;
}
