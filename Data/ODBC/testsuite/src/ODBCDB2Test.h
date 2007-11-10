//
// ODBCDB2Test.h
//
// $Id: //poco/Main/Data/ODBC/testsuite/src/ODBCDB2Test.h#4 $
//
// Definition of the ODBCDB2Test class.
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


#ifndef ODBCDB2Test_INCLUDED
#define ODBCDB2Test_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/SharedPtr.h"
#include "CppUnit/TestCase.h"
#include "SQLExecutor.h"


class ODBCDB2Test: public CppUnit::TestCase
	/// IBM DB2 UDB ODBC test class
	/// Tested:
	/// 
	/// Driver		|	DB				| OS
	/// ------------+-------------------+------------------------------------------
	///	9.01.00.356 | DB2 Express-C 9.1	| MS Windows XP Professional x64 v.2003/SP1
{
public:
	ODBCDB2Test(const std::string& name);
	~ODBCDB2Test();

	void testBareboneODBC();

	void testSimpleAccess();
	void testComplexType();

	void testSimpleAccessVector();
	void testComplexTypeVector();
	void testInsertVector();
	void testInsertEmptyVector();

	void testSimpleAccessList();
	void testComplexTypeList();
	void testInsertList();
	void testInsertEmptyList();

	void testSimpleAccessDeque();
	void testComplexTypeDeque();
	void testInsertDeque();
	void testInsertEmptyDeque();

	void testInsertSingleBulk();
	void testInsertSingleBulkVec();

	void testLimit();
	void testLimitOnce();
	void testLimitPrepare();
	void testLimitZero();
	void testPrepare();
	void testStep();

	void testSetSimple();
	void testSetComplex();
	void testSetComplexUnique();
	void testMultiSetSimple();
	void testMultiSetComplex();
	void testMapComplex();
	void testMapComplexUnique();
	void testMultiMapComplex();
	void testSelectIntoSingle();
	void testSelectIntoSingleStep();
	void testSelectIntoSingleFail();
	void testLowerLimitOk();
	void testLowerLimitFail();
	void testCombinedLimits();
	void testCombinedIllegalLimits();
	void testRange();
	void testIllegalRange();
	void testSingleSelect();
	void testEmptyDB();

	void testBLOB();
	void testBLOBStmt();

	void testDate();
	void testTime();
	void testDateTime();

	void testFloat();
	void testDouble();

	void testTuple();
	void testTupleVector();

	void testInternalExtraction();
	void testInternalStorageType();

	void testStoredProcedure();
	void testStoredProcedureAny();
	void testStoredProcedureDynamicAny();
	void testStoredFunction();

	void testNull();
	void testRowIterator();

	void testAsync();

	void testAny();
	void testDynamicAny();

	void testMultipleResults();

	void setUp();
	void tearDown();

	static CppUnit::Test* suite();

private:
	typedef Poco::Data::ODBC::Utility::DriverMap Drivers;
	typedef Poco::SharedPtr<Poco::Data::Session> SessionPtr;
	typedef Poco::SharedPtr<SQLExecutor>         ExecPtr;

	void dropObject(const std::string& type, const std::string& tableName);
	void recreatePersonTable();
	void recreatePersonBLOBTable();
	void recreatePersonDateTable();
	void recreatePersonTimeTable();
	void recreatePersonDateTimeTable();
	void recreateStringsTable();
	void recreateIntsTable();
	void recreateFloatsTable();
	void recreateTuplesTable();
	void recreateVectorsTable();
	void recreateAnysTable();
	void recreateNullsTable(const std::string& notNull = "");

	static bool init(const std::string& driver, const std::string& dsn);
	static bool canConnect(const std::string& driver, const std::string& dsn);

	static Drivers     _drivers;
	static std::string _dbConnString;
	static SessionPtr  _pSession;
	static ExecPtr     _pExecutor;
	static const bool  bindValues[8];
};


#endif // ODBCDB2Test_INCLUDED
