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

#ifndef SMTPFS_TYPE_BASIC
#define SMTPFS_TYPE_BASIC

#include <string>
#include <cstdint>

class TypeBasic
{
public:
    TypeBasic():
        m_id(0),
        m_parent_id(0),
        m_storage_id(0),
        m_name()
    {}

    TypeBasic(uint32_t id, uint32_t parent_id, uint32_t storage_id,
        const std::string &name):
        m_id(id),
        m_parent_id(parent_id),
        m_storage_id(storage_id),
        m_name(name)
    {}

    TypeBasic(const TypeBasic &copy):
        m_id(copy.m_id),
        m_parent_id(copy.m_parent_id),
        m_storage_id(copy.m_storage_id),
        m_name(copy.m_name)
    {}

    uint32_t id() const { return m_id; }
    uint32_t parentid() const { return m_parent_id; }
    uint32_t storageid() const { return m_storage_id; }
    std::string name() const { return m_name; }

    void setId(uint32_t id) { m_id = id; }
    void setParent(uint32_t parent_id) { m_parent_id = parent_id; }
    void setStorage(uint32_t storage_id) { m_storage_id = storage_id; }
    void setName(const std::string &name) { m_name = name; }

    TypeBasic &operator= (const TypeBasic &rhs)
    {
        m_id = rhs.m_id;
        m_parent_id = rhs.m_parent_id;
        m_storage_id = rhs.m_storage_id;
        m_name = rhs.m_name;
        return *this;
    }

    bool operator ==(const std::string &rhs) const { return m_name == rhs; }
    bool operator ==(const TypeBasic &rhs) const { return m_name == rhs.m_name; }
    bool operator <(const std::string &rhs) const { return m_name < rhs; }
    bool operator <(const TypeBasic &rhs) const { return m_name < rhs.m_name; }

protected:
    uint32_t m_id;
    uint32_t m_parent_id;
    uint32_t m_storage_id;
    std::string m_name;
};

#endif // SMTPFS_TYPE_BASIC
