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

#ifndef SMTPFS_TYPE_FILE_H
#define SMTPFS_TYPE_FILE_H

#include <cstring>
#include <string>
extern "C" {
#  include <libmtp.h>
}
#include "simple-mtpfs-type-basic.h"

class TypeFile: public TypeBasic
{
public:
    TypeFile();
    TypeFile(uint32_t id, uint32_t parent_id, uint32_t storage_id,
        const std::string &name, uint64_t size, time_t modif_date);
    TypeFile(LIBMTP_file_t *file);
    TypeFile(const TypeFile &copy);

    uint64_t size() const { return m_size; }
    time_t modificationDate() const { return m_modif_date; }

    void setSize(uint64_t size) { m_size = size; }
    void setModificationDate(time_t modif_date) { m_modif_date = modif_date; }

    LIBMTP_file_t *toLIBMTPFile() const;
    TypeFile &operator =(const TypeFile &rhs);

    bool operator ==(const std::string &rhs) const { return TypeBasic::operator ==(rhs); }
    bool operator ==(const TypeFile &rhs) const { return TypeBasic::operator ==(rhs); }
    bool operator <(const std::string &rhs) const { return TypeBasic::operator <(rhs); }
    bool operator <(const TypeFile &rhs) const { return TypeBasic::operator <(rhs); }

private:
    uint64_t m_size;
    time_t m_modif_date;
};

#endif // SMTPFS_TYPE_FILE_H
