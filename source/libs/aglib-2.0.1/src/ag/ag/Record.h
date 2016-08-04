// Record.h: Record style file model class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _RECORD_H_
#define _RECORD_H_

#include <string>
#include <fstream>
#include <ag/agfioError.h>

/// Record style file model.
class DllExport Record
{

private:
  int       size;          // # fields per record
  ifstream  input;         // input file
  string    filename;      // input file name
  string    *record_buf;   // one entry buffer for a record

protected:
  virtual void
  preprocess() {}

  virtual void
  postprocess() {}

  virtual void
  read_entry() = 0;

public:
  /**
   * @brief Out-of-range error.
   *
   * Some methods will throw this error when they
   * detect an out-of-range data access.
   */
  class RangeError : public agfioError
  {
  public:
    /**
     * @param s
     *   Error message.
     */
    RangeError(const string &s): agfioError("Record: " + s) {}
  };

  /**
   * @param n
   *   Size of record (= # fields).
   */
  Record(int n): size(n)
  { record_buf = new string[size]; }

  ~Record()
  { delete[] record_buf; }

  /**
   * @return
   *   True if open succeeds, false otherwise.
   */
  bool
  open(const string &filename);

  /**
   * @param line
   *   Input will be copied into this variable.
   * @return
   *   True if reading line succeeds, false if reading fails or reads an EOF.
   */
  bool
  readline(string& line);

  /**
   * @param i
   *    Index of the target field.
   * @param v
   *    Value to put on the i-th field of the record.
   */
  void
  put_ith(int i, const string& v);

  /**
   * @param i
   *    Index of the target field.
   * @return i-th field of the record.
   */
  string
  get_ith(int i);

  /// get file name
  string
  get_filename()
  { return filename; }

  /// return Size of the record.
  int
  get_size()
  { return size; }

  /// clear the read buffer
  void
  clear_record_buf() {
    for (int i=0; i < size; i++)
      record_buf[i] = "";
  }

  /**
   * @return
   *   True if the input stream is good, false otherwise.
   */
  bool
  good()
  { return input.good(); }

  /**
   * @return
   *   True if the file hit the end, false otherwise.
   */
  bool
  eof()
  { return input.eof(); }

};


#endif
