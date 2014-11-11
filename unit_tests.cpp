#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>

#include "async_stm.hpp"

using namespace astm;

int main()
{
    { // Read A, Write A
        atomic<int> A(1);

        HPX_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            HPX_TEST(A_ == 1); 
            A_ = 2; 
        } while (!t.commit_transaction());

        HPX_TEST(A.read() == 2); 
    }

    { // Write A, Read A
        atomic<int> A(1);

        HPX_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            A_ = 2; 
            HPX_TEST(A_ == 2); 
        } while (!t.commit_transaction());

        HPX_TEST(A.read() == 2); 
    }

    { // Write A, Write A
        atomic<int> A(1);

        HPX_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            HPX_TEST(A_ == 1); 
            HPX_TEST(A_ == 1); 
        } while (!t.commit_transaction());

        HPX_TEST(A.read() == 1); 
    }

    { // Write A, Write A
        atomic<int> A(1);

        HPX_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            A_ = 2; 
            A_ = 2; 
        } while (!t.commit_transaction());

        HPX_TEST(A.read() == 2); 
    }

    { // Read A, Write A, Read A
        atomic<int> A(1);

        HPX_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            HPX_TEST(A_ == 1); 
            A_ = 2; 
            HPX_TEST(A_ == 2); 
        } while (!t.commit_transaction());

        HPX_TEST(A.read() == 2); 
    }

    { // Write A, Read A, Write A
        atomic<int> A(1);

        HPX_TEST(A.read() == 1); 

        transaction t;
        do {
            auto A_ = A.get_local(t);

            A_ = 2; 
            HPX_TEST(A_ == 2); 
            A_ = 2; 
        } while (!t.commit_transaction());

        HPX_TEST(A.read() == 2); 
    }

    { // Basic arithmetic with local atomics.
        atomic<int> A(4);
        atomic<int> B(1);

        // atomic { A = A*A - B; }
        transaction t;
        do {
            auto A_ = A.get_local(t);
            auto B_ = B.get_local(t);

            A_ = A_*A_ - B_;
        } while (!t.commit_transaction());

        std::cout << "A = " << A.read() << "\n"
                  << "B = " << B.read() << "\n"; 
    }

    { // Read A to future.
        atomic<int> A(4);
        atomic<int> B(1);
        hpx::future<void> IO;

        // future<void> IO;
        // atomic {
        //     A = A*A;
        //     IO = async { std::cout << A << "\n"; }
        //     A = A - B;
        // }
        transaction t;
        do {
            auto A_ = A.get_local(t);
            auto B_ = B.get_local(t);

            A_ = A_*A_;

            t.async(&IO,
                [local_A = int(A_)] (transaction*) { std::cout << local_A << "\n"; }
            );

            A_ = A_ - B_;
        } while (!t.commit_transaction());

        std::cout << "A = " << A.read() << "\n"
                  << "B = " << B.read() << "\n"; 

        IO.get();
    }

    {
        atomic<int> A(4);

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

        std::cout << "A = " << A.read() << "\n"
                  << "Attempts: " << attempt_count << "\n"; 
    }

    { // Heat equation example (needs fixing)
        atomic<std::vector<double> > U(std::vector<double>(20, 0.0));
        double const C = 1.0;

        hpx::future<void> exchange;
        hpx::future<void> I0; 
        transaction t;
        do {
            std::vector<double> U_ = U.get_local(t);
                    
            auto Idx =
                [size = U_.size()] (int i)
                {
                    return (i < 0) ? ((i + size) % size) : i % size;
                };

            for (int i = 0; i < U_.size(); ++i)
            {
                U_[Idx(i)] = U_[Idx(i)]
                           + C * (U_[Idx(i-1)] - 2*U_[Idx(i)] + U_[Idx(i+1)]); 
            } 

            t.async(&exchange,
                [ lower_gz = double(U_[1])
                , upper_gz = double(U_[U_.size()-1]) 
                ] (transaction*)
                {
                    // ... 
                }
            );

            U.get_local(t) = U_;
        } while (!t.commit_transaction());

        exchange.get();
    }

    return 0;
}
