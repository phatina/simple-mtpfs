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

#ifndef SMTPFS_LOG_H
#define SMTPFS_LOG_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <mutex>

class  Logger;
extern Logger logmsg;
extern Logger logerr;
extern Logger logdebug;

class Logger
{
public:
    Logger(const std::string &fixed_message = std::string()):
        m_fixed_message(fixed_message),
        m_verbose(false)
    {
    }

    template <typename t, typename ...p>
    void output(t data, p ...args)
    {
        if (!s_verbose && !m_verbose)
            return;
        criticalEnter();
        std::cout << timestamp();
        if (!m_fixed_message.empty())
            std::cout << ": " << m_fixed_message;
        std::cout << ": ";
        outputHelper(data, args...);
        criticalLeave();
    }

    template<typename t, typename ...p>
    void operator() (t data, p ...args)
    {
        output(data, args...);
    }

    void setVerbose(bool v = true) { m_verbose = v; }
    static void setGlobalVerbose(bool v = true) { s_verbose = v; }

private:
    static std::string timestamp();
    static void criticalEnter() { s_output_mutex.lock(); }
    static void criticalLeave() { s_output_mutex.unlock(); }

    void outputHelper() { }

    template <typename T, typename ...P>
    void outputHelper(T t, P ...p)
    {
        std::cout << t;
        if (sizeof...(p))
            outputHelper(p...);
    }

    static bool s_verbose;
    static std::mutex s_output_mutex;
    std::string m_fixed_message;
    bool m_verbose;
};

#endif // SMTPFS_LOG_H
