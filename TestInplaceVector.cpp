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
#include <memory>
#include <print>
#include <ranges>
#include <string>
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

class M // non-trival object for testing
{
public:
  M() : M( "Initialized", 42, 123.456f )
    /*s_ { "Initialized" },
    v_{ 42, 42 },
    p_{ new float{ 123.456f } } */
  {
  }

  M( const std::string& s, int i, float f ) :
    s_{ s },
    v_{ i, i },
    p_{ new float{ f } }
  {
  }


  ~M()
  {
    s_ = "Destroyed";
    v_.assign( 3, 0xDEADBEEF );
    p_.reset( new float{ 654.321f } );
  }

  void set( const std::string& s, int i, float f )
  {
    s_ = s;
    v_.assign( 2, i );
    *p_ = f;
  }

  std::string getStr() const
  {
    return s_;
  }

  M( const M& ) = default;
  M( M&& ) = default;
  M& operator=( const M& ) = default;
  M& operator=( M&& ) = default;

  bool operator==( const M& rhs ) const
  {
    return s_ == rhs.s_ &&
           v_ == rhs.v_ &&
          *p_ == *rhs.p_;
  }

private:
  std::string s_;
  std::vector<int> v_;
  std::shared_ptr<float> p_;
};

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

    inplace_vector<M, 10> ivM;
    test( ivM.empty() );
    test( ivM.size() == 0 );
    test( ivM.capacity() == 10 );
    test( ivM.max_size() == 10 );
    ivM.reserve( 5 );
    test( ivM.capacity() == 10 );
    ivM.shrink_to_fit();
    test( ivM.capacity() == 10 );
    test( ivM.size() == 0 );
  }

  // count ctor, front, back, array access
  {
    inplace_vector<int, 4> iv(3);
    test( iv.size() == 3 );
    test( iv.capacity() == 4 );
    test( iv.front() == 0 );
    test( iv.back() == 0 );
    test( iv[ 1 ] == 0 );

    inplace_vector<M, 4> ivM( 3 );
    test( ivM.size() == 3 );
    test( ivM.capacity() == 4 );
    test( ivM.front() == M{} );
    test( ivM.back() == M{} );
    test( ivM[ 1 ] == M{} );
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

    inplace_vector<M, 4> ivM( 3 );
    inplace_vector<M, 4> ivM2( std::move( ivM ) );
#pragma warning(push)
#pragma warning(disable: 26800) // use of a moved object
    test( ivM.empty() );
#pragma warning(pop)
    test( ivM != ivM2 );
    test( ivM2.size() == 3 );
    test( ivM2.capacity() == 4 );
    test( ivM2.front().getStr() == "Initialized" );
    test( ivM2.back().getStr() == "Initialized" );
    test( ivM2[ 1 ] == M{} );
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

  // copy assignment
  {
    inplace_vector<M, 10> ivEmpty;
    inplace_vector<M, 10> ivM1;
    inplace_vector<M, 10> ivM2{ 10, M{ "copied from", 123, 0.11f } };
    test( ivM1 != ivM2 );
    ivM1 = ivM2;
    test( ivM1 == ivM2 );
    ivM2 = ivEmpty;
    test( ivM2 == ivEmpty );
  }

  // move assignment
  {
    inplace_vector<M, 4> ivA{ 2 };
    inplace_vector<M, 4> ivB{ 2, M{ "iv", 321, 0.22f } };
    test( ivA[ 1 ].getStr() == "Initialized" );
    test( ivB[ 1 ].getStr() == "iv" );
    ivA = inplace_vector<M, 4>(); // move assignment from temp
    test( ivA.empty() );
    ivA = std::move( ivB ); // move assignment w/ std::move
    test( ivA.size() == 2 );
    test( ivA[ 1 ].getStr() == "iv" );
    test( ivB.empty() );
  }


}

///////////////////////////////////////////////////////////////////////////////
