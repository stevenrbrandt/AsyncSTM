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

int main()
{
    { // Read A, Write A
        shared_var<int> A(1);

        ASTM_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            ASTM_TEST(A_ == 1); 
            A_ = 2; 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 2); 
    }

    { // Write A, Read A
        shared_var<int> A(1);

        ASTM_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            A_ = 2; 
            ASTM_TEST(A_ == 2); 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 2); 
    }

    { // Write A, Write A
        shared_var<int> A(1);

        ASTM_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            ASTM_TEST(A_ == 1); 
            ASTM_TEST(A_ == 1); 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 1); 
    }

    { // Write A, Write A
        shared_var<int> A(1);

        ASTM_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            A_ = 2; 
            A_ = 2; 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 2); 
    }

    { // Read A, Write A, Read A
        shared_var<int> A(1);

        ASTM_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            ASTM_TEST(A_ == 1); 
            A_ = 2; 
            ASTM_TEST(A_ == 2); 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 2); 
    }

    { // Write A, Read A, Write A
        shared_var<int> A(1);

        ASTM_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            A_ = 2; 
            ASTM_TEST(A_ == 2); 
            A_ = 2; 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 2); 
    }

    { // Basic arithmetic with local_vars.
        shared_var<int> A(4);
        shared_var<int> B(1);

        // atomic { A = A*A - B; }
        transaction t;
        do {
            auto A_ = A.get_local(t);
            auto B_ = B.get_local(t);

            A_ = A_*A_ - B_;
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 15);
        ASTM_TEST(B.read() == 1);
    }

    { // Read A to future.
        shared_var<int> A(4);
        shared_var<int> B(1);

        // future<void> IO;
        // atomic {
        //     A = A*A;
        //     IO = async { std::cout << A << "\n"; }
        //     A = A - B;
        // }
        transaction t;
        transaction_future IO(t);

        do {
            auto A_ = A.get_local(t);
            auto B_ = B.get_local(t);

            A_ = A_*A_;

            IO.then(
                [local_A = int(A_)] (transaction*) { ASTM_TEST(local_A == 16); }
            );

            A_ = A_ - B_;
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 15);
        ASTM_TEST(B.read() == 1);

        IO.get();
    }

    {
        shared_var<int> A(4);

        // atomic { A = A*A; }
        bool fail = true;
        int attempt_count = 0;
        transaction t;
        do {
            ++attempt_count;

            auto A_ = A.get_local(t);

            int tmp = A_*A_;

            if (fail)
            {
                A.write(3);
                fail = false;
            }

            A_ = tmp; 
        } while (!t.commit_transaction());

        ASTM_TEST(A.read() == 9);
        ASTM_TEST(attempt_count == 2);
    }

    return ASTM_REPORT;
}
