//
// Extractor.h
//
// $Id: //poco/Main/Data/ODBC/include/Poco/Data/ODBC/Extractor.h#5 $
//
// Library: ODBC
// Package: ODBC
// Module:  Extractor
//
// Definition of the Extractor class.
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


#ifndef DataConnectors_ODBC_Extractor_INCLUDED
#define DataConnectors_ODBC_Extractor_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include "Poco/Data/AbstractExtractor.h"
#include "Poco/Data/ODBC/Preparation.h"
#include "Poco/Data/ODBC/ODBCColumn.h"
#include "Poco/Data/ODBC/Error.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/DateTime.h"
#include "Poco/Any.h"
#include "Poco/DynamicAny.h"
#include "Poco/Exception.h"
#include <map>
#ifdef POCO_OS_FAMILY_WINDOWS
#include <windows.h>
#endif
#include <sqltypes.h>


namespace Poco {
namespace Data {
namespace ODBC {


class ODBC_API Extractor: public Poco::Data::AbstractExtractor
	/// Extracts and converts data values from the result row returned by ODBC.
	/// If NULL is received, the incoming val value is not changed and false is returned
{
public:
	Extractor(const StatementHandle& rStmt, 
		Preparation& rPreparation);
		/// Creates the Extractor.

	~Extractor();
		/// Destroys the Extractor.

	bool extract(std::size_t pos, Poco::Int8& val);
		/// Extracts an Int8.

	bool extract(std::size_t pos, Poco::UInt8& val);
		/// Extracts an UInt8.

	bool extract(std::size_t pos, Poco::Int16& val);
		/// Extracts an Int16.

	bool extract(std::size_t pos, Poco::UInt16& val);
		/// Extracts an UInt16.

	bool extract(std::size_t pos, Poco::Int32& val);
		/// Extracts an Int32.

	bool extract(std::size_t pos, Poco::UInt32& val);
		/// Extracts an UInt32.

	bool extract(std::size_t pos, Poco::Int64& val);
		/// Extracts an Int64.

	bool extract(std::size_t pos, Poco::UInt64& val);
		/// Extracts an UInt64.

#ifndef POCO_LONG_IS_64_BIT
	bool extract(std::size_t pos, long& val);
		/// Extracts a long.
#endif

	bool extract(std::size_t pos, bool& val);
		/// Extracts a boolean.

	bool extract(std::size_t pos, float& val);
		/// Extracts a float.

	bool extract(std::size_t pos, double& val);
		/// Extracts a double.

	bool extract(std::size_t pos, char& val);
		/// Extracts a single character.

	bool extract(std::size_t pos, std::string& val);
		/// Extracts a string.

	bool extract(std::size_t pos, Poco::Data::BLOB& val);
		/// Extracts a BLOB.

	bool extract(std::size_t pos, Poco::Data::Date& val);
		/// Extracts a Date.

	bool extract(std::size_t pos, Poco::Data::Time& val);
		/// Extracts a Time.

	bool extract(std::size_t pos, Poco::DateTime& val);
		/// Extracts a DateTime.
	
	bool extract(std::size_t pos, Poco::Any& val);
		/// Extracts an Any.

	bool extract(std::size_t pos, Poco::DynamicAny& val);
		/// Extracts a DynamicAny.

	void setDataExtraction(Preparation::DataExtraction ext);
		/// Set data extraction mode.

	Preparation::DataExtraction getDataExtraction() const;
		/// Returns data extraction mode.

	bool isNull(std::size_t pos);
		/// Returns true if the current row value at pos column is null.

	void reset();
		/// Resets the internally cached null value indicators. 

private:
	static const int CHUNK_SIZE = 1024;
		/// Amount of data retrieved in one SQLGetData() request when doing manual extract.

	static const std::string FLD_SIZE_EXCEEDED_FMT;
		/// String format for the exception message when the field size is exceeded.

	void checkDataSize(std::size_t size);
		/// This check is only performed for bound data
		/// retrieval from variable length columns.
		/// The reason for this check is to ensure we can
		/// accept the value ODBC driver is supplying
		/// (i.e. the bound buffer is large enough to receive
		/// the returned value)

	void resizeLengths(std::size_t pos);
		/// Resizes the vector holding extracted data lengths to the
		/// appropriate size.

	template<typename T>
	bool extractBoundImpl(std::size_t pos, T& val)
	{
		if (isNull(pos)) return false;

		poco_assert (typeid(T) == _rPreparation[pos].type());

		val = *AnyCast<T>(&_rPreparation[pos]); 
		return true;
	}

	template<typename T>
	bool extractManualImpl(std::size_t pos, T& val, SQLSMALLINT cType)
	{
		SQLRETURN rc = 0;
		T value = (T) 0;

		resizeLengths(pos);

		rc = SQLGetData(_rStmt, 
			(SQLUSMALLINT) pos + 1, 
			cType,  //C data type
			&value, //returned value
			0,      //buffer length (ignored)
			&_lengths[pos]);  //length indicator

		if (Utility::isError(rc))
			throw StatementException(_rStmt, "SQLGetData()");
		
		if (isNullLengthIndicator(_lengths[pos])) 
			return false;
		else 
		{
			//for fixed-length data, buffer must be large enough
			//otherwise, driver may write past the end
			poco_assert_dbg (_lengths[pos] <= sizeof(T));
			val = value;
		}

		return true;
	}

	template <typename T>
	bool extractImpl(std::size_t pos, T& val)
		/// Utility function for extraction of Any and DynamicAny.
	{
		ODBCColumn column(_rStmt, pos);

		switch (column.type())
		{
			case MetaColumn::FDT_INT8:
			{ Poco::Int8 i = 0; extract(pos, i); val = i; return true; }

			case MetaColumn::FDT_UINT8:
			{ Poco::UInt8 i = 0; extract(pos, i); val = i; return true;	}

			case MetaColumn::FDT_INT16:
			{ Poco::Int16 i = 0; extract(pos, i); val = i; return true;	}

			case MetaColumn::FDT_UINT16:
			{ Poco::UInt16 i = 0; extract(pos, i); val = i; return true; }

			case MetaColumn::FDT_INT32:
			{ Poco::Int32 i = 0; extract(pos, i); val = i; return true;	}

			case MetaColumn::FDT_UINT32:
			{ Poco::UInt32 i = 0; extract(pos, i); val = i; return true; }

			case MetaColumn::FDT_INT64:
			{ Poco::Int64 i = 0; extract(pos, i); val = i; return true;	}

			case MetaColumn::FDT_UINT64:
			{ Poco::UInt64 i = 0; extract(pos, i); val = i; return true; }

			case MetaColumn::FDT_BOOL:
			{ bool b; extract(pos, b); val = b; return true; }

			case MetaColumn::FDT_FLOAT:
			{ float f; extract(pos, f); val = f; return true; }

			case MetaColumn::FDT_DOUBLE:
			{ double d; extract(pos, d); val = d; return true; }

			case MetaColumn::FDT_STRING:
			{ std::string s; extract(pos, s); val = s; return true;	}

			case MetaColumn::FDT_BLOB:
			{ Poco::Data::BLOB b; extract(pos, b); val = b; return true; }

			case MetaColumn::FDT_TIMESTAMP:
			{ Poco::DateTime b; extract(pos, b); val = b; return true; }

			default: 
				throw DataFormatException("Unsupported data type.");
		}

		return false;
	}

	bool isNullLengthIndicator(SQLLEN val) const;
		/// The reason for this utility wrapper are platforms where 
		/// SQLLEN macro (a.k.a. SQLINTEGER) yields 64-bit value, 
		/// while SQL_NULL_DATA (#define'd as -1 literal) remains 32-bit.

	const StatementHandle&      _rStmt;
	Preparation&                _rPreparation;
	Preparation::DataExtraction _dataExtraction;
	std::vector<SQLLEN>         _lengths;
};


///
/// inlines
///


inline void Extractor::setDataExtraction(Preparation::DataExtraction ext)
{
	_rPreparation.setDataExtraction(_dataExtraction = ext);
}


inline Preparation::DataExtraction Extractor::getDataExtraction() const
{
	return _dataExtraction;
}


inline void Extractor::reset()
{
	_lengths.clear();
}


inline void Extractor::resizeLengths(std::size_t pos)
{
	if (pos >= _lengths.size()) 
		_lengths.resize(pos + 1, (SQLLEN) 0);
}


inline bool Extractor::isNullLengthIndicator(SQLLEN val) const
{
	return SQL_NULL_DATA == (int) val;
}


} } } // namespace Poco::Data::ODBC


#endif // DataConnectors_ODBC_Extractor_INCLUDED
