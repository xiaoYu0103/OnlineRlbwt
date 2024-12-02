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

    parser.parse_check(argc, argv);
    const std::string in = parser.get<std::string>("input");
    std::ifstream ifs(in);

    const size_t step = 1000000;
    size_t last_step = 0;

    using BTreeNodeT = BTreeNode<32>;
    using BtmNodeMT = BtmNodeM_StepCode<BTreeNodeT, 32>;
    using BtmMInfoT = BtmMInfo_BlockVec<BtmNodeMT, 512>;
    using BtmNodeST = BtmNodeS<BTreeNodeT, uint32_t, 8>;
    using BtmSInfoT = BtmSInfo_BlockVec<BtmNodeST, 1024>;
    using RynRleT = DynRleForRlbwt<WBitsBlockVec<1024>, Samples_Null, BtmMInfoT, BtmSInfoT>;
    OnlineRlbwt<RynRleT> rlbwt(1);

    char c; // assume that the input character fits in char.
    unsigned char uc;
    while (ifs.peek() != std::ios::traits_type::eof())
    {
        ifs.get(c);
        uc = static_cast<unsigned char>(c);
        if (uc == 10) // if c == '\n' means is a end of current string
        {
            uc = 0;
        }
        rlbwt.extend(uint8_t(uc));
        rlbwt.printDetailInfo();
    }
}