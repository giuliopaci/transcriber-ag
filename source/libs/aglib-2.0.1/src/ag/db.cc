// db.cc: To connect to/disconnect from, querying a database server
// Author: Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <cstdio>
#include <string.h>
#include <string>

#include <ag/db.h>

#define MAXCOLS 32

SQLHENV henv;
SQLHDBC hdbc;
SQLHSTMT hstmt;
int connected;


/*
 *  Connect to the datasource
 *
 *  The connect string can have the following parts and they refer to
 *  the values in the odbc.ini file
 *
 *	DSN=<data source name>		[mandatory]
 *	HOST=<server host name>		[optional - value of Host]
 *	SVT=<database server type>	[optional - value of ServerType]
 *	DATABASE=<database path>	[optional - value of Database]
 *	OPTIONS=<db specific opts>	[optional - value of Options]
 *	UID=<user name>			[optional - value of LastUser]
 *	PWD=<password>			[optional]
 *	READONLY=<N|Y>			[optional - value of ReadOnly]
 *	FBS=<fetch buffer size>		[optional - value of FetchBufferSize]
 *
 *   Examples:
 *
 *	HOST=star;SVT=Informix 5;UID=demo;PWD=demo;DATABASE=stores5
 *
 *	DSN=stores5_informix;PWD=demo
 */
int DB_Connect (const char *connStr)
{
  short buflen;
  char buf[257];
  SQLCHAR dataSource[120];
  int status;

  if (SQLAllocEnv (&henv) != SQL_SUCCESS)
    return -1;

  if (SQLAllocConnect (henv, &hdbc) != SQL_SUCCESS)
    return -1;

  if (connStr && *connStr)
    strcpy ((char *)dataSource, connStr);
  else {
    DB_MesgHandler("Empty connection string!");
    return -1;
  }

  status = SQLDriverConnect (hdbc, 0, (UCHAR *) dataSource, SQL_NTS, 
  	(UCHAR *) buf, sizeof (buf), &buflen, SQL_DRIVER_COMPLETE);

  if (status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO)
    return -1;

  SQLSetConnectOption( hdbc, SQL_OPT_TRACEFILE, (UDWORD) "\\SQL.LOG"); 

  connected = 1;

  if (SQLAllocStmt (hdbc, &hstmt) != SQL_SUCCESS)
    return -1;

  return 0;
}


/*
 *  Disconnect from the database
 */
int DB_Disconnect (void)
{
  if (hstmt)
    SQLFreeStmt (hstmt, SQL_DROP);

  if (connected)
    SQLDisconnect (hdbc);

  if (hdbc)
    SQLFreeConnect (hdbc);

  if (henv)
    SQLFreeEnv (henv);

  return 0;
}

int DB_Query(string sqlQuery) {
  const char* request = sqlQuery.c_str();
  if (SQLPrepare (hstmt, (UCHAR *) request, SQL_NTS) != SQL_SUCCESS) {
    DB_Errors ("SQLPrepare");
    return -1;
  }
  if (SQLExecute (hstmt) != SQL_SUCCESS) {
    //DB_Errors ("SQLExec");
    return -1;
  }

  return 0;
}


/*
 *  This is the message handler for the communications layer.
 *
 *  The messages received here are not passed through SQLError,
 *  because they might occur when no connection is established.
 *
 *  Typically, Rejections from oplrqb are trapped here, and
 *  also RPC errors.
 *
 *  When no message handler is installed, the messages are output to stderr
 */
void DB_MesgHandler (char *reason)
{
  fprintf (stderr, "DB_MesgHandler: %s\n", reason);
}


/*
 *  Show all the error information that is available
 */
int DB_Errors (char *where)
{
  unsigned char buf[250];
  unsigned char sqlstate[15];

  /*
   *  Get statement errors
   */
  while (SQLError (henv, hdbc, hstmt, sqlstate, NULL,
      buf, sizeof(buf), NULL) == SQL_SUCCESS)
    {
      fprintf (stderr, "%s, SQLSTATE=%s\n", buf, sqlstate);
    }

  /*
   *  Get connection errors
   */
  while (SQLError (henv, hdbc, SQL_NULL_HSTMT, sqlstate, NULL,
		   buf, sizeof(buf), NULL) == SQL_SUCCESS)
    {
      fprintf (stderr, "%s, SQLSTATE=%s\n", buf, sqlstate);
    }

  /*
   *  Get environmental errors
   */
  while (SQLError (henv, SQL_NULL_HDBC, SQL_NULL_HSTMT, sqlstate, NULL,
      buf, sizeof(buf), NULL) == SQL_SUCCESS)
    {
      fprintf (stderr, "%s, SQLSTATE=%s\n", buf, sqlstate);
    }

  return -1;
}
