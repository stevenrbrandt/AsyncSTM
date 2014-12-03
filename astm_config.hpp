////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2014 Bryce Adelstein-Lelbach
//  Copyright (c) 2014 Steve Brandt 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#if !defined(ASTM_DBA88345_57B8_4CC8_A574_D5F007250E95)
#define ASTM_DBA88345_57B8_4CC8_A574_D5F007250E95

#if defined(ASTM_HPX) // Use hpx::
    #include <hpx/include/lcos.hpp>
    #include <hpx/async.hpp>

    #define ASTM_MUTEX hpx::util::spinlock
    #define ASTM_LOCK hpx::util::spinlock::scoped_lock
    #define ASTM_FUTURE hpx::future
    #define ASTM_FUNCTION HPX_STD_FUNCTION
    #define ASTM_ASYNC hpx::async
    #define ASTM_MAKE_READY_FUTURE hpx::make_ready_future

    #include <hpx/util/lightweight_test.hpp>

    #define ASTM_TEST HPX_TEST
    #define ASTM_REPORT hpx::util::report_errors()
#else                 // Use std::
    #include <future>
    #include <thread>

    #define ASTM_MUTEX std::mutex
    #define ASTM_LOCK std::unique_lock<std::mutex> 
    #define ASTM_FUTURE std::future
    #define ASTM_FUNCTION std::function
    #define ASTM_ASYNC std::async

    // Note: this is a hack, no make_ready_future in std::
    #define ASTM_MAKE_READY_FUTURE std::async([](){})

    #include <assert.h>

    #define ASTM_TEST assert
    #define ASTM_REPORT 0 
#endif 

#endif // ASTM_DBA88345_57B8_4CC8_A574_D5F007250E95
