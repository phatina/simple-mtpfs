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

#include <config.h>
#include <algorithm>
#include <sstream>
#include "simple-mtpfs-tmp-files-pool.h"
#include "simple-mtpfs-sha1.h"
#include "simple-mtpfs-util.h"

TmpFilesPool::TmpFilesPool():
    m_tmp_dir(smtpfs_get_tmpdir()),
    m_pool()
{
}

TmpFilesPool::~TmpFilesPool()
{
}

void TmpFilesPool::removeFile(int desc)
{
    auto it = std::find(m_pool.begin(), m_pool.end(), desc);
    if (it == m_pool.end())
        return;
    m_pool.erase(it);
}

const TypeTmpFile *TmpFilesPool::getFile(int desc) const
{
    auto it = std::find(m_pool.begin(), m_pool.end(), desc);
    if (it == m_pool.end())
        return nullptr;
    return static_cast<const TypeTmpFile*>(&*it);
}

std::string TmpFilesPool::makeTmpPath(const std::string &path_device) const
{
    static int cnt = 0;
    std::stringstream ss;
    ss << path_device << ++cnt;
    return m_tmp_dir + std::string("/") + SHA1::sumString(ss.str());
}

bool TmpFilesPool::createTmpDir()
{
    removeTmpDir();
    return smtpfs_create_dir(m_tmp_dir);
}

bool TmpFilesPool::removeTmpDir()
{
    if (!m_tmp_dir.empty())
        return smtpfs_remove_dir(m_tmp_dir);
    return false;
}
