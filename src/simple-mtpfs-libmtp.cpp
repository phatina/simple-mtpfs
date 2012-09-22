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
#include <cstdio>
#include <cstdlib>
#include "simple-mtpfs-libmtp.h"

extern "C" {

void LIBMTP_Free_Files_And_Folders(LIBMTP_file_t **files)
{
    if (!files || !*files)
        return;

    LIBMTP_file_t *f = *files;
    while (f) {
        LIBMTP_file_t *tmp = f;
        char *filename = f->filename;
        f = f->next;
        free(static_cast<void*>(filename));
        free(static_cast<void*>(tmp));
    }
    *files = nullptr;
}

}
