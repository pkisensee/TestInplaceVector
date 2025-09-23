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
  inplace_vector<int,1> v;
  test( v.unchecked_emplace_back( 42 ) == 42 );
  test( v.back() == 42 );
}

///////////////////////////////////////////////////////////////////////////////
