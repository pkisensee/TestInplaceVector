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
#include <print>
#include <ranges>
#include <utility>
#include <vector>

#include "inplace_vector.h"
#include "Util.h"

using namespace PKIsensee;
using namespace std::literals;

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

  // iterator ctor
  {
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv( init.begin(), init.end() );
    test( iv.size() == 3 );
    test( iv[ 0 ] == 1 );
    test( iv[ 1 ] == 2 );
    test( iv[ 2 ] == 3 );
  }

  // range ctor
  {
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv( std::from_range, init );
    test( iv.size() == 3 );
    test( iv.front() == 1 );
    test( iv.back() == 3 );
  }

  // copy ctor
  {
    inplace_vector<int, 4> iv( 3, 42 );
    // inplace_vector<int, 5> ivx( iv ); // compiler error
    inplace_vector<int, 4> iv2( iv );
    test( iv == iv2 );
  }

  // move ctor
  {
    inplace_vector<int, 4> iv( 3, 42 );
    inplace_vector<int, 4> iv2( std::move(iv) );
#pragma warning(push)
#pragma warning(disable: 26800) // use of a moved object
    test( iv.empty() );
#pragma warning(pop)
    test( iv != iv2 );
    test( iv2.size() == 3 );
    test( iv2.capacity() == 4 );
    test( iv2.front() == 42 );
    test( iv2.back() == 42 );
    test( iv2[ 1 ] == 42 );
  }

  // init list ctor
  {
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv( init );
    test( iv.size() == 3 );
    test( iv.front() == 1 );
    test( iv.back() == 3 );
    try
    {
      inplace_vector<int, 2> ivx( init ); // throws; ivx too small
    }
    catch ( const std::bad_alloc& badAlloc )
    {
      test( badAlloc.what() == "bad allocation"s );
    }

  }
}

///////////////////////////////////////////////////////////////////////////////
