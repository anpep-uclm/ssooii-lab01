//
// backupd -- Performs a copy of the source tree every minute
// Copyright (c) Angel <angel@ttm.sh>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ftw.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <thread>

using namespace std::chrono_literals;

//! Destination path for the backup
static const std::string k_backupPath { "../backup/" };
//! Number of concurrent descriptors for ftw(2)
static constexpr auto k_ftwDescriptorCount { 16 };
//! Backup interval
static constexpr auto k_backupInterval { 1min };

/**
 * Callback function that performs a copy of a filesystem node
 * @param rawPath Node path
 * @param st Struct with node information
 * @param type Node type
 * @return 0 on success or a negative value on failure
 */
static int backupNode(const char *rawPath, const struct stat *st, int type)
{
    const std::string path { rawPath };
    std::cout << "backing up: " << path << std::endl;

    if (type == FTW_D) {
        // backup directory
        if (path == ".")
            mkdir(k_backupPath.c_str(), st->st_mode);
        else
            mkdir((k_backupPath + path).c_str(), st->st_mode);
    } else if (type == FTW_F) {
        // backup regular file
        const std::ifstream srcStream { path, std::ios::binary };
        std::ofstream destStream { (k_backupPath + path), std::ios::binary };
        destStream << srcStream.rdbuf();
    } else {
        std::cerr << "ignoring unknown entry type" << std::endl;
    }
    return 0;
}

/**
 * Program entry point
 * @return EXIT_SUCCESS on success or a non-zero value on failure
 */
int main()
{
    while (true) {
        if (ftw(".", backupNode, k_ftwDescriptorCount) < 0)
            std::cerr << "ftw() failed: " << strerror(errno) << std::endl;

        const auto now = std::chrono::steady_clock::now();
        std::this_thread::sleep_until(now + k_backupInterval);
    }

    return EXIT_SUCCESS;
}