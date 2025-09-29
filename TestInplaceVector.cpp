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
#include <numeric>
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

// clever lambda to pass any type of expression to the TryCatch helper
#define testex(e, E, msg) TryCatch<E>( ( [&]() { (e); } ), msg );

template <typename Exception, typename TryLambda>
void TryCatch( TryLambda&& tryLambda, std::string_view exceptionMsg )
{
  try
  {
    tryLambda();
  }
  catch ( Exception& ex )
  {
    test( ex.what() == exceptionMsg );
  }
  catch ( ... )
  {
    test( false );
  }
}

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

  bool operator<( const M& rhs ) const
  {
    return s_ < rhs.s_;
  }

  bool operator>( const M& rhs ) const
  {
    return s_ > rhs.s_;
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

    testex( ivM.reserve( 11 ), std::bad_alloc, "bad allocation"sv );
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
    testex( ( [&]() { inplace_vector<int, 2> ivx( init ); } ), std::bad_alloc, "bad allocation"sv );
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

  // init list assignment
  {
    const auto init = { M( "a", 1, 2.0f ), M( "b", 3, 4.0f ), M( "c", 5, 6.0f ) };
    inplace_vector<M, 4> iv;
    test( iv.size() == 0 );
    iv = init;
    test( iv.size() == 3 );
    test( iv[ 0 ].getStr() == "a" );
    test( iv[ 1 ].getStr() == "b" );
    test( iv[ 2 ].getStr() == "c" );

    iv = init;
    test( iv.size() == 3 );
    test( iv[ 2 ].getStr() == "c" );
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

    testex( iv.at( 3 ), std::out_of_range&, "inplace_vector::at"sv );
    testex( constAt( iv, 3 ), std::out_of_range&, "inplace_vector::at"sv );
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
    using ivM = inplace_vector<M, 10>;
    ivM iv;
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

    testex( iv.resize( 10 + 1 ), std::bad_alloc, "bad allocation"sv );
    testex( iv.resize( 10 + 1, mA ), std::bad_alloc, "bad allocation"sv );
  }

  // insert() and insert_range()
  {
    using ivM = inplace_vector<M, 10>;
    ivM iv;
    M mA{ "a", 0, 1.0f };
    M mB{ "b", 2, 3.0f };
    M mC{ "c", 4, 5.0f };

    test( iv.insert( iv.end(), mA )->getStr() == "a" );
    test( iv.size() == 1 );

    test( iv.insert( iv.begin(), mB )->getStr() == "b" );
    test( iv.size() == 2 );
    test( iv[ 0 ].getStr() == "b" );
    test( iv[ 1 ].getStr() == "a" );

    test( iv.insert( iv.begin() + 1, mC )->getStr() == "c" );
    test( iv[ 0 ].getStr() == "b" );
    test( iv[ 1 ].getStr() == "c" );
    test( iv[ 2 ].getStr() == "a" );

    test( iv.insert( iv.end(), mA)->getStr() == "a");
    test( iv[ 0 ].getStr() == "b" );
    test( iv[ 1 ].getStr() == "c" );
    test( iv[ 2 ].getStr() == "a" );
    test( iv[ 3 ].getStr() == "a" );

    test( iv.insert( iv.begin(), mC )->getStr() == "c" );
    test( iv[ 0 ].getStr() == "c" );
    test( iv[ 1 ].getStr() == "b" );
    test( iv[ 2 ].getStr() == "c" );
    test( iv[ 3 ].getStr() == "a" );
    test( iv[ 4 ].getStr() == "a" );

    test( iv.insert( iv.begin() + 2, M{} )->getStr() == "Initialized" ); // move
    test( iv[ 0 ].getStr() == "c" );
    test( iv[ 1 ].getStr() == "b" );
    test( iv[ 2 ].getStr() == "Initialized" );
    test( iv[ 3 ].getStr() == "c" );
    test( iv[ 4 ].getStr() == "a" );
    test( iv[ 5 ].getStr() == "a" );

    test( iv.insert( iv.begin(), 2, M{} )->getStr() == "Initialized" ); // count
    test( iv[ 0 ].getStr() == "Initialized" );
    test( iv[ 1 ].getStr() == "Initialized" );
    test( iv[ 2 ].getStr() == "c" );
    test( iv[ 3 ].getStr() == "b" );
    test( iv[ 4 ].getStr() == "Initialized" );
    test( iv[ 5 ].getStr() == "c" );
    test( iv[ 6 ].getStr() == "a" );
    test( iv[ 7 ].getStr() == "a" );

    testex( iv.insert( iv.begin(), 3, M{} ), std::bad_alloc, "bad allocation"sv );
    test( iv.size() == 8 );

    using ivInt = inplace_vector<int, 10>;
    ivInt ivI;
    const auto init = { 1, 2, 3 };
    test( *( ivI.insert( ivI.end(), init.begin(), init.end() ) ) == 1 ); // iterators
    test( ivI.size() == 3 );
    test( ivI[ 0 ] == 1 );
    test( ivI[ 1 ] == 2 );
    test( ivI[ 2 ] == 3 );
    test( *( ivI.insert( ivI.begin() + 2, init.begin(), init.end() ) ) == 1 );
    test( ivI.size() == 6 );
    test( ivI[ 0 ] == 1 );
    test( ivI[ 1 ] == 2 );
    test( ivI[ 2 ] == 1 );
    test( ivI[ 3 ] == 2 );
    test( ivI[ 4 ] == 3 );
    test( ivI[ 5 ] == 3 );

    // insert nothing
    test( ivI.insert( ivI.begin() + 1, init.end(), init.end() ) == ivI.begin() + 1 );

    // init list
    auto oldEnd = ivI.end();
    test( ivI.insert( ivI.end(), init ) == oldEnd );
    test( ivI[ 0 ] == 1 );
    test( ivI[ 1 ] == 2 );
    test( ivI[ 2 ] == 1 );
    test( ivI[ 3 ] == 2 );
    test( ivI[ 4 ] == 3 );
    test( ivI[ 5 ] == 3 );
    test( ivI[ 6 ] == 1 );
    test( ivI[ 7 ] == 2 );
    test( ivI[ 8 ] == 3 );

    testex( ivI.insert( ivI.begin(), 3, 42 ), std::bad_alloc, "bad allocation"sv );
    test( ivI.size() == 9 );

    ivI.clear();
    ivI.insert_range( ivI.end(), init ); // insert_range
    test( ivI.size() == 3 );
    test( ivI[ 2 ] = 3 );
    ivI.insert_range( ivI.begin() + 1, init );
    test( ivI.size() == 6 );
    test( ivI[ 0 ] == 1 );
    test( ivI[ 1 ] == 1 );
    test( ivI[ 2 ] == 2 );
    test( ivI[ 3 ] == 3 );
    test( ivI[ 4 ] == 2 );
    test( ivI[ 5 ] == 3 );

    oldEnd = ivI.end();
    test( ivI.insert_range( ivI.end(), init ) == oldEnd );
    test( ivI.size() == 9 );
  }

  // emplace()
  {
    using ivC = inplace_vector<char, 3>;
    ivC iv;

    test( iv.emplace( iv.end(), 'a' ) == iv.begin() );
    test( iv.size() == 1 );
    test( iv[ 0 ] == 'a' );

    auto posB = iv.emplace( iv.end(), 'b' );
    test( posB == iv.end() - 1 );
    test( posB == iv.begin() + 1 );
    test( *posB == 'b' );
    test( iv.size() == 2 );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );

    test( iv.emplace( iv.begin(), 'c' ) == iv.begin() );
    test( iv[ 0 ] == 'c' );
    test( iv[ 1 ] == 'a' );
    test( iv[ 2 ] == 'b' );

    testex( iv.emplace( iv.begin(), 'd' ), std::bad_alloc, "bad allocation"sv );
    test( iv.size() == 3 );
    test( iv[ 2 ] == 'b' );
  }

  // emplace_back()
  {
    using ivC = inplace_vector<char, 3>;
    ivC iv;

    test( iv.emplace_back( 'a' ) == 'a' );
    test( iv.size() == 1 );
    test( iv[ 0 ] == 'a' );

    test( iv.emplace_back( 'b' ) == 'b' );
    test( iv.size() == 2 );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );

    test( iv.emplace_back( 'c' ) == 'c' );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );
    test( iv[ 2 ] == 'c' );

    testex( iv.emplace_back( 'd' ), std::bad_alloc, "bad allocation"sv );
    test( iv.size() == 3 );
    test( iv[ 2 ] == 'c' );
  }

  // try_emplace_back()
  {
    using ivC = inplace_vector<char, 3>;
    ivC iv;

    test( *iv.try_emplace_back( 'a' ) == 'a' );
    test( iv.size() == 1 );
    test( iv[ 0 ] == 'a' );

    test( *iv.try_emplace_back( 'b' ) == 'b' );
    test( iv.size() == 2 );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );

    test( *iv.try_emplace_back( 'c' ) == 'c' );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );
    test( iv[ 2 ] == 'c' );

    test( iv.try_emplace_back( 'd' ) == nullptr );
  }

  // unchecked_emplace_back()
  {
    using ivC = inplace_vector<char, 3>;
    ivC iv;

    test( iv.unchecked_emplace_back( 'a' ) == 'a' );
    test( iv.size() == 1 );
    test( iv[ 0 ] == 'a' );

    test( iv.unchecked_emplace_back( 'b' ) == 'b' );
    test( iv.size() == 2 );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );

    test( iv.unchecked_emplace_back( 'c' ) == 'c' );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );
    test( iv[ 2 ] == 'c' );

    // test( iv.unchecked_emplace_back( 'd' ) == 'd' ); // assertion
  }

  // push_back() and friends
  {
    using ivC = inplace_vector<char, 3>;
    ivC iv;
    char b = 'b';
    char c = 'c';

    test( iv.push_back( 'a' ) == 'a' );
    test( *iv.try_push_back( 'b' ) == 'b' ); // move
    iv.pop_back();
    test( *iv.try_push_back( b ) == 'b' ); // const value&
    test( iv.unchecked_push_back( 'c' ) == 'c' ); // move
    iv.pop_back();
    test( iv.unchecked_push_back( c ) == 'c' ); // const value&
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'b' );
    test( iv[ 2 ] == 'c' );

    test( iv.try_push_back( 'd' ) == nullptr );
    // test( iv.unchecked_push_back( 'e' ) == 'e' ); // assertion

    testex( iv.push_back( 'f' ), std::bad_alloc, "bad allocation"sv );
    test( iv.size() == 3 );
    test( iv[ 2 ] == 'c' );
  }

  // pop_back()
  {
    using ivC = inplace_vector<char, 3>;
    ivC iv;

    // iv.pop_back(); // assertion
    test( iv.push_back( 'a' ) == 'a' );
    iv.pop_back();
    test( iv.empty() );
    test( iv.push_back( 'a' ) == 'a' );
    test( iv.push_back( 'b' ) == 'b' );
    iv.pop_back();
    test( iv.size() == 1 );
    test( iv[ 0 ] == 'a' );

    using ivM = inplace_vector<M, 2>;
    ivM ivm;

    test( ivm.push_back( M{} ) == M{} );
    ivm.pop_back();
    test( ivm.empty() );

    M mA{ "a", 0, 1.0f };
    M mB{ "b", 0, 1.0f };
    test( ivm.push_back( mA ) == mA );
    test( ivm.push_back( mB ) == mB );
    test( ivm[ 1 ] == mB );
    ivm.pop_back();
    test( ivm.size() == 1 );
    test( ivm[ 0 ] == mA );
  }

  // append_range()
  {
    const auto init = { 1, 2, 3 };
    inplace_vector<int, 4> iv;
    iv.append_range( init );
    test( iv[ 0 ] == 1 );
    test( iv[ 1 ] == 2 );
    test( iv[ 2 ] == 3 );

    testex( iv.append_range( init ), std::bad_alloc, "bad allocation"sv );
    test( iv.size() == 3 );
    test( iv[ 2 ] == 3 );

    test( iv.try_append_range( init ) == std::ranges::begin( init ) + 1 );
    test( iv.size() == 4 );
    test( iv[ 2 ] == 3 );
    test( iv[ 3 ] == 1 );

    test( iv.try_append_range( init ) == std::ranges::begin( init ) );
    test( iv.size() == 4 );

    const std::vector<int> emptyRange;
    test( iv.try_append_range( emptyRange ) == std::ranges::end( emptyRange ) );
  }

  // clear(), erase()
  {
    using ivC = inplace_vector<char, 5>;
    ivC iv;

    test( iv.push_back( 'a') == 'a' );
    iv.clear();
    test( iv.empty() );

    test( iv.push_back( 'a' ) == 'a' );
    test( iv.push_back( 'b' ) == 'b' );
    test( iv[ 1 ] == 'b' );
    iv.clear();
    test( iv.size() == 0 );

    // erase w/ pos
    test( iv.push_back( 'a' ) == 'a' );
    test( iv.push_back( 'b' ) == 'b' );
    test( iv.push_back( 'c' ) == 'c' );
    test( iv.push_back( 'd' ) == 'd' );
    test( iv.push_back( 'e' ) == 'e' );
    test( iv.size() == 5 );
    test( *iv.erase( iv.begin() ) == 'b' ); // erase a
    test( iv.size() == 4 );
    test( *iv.erase( iv.begin() + 1 ) == 'd' ); // erase c
    test( iv.size() == 3 );
    test( iv[ 0 ] == 'b' );
    test( iv[ 1 ] == 'd' );
    test( iv[ 2 ] == 'e' );
    // test( iv.erase( iv.end() ) == iv.end() ); // assertion; end() cannot be used for pos
    auto newEnd = iv.erase( iv.end() - 1 );
    test( newEnd == iv.end() );
    test( iv.size() == 2 );
    test( iv[ 0 ] == 'b' );
    test( iv[ 1 ] == 'd' );
    iv.erase( iv.begin() );
    test( iv[ 0 ] == 'd' );
    iv.erase( iv.begin() );
    test( iv.empty() );
    // iv.erase( iv.begin() ); // assertion; cannot call on empty container

    // erase w/ iterators
    test( iv.push_back( 'a' ) == 'a' );
    test( iv.push_back( 'b' ) == 'b' );
    test( iv.push_back( 'c' ) == 'c' );
    test( iv.push_back( 'd' ) == 'd' );
    test( iv.push_back( 'e' ) == 'e' );
    test( iv.size() == 5 );
    test( *iv.erase( iv.begin(), iv.begin() ) == 'a' ); // removes nothing
    test( iv.size() == 5 );
    test( *iv.erase( iv.begin() + 1, iv.begin() + 3 ) == 'd' ); // removes bc
    test( iv.size() == 3 );
    test( iv[ 0 ] == 'a' );
    test( iv[ 1 ] == 'd' );
    test( iv[ 2 ] == 'e' );
    auto result = iv.erase( iv.begin(), iv.end() );
    test( result == iv.end() );
    test( iv.empty() );

    // clear/erase w/ non-trivial elements
    using ivM = inplace_vector<M, 5>;
    ivM ivm;

    test( ivm.push_back( M{} ) == M{} );
    ivm.clear();
    test( ivm.empty() );

    const auto init = { M{ "a", 0, 1.0f },
                        M{ "b", 0, 1.0f },
                        M{ "c", 0, 1.0f },
                        M{ "d", 0, 1.0f },
                        M{ "e", 0, 1.0f } };
    ivm.append_range( init );
    ivm.clear();
    test( ivm.size() == 0 );
    ivm.append_range( init );

    test( ivm.erase( ivm.begin() )->getStr() == "b" ); // erase a
    test( ivm.size() == 4 );
    test( ivm.erase( ivm.begin() + 1 )->getStr() == "d" ); // erase c
    test( ivm.size() == 3 );
    test( ivm[ 0 ].getStr() == "b" );
    test( ivm[ 1 ].getStr() == "d" );
    test( ivm[ 2 ].getStr() == "e" );
    auto newEndm = ivm.erase( ivm.end() - 1 );
    test( newEndm == ivm.end() );
    test( ivm.size() == 2 );
    test( ivm[ 0 ].getStr() == "b" );
    test( ivm[ 1 ].getStr() == "d" );
    ivm.erase( ivm.begin() );
    test( ivm[ 0 ].getStr() == "d" );
    ivm.erase( ivm.begin() );
    test( ivm.empty() );

    // erase w/ iterators
    ivm.append_range( init );
    test( ivm.erase( ivm.begin(), ivm.begin() )->getStr() == "a" ); // removes nothing
    test( ivm.size() == 5 );
    test( ivm.erase( ivm.begin() + 1, ivm.begin() + 3 )->getStr() == "d" ); // removes bc
    test( ivm.size() == 3 );
    test( ivm[ 0 ].getStr() == "a" );
    test( ivm[ 1 ].getStr() == "d" );
    test( ivm[ 2 ].getStr() == "e" );
    auto resultm = ivm.erase( ivm.begin(), ivm.end() );
    test( resultm == ivm.end() );
    test( ivm.empty() );
  }

  // swap
  {
    using ivI = inplace_vector<int, 5>;
    ivI ivi( { 1,2,3,4,5 } );
    ivI ivi2( { 5,4,3,2,1 } );
    ivi.swap( ivi2 );
    test( ivi[ 0 ] == 5 );
    test( ivi[ 1 ] == 4 );
    test( ivi[ 2 ] == 3 );
    test( ivi[ 3 ] == 2 );
    test( ivi[ 4 ] == 1 );
    test( ivi2[ 0 ] == 1 );
    test( ivi2[ 1 ] == 2 );
    test( ivi2[ 2 ] == 3 );
    test( ivi2[ 3 ] == 4 );
    test( ivi2[ 4 ] == 5 );

    using ivM = inplace_vector<M, 2>;
    ivM ivm{ { M{ "a", 1, 1.0f }, M{ "b", 2, 2.0f } } };
    ivM ivm2{ { M{ "x", 3, 3.0f }, M{ "y", 4, 4.0f } } };
    ivm.swap( ivm2 );
    test( ivm[ 0 ].getStr() == "x" );
    test( ivm[ 1 ].getStr() == "y" );
    test( ivm2[ 0 ].getStr() == "a" );
    test( ivm2[ 1 ].getStr() == "b" );

    std::swap( ivm, ivm2 );
    test( ivm[ 0 ].getStr() == "a" );
    test( ivm[ 1 ].getStr() == "b" );
    test( ivm2[ 0 ].getStr() == "x" );
    test( ivm2[ 1 ].getStr() == "y" );
  }

  // non-member erase and erase_if
  {
    inplace_vector<int, 10> iv( 10, 0 );
    std::ranges::iota( iv, 0 );

    auto count = erase( iv, 3 );
    test( count == 1 );
    test( iv[ 2 ] == 2 );
    test( iv[ 3 ] == 4 );

    count = erase_if( iv, []( int x ) { return x % 2 == 0; } );
    test( count == 5 );
    test( iv.size() == 4 );
    test( iv[ 0 ] == 1 );
    test( iv[ 1 ] == 5 );
    test( iv[ 2 ] == 7 );
    test( iv[ 3 ] == 9 );
  }

  // comparison
  {
    inplace_vector<int, 2> ivA{ {1,2} };
    inplace_vector<int, 2> ivB{ {1,2} };
    test( ivA == ivB );
    ivA[ 0 ] = 2;
    test( ivA != ivB );
    test( ivA > ivB );
    test( ivB < ivA );
    test( ivA >= ivB );
    test( ivB <= ivA );
    ivA.pop_back();
    test( ivA < ivB );
    test( ivB > ivA );

    inplace_vector<M, 2> ivX{ M{}, M{} };
    inplace_vector<M, 2> ivY{ M{}, M{} };
    test( ivX == ivY );
    ivX[ 0 ] = M{ "a", 1, 20.f };
    test( ivX > ivY );
    test( ivY < ivX );
    test( ivX >= ivY );
    test( ivY <= ivX );
    ivX[ 0 ] = M{};
    test( ivX >= ivY );
    test( ivY <= ivX );
  }

  // test code from cppreference.com
  {
    inplace_vector<int, 4> v1{ 0, 1, 2 };
    test( v1.max_size() == 4 );
    test( v1.capacity() == 4 );
    test( v1.size() == 3 );
    test( std::ranges::equal( v1, std::array{ 0, 1, 2 } ) );
    test( v1[ 0 ] == 0 );
    test( v1.at( 0 ) == 0 );
    test( v1.front() == 0 );
    test( *v1.begin() == 0 );
    test( v1.back() == 2 );
    v1.push_back( 3 );
    test( v1.back() == 3 );
    test( std::ranges::equal( v1, std::array{ 0, 1, 2, 3 } ) );
    v1.resize( 3 );
    test( std::ranges::equal( v1, std::array{ 0, 1, 2 } ) );
    test( v1.try_push_back( 3 ) != nullptr );
    test( v1.back() == 3 );
    test( v1.size() == 4 );
    test( v1.try_push_back( 13 ) == nullptr ); // no place
    test( v1.back() == 3 );
    test( v1.size() == 4 );
    v1.clear();
    test( v1.size() == 0 );
    test( v1.empty() );
  }
}

///////////////////////////////////////////////////////////////////////////////
