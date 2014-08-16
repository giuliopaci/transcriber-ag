// RE.h: regex class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _RE_H_
#define _RE_H_

#include <string>
#include <sys/types.h>
#include <ag/regex.h>
#include <ag/agfioError.h>

/**
 * @brief Regex wrapper class.
 *
 * RE wraps the regex library by Henry Spencer, which is included in the
 * aglib package.
 */
class DllExport RE
{
private:
  regex_t *preg;
  size_t nmatch;
  regmatch_t *pmatch;
  string s;
  int span[2];
  bool matched;

public:
  /**
   * @brief Regular expression compilation error.
   */
  class CompError : public agfioError
  {
  public:
    CompError(const string& s): agfioError("RE:" + s) {}
  };

  /**
   * @brief Regular expression execution error.
   *
   * This error occurs when it is impossible to perform matching
   * because of some problem.
   */
  class MatchError : public agfioError
  {
  public:
    MatchError(const string& s): agfioError("RE:" + s) {}
  };

  /**
   * @brief Regular expression match error.
   *
   * GetError is thrown when there is an inappropriate attempt to get
   * matched substring. For example, if you try to get the 3rd matched
   * substring, when there are only 2 substrings in the expression,
   * GetError is thrown.
   */
  class GetError : public agfioError
  {
  public:
    GetError(const string& s): agfioError("RE:" + s) {}
  };

  /**
   * @brief Constructor.
   *
   * Compile the given "pattern"(a regular expression) into internal form
   * for matches which will be performed later on.
   */
  RE(const string &pattern);
  ~RE();

  /**
   * Match given string "s" with the compiled expression.
   * "True" is returned for "match success", "false" for "match failed."
   * The exception MatchError doesn't mean "match failed."
   */
  bool match(const string &s);

  /**
   * Get i-th matched substring.
   *
   * @param i
   *    The index of substring to get.
   * @return i-th substring
   *
   * Calling this after a failed match will cause GetError exception.
   * Out-of-range error detected and GetError will be thrown.
   */
  string
  get_matched(const int i);

  /**
   * Get the start and end index of i-th substring relative to the whole
   * string.
   *
   * @param i
   *    The index of target substring.
   * @return
   *    Array(pair) of integers. First one is the start index, and second
   *    one, the end index. The values in the array will be updated after
   *    another get_span() call.
   */
  const int*
  get_span(const int i);
  
  /**
   * @return
   *   The number of subexpressions in the expression
   */
  int
  get_nsub();
};

#endif
