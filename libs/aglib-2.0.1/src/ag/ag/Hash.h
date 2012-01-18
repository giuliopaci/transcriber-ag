// HashString.h: HashString generates unique HashString for anchors
// and annotations
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef HashString_h
#define HashString_h


#include <string>
using namespace std;

/* (( BT Patch -- */
#if (__GNUC__ >= 3)
/* -- BT Patch )) */
#include <ext/hash_map>
#include <ext/hash_set>
/* (( BT Patch -- */
  #if (__GNUC__ > 3 || __GNUC_MINOR__ >= 1)
/* -- BT Patch )) */
  using namespace __gnu_cxx;
  #endif
#else
#include <hash_map>
#include <hash_set>
/* (( BT Patch -- */
  using namespace __gnu_cxx;
/* -- BT Patch )) */
#endif

/// Function to test string equivalence.
class StringEqual
{
 public:
  size_t operator()(string const &s1, string const &s2) const
    {
      return (s1 == s2);
    }
};

/// Hash function for strings.
class hashString
{
 public:
  size_t operator()(string const &str) const
    {
      hash<char const *> h;
      return (h(str.c_str()));
    }
};

/// Function to test pair<string, string> equivalence.
class StringPairEqual
{
 public:
  size_t operator()(pair<string,string> const &sp1, pair<string,string> const &sp2) const
    {
      return (sp1.first == sp2.first && sp1.second == sp2.second);
    }
};

/**
 * Hash function for pair<string,string>.
 * Concatenate the pair of strings and then do a string hash.
 **/
class hashStringPair
{
 public:
  size_t operator()(pair<string,string> const &sp) const
    {
      hash<char const *> h;
      return (h((sp.first+sp.second).c_str()));
    }
};

/// Function to test pointer equivalence
class PointerEqual
{
 public:
  size_t operator()(void*  const &p1, void* const &p2) const
    {
      return (p1 == p2);
    }
};

/// Hash function for pointers
class hashPointer
{
 public:
  size_t operator()(void* const &p) const
    {
      return (size_t)p;
    }
};


#endif
