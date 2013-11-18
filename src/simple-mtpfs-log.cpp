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
#include <sstream>
#include <ctime>
#include "simple-mtpfs-log.h"

Logger logmsg;
Logger logerr("Error");
Logger logdebug("Debug");

bool Logger::s_verbose;
std::mutex Logger::s_output_mutex;

std::string Logger::timestamp()
{
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    std::stringstream ss;
    ss  << std::setfill('0') << std::setw(4) << time_info->tm_year + 1900
        << '/' << std::setw(2) << time_info->tm_mon
        << '/' << std::setw(2) << time_info->tm_mday
        << ' ' << std::setw(2) << time_info->tm_hour
        << ':' << std::setw(2) << time_info->tm_min
        << ':' << std::setw(2) << time_info->tm_sec;
    return ss.str();
}
