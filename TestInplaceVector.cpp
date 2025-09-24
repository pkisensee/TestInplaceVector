///////////////////////////////////////////////////////////////////////////////
//
//  TestInplaceVector.cpp
//
//  Copyright © Pete Isensee (PKIsensee@msn.com).
//  All rights reserved worldwide.
//
//  Permission to copy, modify, reproduce or redistribute this source code is
//  granted provided the above copyright notice is retained in the resulting 
//  source code.
// 
//  This software is provided "as is" and without any express or implied
//  warranties.
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <ranges>
#include <utility>
#include <vector>

#include "inplace_vector.h"
#include "Util.h"

using namespace PKIsensee;

#ifdef _DEBUG
#define test(e) assert(e)
#else
#define test(e) static_cast<void>( (e) || ( Util::DebugBreak(), 0 ) )
#endif

int __cdecl main()
{
  // default ctor, size, capacity
  {
    inplace_vector<int, 100> iv;
    test( iv.empty() );
    test( iv.size() == 0 );
    test( iv.capacity() == 100 );
    test( iv.max_size() == 100 );
    iv.reserve( 10 );
    test( iv.capacity() == 100 );
    iv.shrink_to_fit();
    test( iv.capacity() == 100 );
    test( iv.size() == 0 );
  }

  // count ctor, front, back, array access
  {
    inplace_vector<int, 4> iv(3);
    test( iv.size() == 3 );
    test( iv.capacity() == 4 );
    test( iv.front() == 0 );
    test( iv.back() == 0 );
    test( iv[ 1 ] == 0 );
  }

  // count/value ctor
  {
    inplace_vector<int, 4> iv( 3, 42 );
    test( iv.size() == 3 );
    test( iv.capacity() == 4 );
    test( iv.front() == 42 );
    test( iv.back() == 42 );
    test( iv[ 1 ] == 42 );
  }


  inplace_vector<int,1> v;
  test( v.unchecked_emplace_back( 42 ) == 42 );
  test( v.back() == 42 );
}

///////////////////////////////////////////////////////////////////////////////
