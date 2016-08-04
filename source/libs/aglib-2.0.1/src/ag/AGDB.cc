// AGDB.cc: impletation of database access functions
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <vector>

#include <ag/AGAPI.h>
#include <ag/db.h>
#include <ag/Utilities.h>


extern SQLHENV henv;
extern SQLHDBC hdbc;
extern SQLHSTMT hstmt;
extern int connected;

#define BUFSIZE 1000

vector<string> getColNames();


bool LoadFromDB(string connStr,AGSetId agSetId) {
  char fetchBuffer[2048];
  SDWORD colIndicator;

  if (DB_Connect(connStr.c_str()) != 0) {
    DB_Errors("DB_Connect");
    return false;
  }

  if (ExistsAGSet(agSetId))
    DeleteAGSet(agSetId);
  CreateAGSet(agSetId);
  

  // Load timelines
  DB_Query("select TIMELINEID from TIMELINE where AGSetId ='" + agSetId + "'");
  while (1) {
      int sts = SQLFetch (hstmt);
      
      if (sts == SQL_NO_DATA_FOUND)
	break;
      
      if (sts != SQL_SUCCESS)
	{
	  DB_Errors ("Fetch");
	  break;
	}
      if (SQLGetData (hstmt, 1, SQL_CHAR, fetchBuffer,
		      sizeof (fetchBuffer), &colIndicator) != SQL_SUCCESS)
	{
	  DB_Errors ("SQLGetData");
	  break;
	}

      CreateTimeline(string(fetchBuffer));
  }


  // Load Signals
  DB_Query("select SIGNALID,MIMECLASS,MIMETYPE,ENCODING,UNIT,XLINKHREF,TRACK from SIGNAL where AGSetId ='" + agSetId + "'");
  while (1) {
    char sid[BUFSIZE],mimeclass[BUFSIZE],mimetype[BUFSIZE],encoding[BUFSIZE];
    char unit[BUFSIZE],xlinkhref[BUFSIZE],track[BUFSIZE];
    int sts = SQLFetch (hstmt);
    
    if (sts == SQL_NO_DATA_FOUND)
      break;
    
    if (sts != SQL_SUCCESS)
      {
	DB_Errors ("Fetch");
	break;
      }

    SQLGetData (hstmt, 1, SQL_CHAR, sid,BUFSIZE, &colIndicator);
    SQLGetData (hstmt, 2, SQL_CHAR, mimeclass,BUFSIZE, &colIndicator);
    SQLGetData (hstmt, 3, SQL_CHAR, mimetype,BUFSIZE, &colIndicator);
    SQLGetData (hstmt, 4, SQL_CHAR, encoding,BUFSIZE, &colIndicator);
    SQLGetData (hstmt, 5, SQL_CHAR, unit,BUFSIZE, &colIndicator);
    SQLGetData (hstmt, 6, SQL_CHAR, xlinkhref,BUFSIZE, &colIndicator);
    SQLGetData (hstmt, 7, SQL_CHAR, track,BUFSIZE, &colIndicator);
    
    CreateSignal(string(sid),string(xlinkhref),string(mimeclass),string(mimetype),string(encoding),string(unit),string(track));
  }

  // Load AGs
  DB_Query("select AGID,TIMELINEID from AG where AGSetId ='" + agSetId + "'");
  while (1) {
    char agid[100],tid[100];
    int sts = SQLFetch (hstmt);
    
    if (sts == SQL_NO_DATA_FOUND)
      break;
    
    if (sts != SQL_SUCCESS)
      {
	DB_Errors ("Fetch");
	break;
      }

    SQLGetData (hstmt, 1, SQL_CHAR, agid,100, &colIndicator);
    SQLGetData (hstmt, 2, SQL_CHAR, tid,100, &colIndicator);
    
    CreateAG(string(agid),string(tid));
  }

  // Load Anchors
  DB_Query("select ANCHORID,OFFSET,UNIT,SIGNALS from ANCHOR where AGSetId ='" + agSetId + "'");
  while (1) {
    char aid[100],unit[100],signals[100];
    float offset;

    int sts = SQLFetch (hstmt);
    
    if (sts == SQL_NO_DATA_FOUND)
      break;
    
    if (sts != SQL_SUCCESS)
      {
	DB_Errors ("Fetch");
	break;
      }

    SQLGetData (hstmt, 1, SQL_CHAR, aid,100, &colIndicator);
    SQLGetData (hstmt, 2, SQL_REAL, &offset,sizeof(float), &colIndicator);
    SQLGetData (hstmt, 3, SQL_CHAR, unit,100, &colIndicator);
    SQLGetData (hstmt, 4, SQL_CHAR, signals,100, &colIndicator);

    set<SignalId> sigSet;
    Utilities::string2set(signals, sigSet);
    CreateAnchor(string(aid),offset,string(unit),sigSet);
  }

  // Load Annotations
  DB_Query("select ANNOTATIONID,START,END,TYPE from ANNOTATION where AGSetId ='" + agSetId + "'");
  while (1) {
    char aid[100],start[100],end[100],type[100];

    int sts = SQLFetch (hstmt);
    
    if (sts == SQL_NO_DATA_FOUND)
      break;
    
    if (sts != SQL_SUCCESS)
      {
	DB_Errors ("Fetch");
	break;
      }

    SQLGetData (hstmt, 1, SQL_CHAR, aid,100, &colIndicator);
    SQLGetData (hstmt, 2, SQL_CHAR, start,100, &colIndicator);
    SQLGetData (hstmt, 3, SQL_CHAR, end,100, &colIndicator);
    SQLGetData (hstmt, 4, SQL_CHAR, type,100, &colIndicator);

    CreateAnnotation(string(aid),string(start),string(end),string(type));
  }
  

  // Load Metadata
  DB_Query("select ID,NAME,VALUE from METADATA where AGSetId ='" + agSetId + "'");
  while (1) {
    char id[100],name[100],value[100];

    int sts = SQLFetch (hstmt);
    
    if (sts == SQL_NO_DATA_FOUND)
      break;
    
    if (sts != SQL_SUCCESS)
      {
	DB_Errors ("Fetch");
	break;
      }

    SQLGetData (hstmt, 1, SQL_CHAR, id,100, &colIndicator);
    SQLGetData (hstmt, 2, SQL_CHAR, name,100, &colIndicator);
    SQLGetData (hstmt, 3, SQL_CHAR, value,100, &colIndicator);

    SetFeature(string(id),string(name),string(value));
  }

  // Load Feature
  if (DB_Query("select * from " + agSetId) == 0) {
    vector<string> colNames = getColNames();
    int numCols = colNames.size();
    string annotationId,value;
    
    while (1) {
      char buffer[BUFSIZE];
      
      int sts = SQLFetch (hstmt);
    
      if (sts == SQL_NO_DATA_FOUND)
	break;
    
      if (sts != SQL_SUCCESS)
	{
	  DB_Errors ("Fetch");
	  break;
	}

      SQLGetData (hstmt, 1, SQL_CHAR, buffer,BUFSIZE, &colIndicator);
      annotationId = string(buffer);
      for (int i = 2; i <= numCols; i++) {
	SQLGetData (hstmt, i, SQL_CHAR, buffer,BUFSIZE, &colIndicator);
	if (colIndicator != SQL_NULL_DATA) {
	  value = string(buffer);
	  SetFeature(annotationId,colNames[i-1],value);
	}
      }
    }
  }
  DB_Disconnect();  
  return true;
}

bool StoreToDB(string connStr, AGSetId agSetId) {
  list<string> storeSQLs = StoreSQLs(agSetId);

  if (DB_Connect(connStr.c_str()) != 0) {
     DB_Errors ("DB_Connect");
     return false;
  }

  DB_Query("delete from AGSET where AGSetId ='" + agSetId + "'");
  DB_Query("delete from TIMELINE where AGSetId ='" + agSetId + "'");
  DB_Query("delete from SIGNAL where AGSetId ='" + agSetId + "'");
  DB_Query("delete from AG where AGSetId ='" + agSetId + "'");
  DB_Query("delete from ANNOTATION where AGSetId ='" + agSetId + "'");
  DB_Query("delete from ANCHOR where AGSetId ='" + agSetId + "'");
  DB_Query("delete from METADATA where AGSetId ='" + agSetId + "'");

  for (list<string>::iterator pos = storeSQLs.begin(); pos != storeSQLs.end(); ++pos) {
    if (DB_Query(*pos) != 0) {
      DB_Errors("DB_Query");
      break;
    }
  }
  DB_Disconnect();

  return true;

}

/*
 *  Get the column names, in its order, for the current cursor.
 */
vector<string> getColNames() {
  short numCols;
  short colNum;
  char colName[50];
  short colType;
  UDWORD colPrecision;
  SDWORD colIndicator;
  short colScale;
  short colNullable;
  vector<string> colNames;

  if (SQLNumResultCols (hstmt, &numCols) != SQL_SUCCESS)
    {
      DB_Errors ("SQLNumResultCols");
      exit(-1);
    }
  if (numCols == 0)
    return colNames;
  /*
   *  Get the names for the columns
   */
  for (colNum = 1; colNum <= numCols; colNum++) {
    /*
     *  Get the name and other type information
     */
    if (SQLDescribeCol (hstmt, colNum, (UCHAR *) colName,
			sizeof (colName), NULL, &colType, &colPrecision,
			&colScale, &colNullable) != SQL_SUCCESS)
      {
	DB_Errors ("SQLDescribeCol");
	exit(-1);
      }
      
    colNames.push_back(string(colName));
  }
  
  return colNames;
}
