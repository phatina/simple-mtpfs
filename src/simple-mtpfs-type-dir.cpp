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
#include <cstdlib>
#include <cstring>
extern "C" {
#  include <libmtp.h>
}
#include "simple-mtpfs-type-dir.h"

TypeDir::TypeDir():
    TypeBasic(),
    m_dirs(),
    m_files(),
    m_access_mutex(),
    m_fetched(false),
    m_modif_date(0)
{
}

TypeDir::TypeDir(uint32_t id, uint32_t parent_id, uint32_t storage_id,
    const std::string &name):
    TypeBasic(id, parent_id, storage_id, name),
    m_dirs(),
    m_files(),
    m_access_mutex(),
    m_fetched(false),
    m_modif_date(0)
{
}

TypeDir::TypeDir(LIBMTP_file_t *file):
    TypeBasic(file->item_id, file->parent_id,
        file->storage_id, std::string(file->filename)),
    m_dirs(),
    m_files(),
    m_access_mutex(),
    m_fetched(false),
    m_modif_date(file->modificationdate)
{
}

TypeDir::TypeDir(const TypeDir &copy):
    TypeBasic(copy),
    m_dirs(copy.m_dirs),
    m_files(copy.m_files),
    m_access_mutex(),
    m_fetched(copy.m_fetched),
    m_modif_date(copy.m_modif_date)
{
}

LIBMTP_folder_t *TypeDir::toLIBMTPFolder() const
{
    LIBMTP_folder_t *f = static_cast<LIBMTP_folder_t*>(
        malloc(sizeof(LIBMTP_folder_t)));
    if (!f)
        return nullptr;
    f->folder_id = m_id;
    f->parent_id = m_parent_id;
    f->storage_id = m_storage_id;
    f->name = strdup(m_name.c_str());
    f->sibling = nullptr;
    f->child = nullptr;
    return f;
}

void TypeDir::addDir(const TypeDir &dir)
{
    enterCritical();
    m_dirs.insert(dir);
    leaveCritical();
}

void TypeDir::addFile(const TypeFile &file)
{
    enterCritical();
    m_files.insert(file);
    leaveCritical();
}

bool TypeDir::removeDir(const TypeDir &dir)
{
    enterCritical();
    auto it = std::find(m_dirs.begin(), m_dirs.end(), dir);
    if (it == m_dirs.end()) {
        leaveCritical();
        return false;
    }
    m_dirs.erase(it);
    leaveCritical();
    return true;
}

bool TypeDir::removeFile(const TypeFile &file)
{
    enterCritical();
    auto it = std::find(m_files.begin(), m_files.end(), file);
    if (it == m_files.end()) {
        leaveCritical();
        return false;
    }
    m_files.erase(it);
    leaveCritical();
    return true;
}

bool TypeDir::replaceFile(const TypeFile &oldfile, const TypeFile &newfile)
{
    enterCritical();
    auto it = std::find(m_files.begin(), m_files.end(), oldfile);
    if (it == m_files.end()) {
        leaveCritical();
        return false;
    }
    m_files.erase(it);
    m_files.insert(newfile);
    leaveCritical();
    return true;
}

TypeDir &TypeDir::operator =(const TypeDir &rhs)
{
    TypeBasic::operator =(rhs);
    m_dirs = rhs.m_dirs;
    m_files = rhs.m_files;
    m_fetched = rhs.m_fetched;
    return *this;
}

const TypeDir *TypeDir::dir(const std::string &name) const
{
    enterCritical();
    auto it = std::find(m_dirs.begin(), m_dirs.end(), name);
    leaveCritical();
    if (it == m_dirs.end())
        return nullptr;
    return &*it;
}

const TypeFile *TypeDir::file(const std::string &name) const
{
    enterCritical();
    auto it = std::find(m_files.begin(), m_files.end(), name);
    leaveCritical();
    if (it == m_files.end())
        return nullptr;
    return &*it;
}
