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
#include <iostream>
#include "simple-mtpfs-fuse.h"

int main(int argc, char **argv)
{
    SMTPFileSystem *filesystem = SMTPFileSystem::createInstance(argc, argv);
    if (!filesystem->good()) {
        std::cout << "Wrong usage! See `" << smtpfs_basename(argv[0])
            << " -h' for details\n";
        return 1;
    }
    return !filesystem->exec();
}
