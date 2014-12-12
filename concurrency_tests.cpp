////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2014 Bryce Adelstein-Lelbach
//  Copyright (c) 2014 Steve Brandt 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "astm.hpp"

#ifdef ASTM_HPX
    #include <hpx/hpx_main.hpp>
#endif

using namespace astm;

void cubeA(shared_var<int>& A)
{
    transaction t;
    do {
        auto A_ = A.get_local(t);

        A_ = A_ * A_; 
    } while (!t.commit_transaction());
}

int main()
{
    {
        shared_var<int> A(1);

        std::vector<ASTM_FUTURE<void> > F;

        for (unsigned i = 0; i < 64; ++i)
            F.push_back(ASTM_ASYNC(std::bind(cubeA, std::ref(A))));

        for (ASTM_FUTURE<void>& f : F)
            f.get(); 
    }  

    return ASTM_REPORT;
}
