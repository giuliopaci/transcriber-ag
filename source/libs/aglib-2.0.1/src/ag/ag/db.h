// db.h: To connect to/disconnect from, querying a database server
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#ifdef _MSC_VER
#include <windows.h>
#include "sql.h"
#include "sqlext.h"
#else
#include "isql.h"
#include "isqlext.h"
#endif
#include <string>

using namespace std;

// Connect to DB server, using specified connect string
int DB_Connect (const char *connStr);

// Disconnect from DB server
int DB_Disconnect (void);

// Prepare and execute a SQL query
int DB_Query(string sqlQuery);

// Handle a message
void DB_MesgHandler (char *reason);

// Report an error
int DB_Errors (char *where);
