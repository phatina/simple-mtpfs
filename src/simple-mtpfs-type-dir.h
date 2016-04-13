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

#ifndef SMTPFS_TYPE_DIR_H
#define SMTPFS_TYPE_DIR_H

#include <mutex>
#include <set>
#include <string>

#include "simple-mtpfs-type-basic.h"
#include "simple-mtpfs-type-file.h"

class TypeDir: public TypeBasic
{
public:
    TypeDir();
    TypeDir(uint32_t id, uint32_t parent_id, uint32_t storage_id,
        const std::string &name);
    TypeDir(LIBMTP_file_t *file);
    TypeDir(const TypeDir &copy);

    void enterCritical() const { m_access_mutex.lock(); }
    void leaveCritical() const { m_access_mutex.unlock(); }

    void clear() { m_dirs.clear(); m_files.clear(); }
    void setFetched(bool f = true) { m_fetched = f; }
    bool isFetched() const { return m_fetched; }
    void addDir(const TypeDir &dir);
    void addFile(const TypeFile &file);
    bool removeDir(const TypeDir &dir);
    bool removeFile(const TypeFile &file);
    bool replaceFile(const TypeFile &oldfile, const TypeFile &newfile);

    std::set<TypeDir>::size_type dirCount() const { return m_dirs.size(); }
    std::set<TypeFile>::size_type fileCount() const { return m_files.size(); }
    const TypeDir  *dir(const std::string &name) const;
    const TypeFile *file(const std::string &name) const;
    std::set<TypeDir> dirs() const { return m_dirs; }
    std::set<TypeFile> files() const { return m_files; }
    bool isEmpty() const { return m_dirs.empty() && m_files.empty(); }

	time_t modificationDate() const { return m_modif_date; }
    void setModificationDate(time_t modif_date) { m_modif_date = modif_date; }
	
    LIBMTP_folder_t *toLIBMTPFolder() const;
    TypeDir &operator =(const TypeDir &rhs);
    bool operator ==(const std::string &rhs) const { return TypeBasic::operator ==(rhs); }
    bool operator ==(const TypeDir &rhs) const { return TypeBasic::operator ==(rhs); }
    bool operator <(const std::string &rhs) const { return TypeBasic::operator <(rhs); }
    bool operator <(const TypeDir &rhs) const { return TypeBasic::operator <(rhs); }

private:
    std::set<TypeDir> m_dirs;
    std::set<TypeFile> m_files;
    mutable std::mutex m_access_mutex;
    bool m_fetched;
    time_t m_modif_date;
};

#endif // SMTPFS_TYPE_DIR_H
