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
    test( iv.empty() );
    test( iv != iv2 );
    test( iv2.size() == 3 );
    test( iv2.capacity() == 4 );
    test( iv2.front() == 42 );
    test( iv2.back() == 42 );
    test( iv2[ 1 ] == 42 );

    inplace_vector<M, 4> ivM( 3 );
    inplace_vector<M, 4> ivM2( std::move( ivM ) );
    test( ivM.empty() );
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

  // assign() count and value
  {
    inplace_vector<M, 4> ivA{ 2 };
    test( ivA[ 1 ].getStr() == "Initialized" );
    M m{ "m", 1, 2.0f };

    ivA.assign( 1, m );
    test( ivA.size() == 1 );
    test( ivA[ 0 ].getStr() == "m" );

    ivA.assign( 3, m );
    test( ivA.size() == 3 );
    test( ivA[ 0 ].getStr() == "m" );
    test( ivA[ 1 ].getStr() == "m" );
    test( ivA[ 2 ].getStr() == "m" );
  }

  // assign() iterator range
  {
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv;
    test( iv.size() == 0 );

    iv.assign( std::begin( init ), std::end( init ) );
    test( iv.size() == 3 );
    test( iv[ 0 ] == 1 );
    test( iv[ 1 ] == 2 );
    test( iv[ 2 ] == 3 );
  }

  // assign() init list
  {
    const auto init = { M("a", 1, 2.0f), M("b", 3, 4.0f), M("c", 5, 6.0f) };
    inplace_vector<M, 4> iv;
    test( iv.size() == 0 );

    iv.assign( init );
    test( iv.size() == 3 );
    test( iv[ 0 ].getStr() == "a" );
    test( iv[ 1 ].getStr() == "b" );
    test( iv[ 2 ].getStr() == "c" );
  }

  // assign() range
  {
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv;
    test( iv.size() == 0 );

    iv.assign_range( init );
    test( iv.size() == 3 );
    test( iv.front() == 1 );
    test( iv.back() == 3 );
  }

  // at
  {
    using ivM5 = inplace_vector<M, 5>;

    auto constAt = []( const ivM5& iv, size_t pos )
      {
        return iv.at( pos );
      };

    ivM5 iv{ 3 };
    test( iv.at( 0 ).getStr() == "Initialized" );
    test( constAt( iv, 1 ).getStr() == "Initialized" );

    try
    {
      iv.at( 3 ); // non-const
    }
    catch ( const std::out_of_range& outOfRange )
    {
      test( outOfRange.what() == "inplace_vector::at"s );
    }
    try
    {
      constAt(iv, 3); // const
    }
    catch ( const std::out_of_range& outOfRange )
    {
      test( outOfRange.what() == "inplace_vector::at"s );
    }
  }

  // operator[]
  {
    using ivM5 = inplace_vector<M, 5>;

    auto constOpBracket = []( const ivM5& iv, size_t pos )
      {
        return iv[ pos ];
      };

    ivM5 iv{ 3 };
    test( iv[0].getStr() == "Initialized");
    test( constOpBracket( iv, 1 ).getStr() == "Initialized" );

    // iv[3]; // asserts
    // constOpBracket( iv, 3 ); // asserts
  }

  // front(), back()
  {
    using ivM5 = inplace_vector<M, 5>;
    ivM5 iv;

    auto constFront = []( const ivM5& iv )
      {
        return iv.front();
      };

    auto constBack = []( const ivM5& iv )
      {
        return iv.back();
      };

    M empty;
    // test( iv.front() == M{} ); // asserts
    // test( iv.back() == M{} ); // asserts
    iv.assign( 1, empty );
    test( iv.front() == empty );
    test( iv.back() == empty );
    test( iv.front() == iv.back() );
    test( constFront( iv ) == constBack( iv ) );

    M m2{ "b", 1, 2.0f };
    iv.emplace( std::end(iv), m2);
    test( iv.front() == empty );
    test( constFront( iv ) == empty );
    test( constBack( iv ) == m2 );
    test( iv.back() == m2 );
    test( iv.front() != iv.back() );
    test( constFront( iv ) != constBack( iv ) );
  }

  // data()
  {
    inplace_vector<int, 1> empty;
    // test( empty.data() != nullptr ); // asserts

    const int arr[ 3 ] = { 1, 2, 3 };
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv( init );
    test( iv.size() == 3 );

    test( iv.data() != nullptr );
    test( iv.data() != arr );

    // memory layout is equivalent
    test( memcmp( iv.data(), arr, sizeof( int ) * 3 ) == 0 );

    auto constData = []( const inplace_vector<int, 4>& iv )
      {
        return iv.data();
      };

    test( constData(iv) != nullptr );
    test( constData( iv ) != arr );

    // memory layout is equivalent
    test( memcmp( constData( iv ), arr, sizeof( int ) * 3 ) == 0 );
  }

  // Iterators
  {
    const auto init = { 1.0, 2.0, 3.0 };
    inplace_vector<double, 4> iv( init );

    test( iv.begin() == iv.cbegin() );
    test( *iv.begin() == 1.0 );
    test( *iv.cbegin() == 1.0 );

    test( iv.end() == iv.cend() );
    test( *( iv.end() - 1 ) == 3.0 );
    test( *( iv.cend() - 1 ) == 3.0 );

    test( iv.rbegin() == iv.crbegin() );
    test( *iv.rbegin() == 3.0 );
    test( *iv.crbegin() == 3.0 );

    test( iv.rend() == iv.crend() );
    test( *( iv.rend() - 1 ) == 1.0 );
    test( *( iv.crend() - 1 ) == 1.0 );

    auto constBegin = []( const inplace_vector<double, 4>& iv )
      {
        return iv.begin();
      };

    auto constEnd = []( const inplace_vector<double, 4>& iv )
      {
        return iv.end();
      };

    auto constRBegin = []( const inplace_vector<double, 4>& iv )
      {
        return iv.rbegin();
      };

    auto constREnd = []( const inplace_vector<double, 4>& iv )
      {
        return iv.rend();
      };

    test( constBegin(iv) == iv.cbegin() );
    test( *constBegin(iv) == 1.0 );
    test( *constBegin(iv) == 1.0 );

    test( constEnd(iv) == iv.cend() );
    test( *( constEnd( iv ) - 1 ) == 3.0 );
    test( *( constEnd( iv ) - 1 ) == 3.0 );

    test( constRBegin( iv ) == iv.crbegin() );
    test( *constRBegin( iv ) == 3.0 );
    test( *constRBegin( iv ) == 3.0 );

    test( constREnd( iv ) == iv.crend() );
    test( *( constREnd( iv ) - 1 ) == 1.0 );
    test( *( constREnd( iv ) - 1 ) == 1.0 );
  }

  // resize()
  {
    using ivM5 = inplace_vector<M, 10>;
    ivM5 iv;
    test( iv.size() == 0 );
    iv.resize( 0 );
    test( iv.size() == 0 );
    iv.resize( 1 );
    test( iv.size() == 1 );
    test( iv.front().getStr() == "Initialized" );
    iv.resize( 0 );
    test( iv.size() == 0 );
    iv.resize( 5 );
    test( iv[4].getStr() == "Initialized" );

    M mA{ "a", 0, 1.0f };
    iv.resize( 6, mA );
    test( iv.size() == 6 );
    test( iv[ 4 ].getStr() == "Initialized" );
    test( iv[ 5 ].getStr() == "a" );
    iv.resize( 5, mA );
    test( iv.size() == 5 );
    test( iv[ 4 ].getStr() == "Initialized" );
    iv.resize( 4 );
    iv.resize( 10, mA );
    test( iv.size() == 10 );
    test( iv[ 3 ].getStr() == "Initialized" );
    for ( size_t i = 4; i < 10; ++i )
      test( iv[ i ].getStr() == "a" );

    try
    {
      iv.resize( 10+1 );
    }
    catch ( const std::bad_alloc& badAlloc )
    {
      test( badAlloc.what() == "bad allocation"s );
    }
    try
    {
      iv.resize( 10+1, mA );
    }
    catch ( const std::bad_alloc& badAlloc )
    {
      test( badAlloc.what() == "bad allocation"s );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
