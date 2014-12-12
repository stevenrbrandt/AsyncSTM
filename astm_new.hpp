////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2014 Bryce Adelstein-Lelbach
//  Copyright (c) 2014 Steve Brandt 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#if !defined(ASTM_65EB3381_65AE_4527_A4E5_A87D5014A33E)
#define ASTM_65EB3381_65AE_4527_A4E5_A87D5014A33E

#include "astm.hpp"

namespace astm
{
    // Asynchronous object allocation  
    template <typename Var, typename T, typename... Args>
    ASTM_FUTURE<void>   
    new_(shared_var<Var>& var, T* ptr, Args&&... args) 
    {
        return var.queue.then(
            [=] (ASTM_FUTURE<void> f)
            {
                ptr = new T(std::forward<Args>(args...));
            }
        );
    }
}

#endif

