#include <config.h>
#include <iostream>
#include <list>
#include <string>
#include <cstdlib>
extern "C" {
#  include <getopt.h>
}
#include "simple-mtpfs-mtp-device.h"

namespace {

class LSOptions
{
public:
    LSOptions() = delete;
    LSOptions(int argc, char **argv);

    bool good() const { return m_good; }
    bool all() const { return m_all; }
    int device() const { return m_device; }
    std::list<std::string> positionalOptions() const { return m_pos_opts; }

protected:
    bool m_good;
    bool m_all;
    int m_device;
    std::list<std::string> m_pos_opts;
};

LSOptions::LSOptions(int argc, char **argv):
    m_good(true),
    m_all(false),
    m_device(0)
{
    int c;
    int opt_ind;
    struct option long_opts[] = {
        { "all", no_argument, 0, 0 },
        { "device", required_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };
    opterr = 0;

    while ((c = getopt_long(argc, argv, "ad:", long_opts, &opt_ind)) != -1) {
        switch (c) {
        case 0:
        case 'a':
            // Fall through, -a --all
            m_all = true;
            break;
        case 1:
        case 'd': {
            // Fall through, -d, --device
            char *end = nullptr;
            m_device = static_cast<int>(strtol(optarg, &end, 10));
            m_good = end[0] == '\0';
            break;
        }
        case '?':
            m_good = false;
            break;
        }
    }

    for (int i = optind; i < argc; ++i)
        m_pos_opts.push_back(std::string(argv[i]));
}

void print_entry(const TypeDir* dir)
{
}

}

int main(int argc, char **argv)
{
    LSOptions o(argc, argv);
    std::cout << "Good: " << o.good() << "\n";
    std::cout << "All:  " << o.all() << "\n";
    std::cout << "Dev:  " << o.device() << "\n";

    std::cout << "---\n";

    MTPDevice device;
    if (!device.connect(o.device())) {
        std::cerr << "Unable to connect to specified MTP device!\n";
        return 1;
    }

    for (std::string &i : o.positionalOptions()) {
        const TypeDir *dir = device.dirFetchContent(i);
    }

    return 0;
}
