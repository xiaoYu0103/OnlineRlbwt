#include <stdint.h>

#include <time.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sys/resource.h>
#include <sys/time.h>

#include "cmdline.h"
#include "OnlineRlbwt.hpp"
#include "DynRleForRlbwt.hpp"
#include "IOutils.hpp"

using namespace itmmti;
using SizeT = uint64_t;

long getPeakRSS()
{
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);

    return rusage.ru_maxrss; // 返回 KB
}

long getCurrentRSS()
{
    long rss = 0L;
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == nullptr)
        return rss;
    char line[128];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            sscanf(line + 6, "%ld", &rss);
            break;
        }
    }
    fclose(fp);
    return rss; // 返回 KB
}

int main(int argc, char *argv[])
{

    cmdline::parser parser;
    parser.add<std::string>("input", 'i', "input file name", true);
    parser.add<std::string>("output", 'o', "output file name BWT", false);

    parser.parse_check(argc, argv);
    const std::string in = parser.get<std::string>("input");
    const std::string out = parser.get<std::string>("output");

    auto t1 = std::chrono::high_resolution_clock::now();

    using BTreeNodeT = BTreeNode<32>;
    using BtmNodeMT = BtmNodeM_StepCode<BTreeNodeT, 32>;
    using BtmMInfoT = BtmMInfo_BlockVec<BtmNodeMT, 512>;
    using BtmNodeST = BtmNodeS<BTreeNodeT, uint32_t, 8>;
    using BtmSInfoT = BtmSInfo_BlockVec<BtmNodeST, 1024>;
    using RynRleT = DynRleForRlbwt<WBitsBlockVec<1024>, Samples_Null, BtmMInfoT, BtmSInfoT>;
    OnlineRlbwt<RynRleT> rlbwt(1);

    std::vector<char> Text;
    uint64_t n = 0, ns = 0;

    load_fasta(in, Text, n, ns);
    auto start = std::chrono::steady_clock::now();
    uint64_t cur_ns, cur_n = 0;
    // std::cout << cur_ns;
    for (unsigned char c : Text)
    {
        rlbwt.optExtend(uint8_t(c));
        cur_n++;
        if (c == '\x01')
        {
            cur_ns++;
            // std::cout << cur_ns;
            if (cur_ns % 1000 == 0)
            {
                auto end = std::chrono::steady_clock::now();
                std::cout << "===================extend over=======================" << std::endl;
                std::cout << "cur_ns:" << cur_ns << "  cur_n:" << cur_n << std::endl;
                std::cout << "Elapsed time in seconds: "
                          << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                          << " milliseconds" << std::endl;
                // long peakRSS = getPeakRSS();
                // std::cout << "Peak RSS: " << peakRSS << " KB" << "--Curr RSS: " << getCurrentRSS() << " KB" << std::endl;

                // start = std::chrono::steady_clock::now();
                // break;
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "===================extend over=======================" << std::endl;
    std::cout << "cur_ns:" << cur_ns << "  cur_n:" << cur_n << std::endl;
    std::cout << "Elapsed time in seconds: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " milliseconds" << std::endl;
    rlbwt.printStatistics(std::cout, true);
    // 判断输出文件是否被提供
    if (!out.empty())
    {
        writeTextToFile(out, Text);
    }
}