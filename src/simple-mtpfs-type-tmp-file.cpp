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

#include <config.h>
#include "simple-mtpfs-type-tmp-file.h"

TypeTmpFile::TypeTmpFile():
    m_path_device(),
    m_path_tmp(),
    m_file_desc(0),
    m_modified(false)
{
}

TypeTmpFile::TypeTmpFile(const std::string &path_device,
    const std::string &path_tmp, int file_desc,
    bool modified):
    m_path_device(path_device),
    m_path_tmp(path_tmp),
    m_file_desc(file_desc),
    m_modified(modified)
{
}

TypeTmpFile::TypeTmpFile(const TypeTmpFile &copy):
    m_path_device(copy.m_path_device),
    m_path_tmp(copy.m_path_tmp),
    m_file_desc(copy.m_file_desc),
    m_modified(copy.m_modified)
{
}

TypeTmpFile &TypeTmpFile::operator =(const TypeTmpFile &rhs)
{
    m_path_device = rhs.m_path_device;
    m_path_tmp = rhs.m_path_tmp;
    m_file_desc = rhs.m_file_desc;
    m_modified = rhs.m_modified;
    return *this;
}
