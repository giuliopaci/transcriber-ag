// CreateTables.c: creates AG tables on a annotation database server
// Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <stdio.h>
#include <string.h>

#include "isql.h"
#include "isqlext.h"

#define MAXCOLS		32

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
int DB_Connect (char *connStr)
{
  short buflen;
  char buf[257];
  SQLCHAR dataSource[120];
  SQLCHAR dsn[33];
  SQLCHAR desc[255];
  SWORD len1, len2;
  int status;

  if (SQLAllocEnv (&henv) != SQL_SUCCESS)
    return -1;

  if (SQLAllocConnect (henv, &hdbc) != SQL_SUCCESS)
    return -1;

  if (connStr && *connStr)
    strcpy ((char *)dataSource, connStr);
  else {
    DB_MesgHandler("Empty connection string!");
    exit(-1);
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

void CreateTables (void) {
  char* AGSET = "CREATE TABLE AGSET (\
AGSETID VARCHAR(50) BINARY NOT NULL,\
VERSION CHAR(10),\
XMLNS CHAR(30),\
XLINK CHAR(30),\
DC CHAR(50),\
PRIMARY KEY (AGSETID))";
  char* AG = "CREATE TABLE AG (\
AGID VARCHAR(50) BINARY NOT NULL,\
AGSETID VARCHAR(50) BINARY NOT NULL,\
TIMELINEID VARCHAR(50) BINARY NOT NULL,\
TYPE CHAR(10),\
TID CHAR(50),\
PRIMARY KEY (AGID))";
  char* TIMELINE = "CREATE TABLE TIMELINE (\
AGSETID VARCHAR(50) BINARY NOT NULL,\
TIMELINEID VARCHAR(50) BINARY NOT NULL,\
PRIMARY KEY (TIMELINEID))";
  char* SIGNAL = "CREATE TABLE SIGNAL (\
AGSETID VARCHAR(50) BINARY NOT NULL,\
TIMELINEID VARCHAR(50) BINARY NOT NULL,\
SIGNALID VARCHAR(50) BINARY NOT NULL,\
MIMECLASS VARCHAR(50),\
MIMETYPE VARCHAR(50),\
ENCODING VARCHAR(50),\
UNIT VARCHAR(50),\
XLINKTYPE VARCHAR(50),\
XLINKHREF VARCHAR(50),\
TRACK VARCHAR(50),\
PRIMARY KEY (SIGNALID))";
  char* ANNOTATION = "CREATE TABLE ANNOTATION (\
AGSETID VARCHAR(50) BINARY NOT NULL,\
AGID VARCHAR(50) BINARY NOT NULL,\
ANNOTATIONID VARCHAR(50) BINARY NOT NULL,\
START VARCHAR(50),\
END VARCHAR(50),\
TYPE VARCHAR(50),\
PRIMARY KEY (ANNOTATIONID))";
  char* ANCHOR = "CREATE TABLE ANCHOR (\
AGSETID VARCHAR(50) BINARY NOT NULL,\
AGID VARCHAR(50) BINARY NOT NULL,\
ANCHORID VARCHAR(50) BINARY NOT NULL,\
OFFSET FLOAT,\
UNIT VARCHAR(50),\
SIGNALS VARCHAR(50),\
PRIMARY KEY (ANCHORID))";

  char* METADATA = "CREATE TABLE METADATA (\
AGSETID VARCHAR(50) BINARY NOT NULL,\
AGID VARCHAR(50) BINARY,\
ID VARCHAR(50) BINARY NOT NULL,\
NAME VARCHAR(50) NOT NULL,\
VALUE TEXT,\
PRIMARY KEY (ID,NAME))";
  
/*    char* FEATURE = "CREATE TABLE FEATURE ( */
/*  AGSETID VARCHAR(50) NOT NULL, */
/*  AGID VARCHAR(50) NOT NULL, */
/*  ANNOTATIONID VARCHAR(50) NOT NULL, */
/*  NAME VARCHAR(50) NOT NULL, */
/*  VALUE TEXT, */
/*  PRIMARY KEY (ANNOTATIONID,NAME))"; */
  
  SQLPrepare (hstmt, (UCHAR *) AGSET, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table AGSET");
  else
    DB_MesgHandler ("Table AGSET created");

  SQLPrepare (hstmt, (UCHAR *) AG, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table AG");
  else
    DB_MesgHandler ("Table AG created");

  SQLPrepare (hstmt, (UCHAR *) TIMELINE, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table TIMELINE");
  else
    DB_MesgHandler ("Table TIMELINE created");

  SQLPrepare (hstmt, (UCHAR *) SIGNAL, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table SIGNAL");
  else
    DB_MesgHandler ("Table SIGNAL created");

  SQLPrepare (hstmt, (UCHAR *) ANNOTATION, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table ANNOTATION");
  else
    DB_MesgHandler ("Table ANNOTATION created");

  SQLPrepare (hstmt, (UCHAR *) ANCHOR, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table ANCHOR");
  else
    DB_MesgHandler ("Table ANCHOR created");

  SQLPrepare (hstmt, (UCHAR *) METADATA, SQL_NTS);
  if (SQLExecute (hstmt) != SQL_SUCCESS)
    DB_MesgHandler ("Error creating table METADATA");
  else
    DB_MesgHandler ("Table METADATA created");

/*    SQLPrepare (hstmt, (UCHAR *) FEATURE, SQL_NTS); */
/*    if (SQLExecute (hstmt) != SQL_SUCCESS) */
/*      DB_MesgHandler ("Error creating table FEATURE"); */
/*    else */
/*      DB_MesgHandler ("Table FEATURE created"); */

  return;
}

void DropTables() {
  SQLPrepare (hstmt, "DROP TABLE AGSET", SQL_NTS);
  SQLExecute (hstmt);
  SQLPrepare (hstmt, "DROP TABLE AG", SQL_NTS);
  SQLExecute (hstmt);
  SQLPrepare (hstmt, "DROP TABLE TIMELINE", SQL_NTS);
  SQLExecute (hstmt);
  SQLPrepare (hstmt, "DROP TABLE SIGNAL", SQL_NTS);
  SQLExecute (hstmt);
  SQLPrepare (hstmt, "DROP TABLE ANNOTATION", SQL_NTS);
  SQLExecute (hstmt);
  SQLPrepare (hstmt, "DROP TABLE ANCHOR", SQL_NTS);
  SQLExecute (hstmt);
  SQLPrepare (hstmt, "DROP TABLE METADATA", SQL_NTS);
  SQLExecute (hstmt);
/*    SQLPrepare (hstmt, "DROP TABLE FEATURE", SQL_NTS); */
/*    SQLExecute (hstmt); */

  return;
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
void
DB_MesgHandler (char *reason)
{
  fprintf (stderr, "DB_MesgHandler: %s\n", reason);
}


/*
 *  Show all the error information that is available
 */
int
DB_Errors (char *where)
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


int main (int argc, char **argv)
{
  if (argc != 2) {
    printf("Usage: CreateTables <connection string>\n");
    return 0;
  }

  if (DB_Connect (argv[1]) != 0)
    {
      DB_Errors ("DB_Connect");
    }

  DropTables();
  CreateTables();

  /*
   *  End the connection
   */
  DB_Disconnect ();
  return 0;
}


