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
#include <cstdlib>
#include "simple-mtpfs-type-file.h"

TypeFile::TypeFile():
    TypeBasic(),
    m_size(0),
    m_modif_date(0)
{
}

TypeFile::TypeFile(uint32_t id, uint32_t parent_id, uint32_t storage_id,
    const std::string &name, uint64_t size, time_t modif_date):
    TypeBasic(id, parent_id, storage_id, name),
    m_size(size),
    m_modif_date(modif_date)
{
}

TypeFile::TypeFile(LIBMTP_file_t *file):
    TypeBasic(file->item_id, file->parent_id, file->storage_id,
        std::string(file->filename)),
    m_size(file->filesize),
    m_modif_date(file->modificationdate)
{
}

TypeFile::TypeFile(const TypeFile &copy):
    TypeBasic(copy),
    m_size(copy.m_size),
    m_modif_date(copy.m_modif_date)
{
}

LIBMTP_file_t *TypeFile::toLIBMTPFile() const
{
    LIBMTP_file_t *f = static_cast<LIBMTP_file_t*>(
        malloc(sizeof(LIBMTP_file_t)));
    if (!f)
        return nullptr;
    f->item_id = m_id;
    f->parent_id = m_parent_id;
    f->storage_id = m_storage_id;
    f->filename = strdup(m_name.c_str());
    f->filesize = m_size;
    f->modificationdate = m_modif_date;
    f->filetype = LIBMTP_FILETYPE_UNKNOWN;
    f->next = nullptr;
    return f;
}

TypeFile &TypeFile::operator =(const TypeFile &rhs)
{
    TypeBasic::operator =(rhs);
    m_size = rhs.m_size;
    m_modif_date = rhs.m_modif_date;
    return *this;
}
