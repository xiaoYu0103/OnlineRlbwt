#include <stdint.h>

#include <time.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#include "cmdline.h"
#include "OnlineRlbwt.hpp"
#include "DynRleForRlbwt.hpp"

using namespace itmmti;
using SizeT = uint64_t;

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

    std::ifstream ifs(in);
    char c; // assume that the input character fits in char.
    unsigned char uc;

    while (ifs.peek() != std::ios::traits_type::eof())
    {
        ifs.get(c);
        uc = static_cast<unsigned char>(c);

        if (uc == 10) // 如果遇到换行符，替换为0
        {
            uc = 1;
        }
        // std::cout << "optExtend : " << c << std::endl;
        rlbwt.optExtend(uint8_t(uc));
        rlbwt.printDetailInfo();
    }

    ifs.close();
    {
        auto t2 = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
        std::cout << "RLBWT construction done. " << sec << " sec" << std::endl;
    }
    rlbwt.printStatistics(std::cout, false);

    if (!(out.empty()))
    {
        std::ofstream ofs(out, std::ios::out);
        rlbwt.optExtend(0);
        // rlbwt.printDetailInfo();
        rlbwt.writeBWT(ofs);
        auto t2 = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
        std::cout << "RLBWT write done. " << sec << " sec" << std::endl;
    }

    std::cout << "over" << std::endl;
}