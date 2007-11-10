//
// Utility.cpp
//
// $Id: //poco/Main/Data/SQLite/src/Utility.cpp#5 $
//
// Library: SQLite
// Package: SQLite
// Module:  Utility
//
// Implementation of Utility
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/SQLite/SQLiteException.h"
#include "Poco/NumberFormatter.h"
#include "Poco/String.h"
#include "Poco/Exception.h"
#include "sqlite3.h"


namespace Poco {
namespace Data {
namespace SQLite {


const std::string Utility::SQLITE_DATE_FORMAT = "%Y-%m-%d";
const std::string Utility::SQLITE_TIME_FORMAT = "%H:%M:%S";


std::string Utility::lastError(sqlite3 *pDB)
{
	return std::string(sqlite3_errmsg(pDB));
}


MetaColumn::ColumnDataType Utility::getColumnType(sqlite3_stmt* pStmt, std::size_t pos)
{
	poco_assert_dbg (pStmt);

	const char* pc = sqlite3_column_decltype(pStmt, (int) pos);
	std::string sqliteType = pc ? pc : "";
	Poco::toUpperInPlace(sqliteType);
	
	if (sqliteType.npos != sqliteType.find("INT"))
		return MetaColumn::FDT_INT64;
	else if (sqliteType.empty() || 
		sqliteType.npos != sqliteType.find("CHAR") ||
		sqliteType.npos != sqliteType.find("CLOB") ||
		sqliteType.npos != sqliteType.find("TEXT"))
		return MetaColumn::FDT_STRING;
	else if (sqliteType.npos != sqliteType.find("REAL") ||
		sqliteType.npos != sqliteType.find("FLOA") ||
		sqliteType.npos != sqliteType.find("DOUB"))
		return MetaColumn::FDT_DOUBLE;
	else if (sqliteType.npos != sqliteType.find("BLOB"))
		return MetaColumn::FDT_BLOB;
	else if (sqliteType.npos != sqliteType.find("DATE"))
		return MetaColumn::FDT_TIMESTAMP;

	throw Poco::NotFoundException();
}


void Utility::throwException(int rc, const std::string& addErrMsg)
{
	switch (rc)
	{
	case SQLITE_OK:
		break;
	case SQLITE_ERROR:
		throw InvalidSQLStatementException(std::string("SQL error or missing database"), addErrMsg);
	case SQLITE_INTERNAL:
		throw InternalDBErrorException(std::string("An internal logic error in SQLite"), addErrMsg);
	case SQLITE_PERM:
		throw DBAccessDeniedException(std::string("Access permission denied"), addErrMsg);
	case SQLITE_ABORT:
		throw ExecutionAbortedException(std::string("Callback routine requested an abort"), addErrMsg);
	case SQLITE_BUSY:
		throw DBLockedException(std::string("The database file is locked"), addErrMsg);
	case SQLITE_LOCKED:
		throw TableLockedException(std::string("A table in the database is locked"), addErrMsg);
	case SQLITE_NOMEM:
		throw NoMemoryException(std::string("A malloc() failed"), addErrMsg);
	case SQLITE_READONLY:
		throw ReadOnlyException(std::string("Attempt to write a readonly database"), addErrMsg);
	case SQLITE_INTERRUPT:
		throw InterruptException(std::string("Operation terminated by sqlite_interrupt()"), addErrMsg);
	case SQLITE_IOERR:
		throw IOErrorException(std::string("Some kind of disk I/O error occurred"), addErrMsg);
	case SQLITE_CORRUPT:
		throw CorruptImageException(std::string("The database disk image is malformed"), addErrMsg);
	case SQLITE_NOTFOUND:
		throw TableNotFoundException(std::string("Table or record not found"), addErrMsg);
	case SQLITE_FULL:
		throw DatabaseFullException(std::string("Insertion failed because database is full"), addErrMsg);
	case SQLITE_CANTOPEN:
		throw CantOpenDBFileException(std::string("Unable to open the database file"), addErrMsg);
	case SQLITE_PROTOCOL:
		throw LockProtocolException(std::string("Database lock protocol error"), addErrMsg);
	case SQLITE_EMPTY:
		throw InternalDBErrorException(std::string("(Internal Only) Database table is empty"), addErrMsg);
	case SQLITE_SCHEMA:
		throw SchemaDiffersException(std::string("The database schema changed"), addErrMsg);
	case SQLITE_TOOBIG:
		throw RowTooBigException(std::string("Too much data for one row of a table"), addErrMsg);
	case SQLITE_CONSTRAINT:
		throw ConstraintViolationException(std::string("Abort due to constraint violation"), addErrMsg);
	case SQLITE_MISMATCH:
		throw DataTypeMismatchException(std::string("Data type mismatch"), addErrMsg);
	case SQLITE_MISUSE:
		throw InvalidLibraryUseException(std::string("Library used incorrectly"), addErrMsg);
	case SQLITE_NOLFS:
		throw OSFeaturesMissingException(std::string("Uses OS features not supported on host"), addErrMsg);
	case SQLITE_AUTH:
		throw AuthorizationDeniedException(std::string("Authorization denied"), addErrMsg);
	case SQLITE_FORMAT:
		throw CorruptImageException(std::string("Auxiliary database format error"), addErrMsg);
	case SQLITE_NOTADB:
		throw CorruptImageException(std::string("File opened that is not a database file"), addErrMsg);
	case SQLITE_RANGE:
		throw InvalidSQLStatementException(std::string("Bind Parameter out of range (Access of invalid position 0? bind starts with 1!)"), addErrMsg);
	case SQLITE_ROW:
		break; // sqlite_step() has another row ready
	case SQLITE_DONE:
		break; // sqlite_step() has finished executing
	default:
		throw SQLiteException(std::string("Unkown error code: ") + Poco::NumberFormatter::format(rc), addErrMsg);
	}
}


} } } // namespace Poco::Data::SQLite
