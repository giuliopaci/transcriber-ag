// RE.cc: regex class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/RE.h>

RE::~RE() {
  regfree_ag(preg);
  delete preg;
  delete[] pmatch;
}

RE::RE(const string &pattern)
{
  preg = new regex_t;
  int fail = regcomp_ag(preg, pattern.c_str(), REG_EXTENDED);
  if (fail)
  {
    delete preg;
    throw CompError("regcomp failed");
  }
  nmatch = preg->re_nsub + 1;  // (# of subexpr) + 1(= the whole expr)
  pmatch = new regmatch_t[nmatch];
  matched = false;
}

bool RE::match(const string &s)
{
  this->s = s;

  int fail = regexec_ag(preg, this->s.c_str(), nmatch, pmatch, 0);

  if (!fail)
    return (matched = true);
  else if (fail == REG_NOMATCH)
    return (matched = false);
  else
    throw MatchError("regexec failed" + fail);
}

string
RE::get_matched(const int i)
{
  if (matched)
  {
    if (i < nmatch && i >= 0)
    {
      if (pmatch[i].rm_so == -1)
	return "";
      else
	return s.substr(pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so);
    }
    else
      throw GetError("out of access range");
  }
  else 
    throw GetError("match didn't succeed");
}

const int*
RE::get_span(const int i)
{
  if (matched)
  {
    if (i < nmatch && i >= 0)
    {
      span[0] = pmatch[i].rm_so;
      span[1] = pmatch[i].rm_eo;
      return span;
    }
    else
      throw GetError("out of access range");
  }
  else
    throw GetError("match didn't succeed");
}

int
RE::get_nsub()
{
  return nmatch - 1;
}
