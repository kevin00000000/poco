//
// RecordSet.cpp
//
// $Id: //poco/Main/Data/src/RecordSet.cpp#2 $
//
// Library: Data
// Package: DataCore
// Module:  RecordSet
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


#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/Date.h"
#include "Poco/Data/Time.h"
#include "Poco/Data/DataException.h"
#include "Poco/DateTime.h"


using Poco::DateTime;


namespace Poco {
namespace Data {


RecordSet::RecordSet(const Statement& rStatement): 
	Statement(rStatement),
	_currentRow(0),
	_pBegin(0),
	_pEnd(0)
{
}


RecordSet::RecordSet(Session& rSession, const std::string& query): 
	Statement((rSession << query, now)),
	_currentRow(0),
	_pBegin(0),
	_pEnd(0)
{
}


RecordSet::~RecordSet()
{
	delete _pBegin;
	delete _pEnd;

	RowMap::iterator it = _rowMap.begin();
	RowMap::iterator end = _rowMap.end();
	for (; it != end; ++it)
		delete it->second;
}


DynamicAny RecordSet::value(std::size_t col, std::size_t row) const
{
	switch (columnType(col))
	{
		case MetaColumn::FDT_BOOL:      return value<bool>(col, row);
		case MetaColumn::FDT_INT8:      return value<Int8>(col, row);
		case MetaColumn::FDT_UINT8:     return value<UInt8>(col, row);
		case MetaColumn::FDT_INT16:     return value<Int16>(col, row);
		case MetaColumn::FDT_UINT16:    return value<UInt16>(col, row);
		case MetaColumn::FDT_INT32:	    return value<Int32>(col, row);
		case MetaColumn::FDT_UINT32:    return value<UInt32>(col, row);
		case MetaColumn::FDT_INT64:     return value<Int64>(col, row);
		case MetaColumn::FDT_UINT64:    return value<UInt64>(col, row);
		case MetaColumn::FDT_FLOAT:     return value<float>(col, row);
		case MetaColumn::FDT_DOUBLE:    return value<double>(col, row);
		case MetaColumn::FDT_STRING:    return value<std::string>(col, row);
		case MetaColumn::FDT_BLOB:      return value<BLOB>(col, row);
		case MetaColumn::FDT_DATE:      return value<Date>(col, row);
		case MetaColumn::FDT_TIME:      return value<Time>(col, row);
		case MetaColumn::FDT_TIMESTAMP: return value<DateTime>(col, row);
		default:
			throw UnknownTypeException("Data type not supported.");
	}
}


DynamicAny RecordSet::value(const std::string& name, std::size_t row) const
{
	switch (columnType(name))
	{
		case MetaColumn::FDT_BOOL:      return value<bool>(name, row);
		case MetaColumn::FDT_INT8:      return value<Int8>(name, row);
		case MetaColumn::FDT_UINT8:     return value<UInt8>(name, row);
		case MetaColumn::FDT_INT16:     return value<Int16>(name, row);
		case MetaColumn::FDT_UINT16:    return value<UInt16>(name, row);
		case MetaColumn::FDT_INT32:	    return value<Int32>(name, row);
		case MetaColumn::FDT_UINT32:    return value<UInt32>(name, row);
		case MetaColumn::FDT_INT64:     return value<Int64>(name, row);
		case MetaColumn::FDT_UINT64:    return value<UInt64>(name, row);
		case MetaColumn::FDT_FLOAT:     return value<float>(name, row);
		case MetaColumn::FDT_DOUBLE:    return value<double>(name, row);
		case MetaColumn::FDT_STRING:    return value<std::string>(name, row);
		case MetaColumn::FDT_BLOB:      return value<BLOB>(name, row);
		case MetaColumn::FDT_DATE:      return value<Date>(name, row);
		case MetaColumn::FDT_TIME:      return value<Time>(name, row);
		case MetaColumn::FDT_TIMESTAMP: return value<DateTime>(name, row);
		default:
			throw UnknownTypeException("Data type not supported.");
	}
}


const RowIterator& RecordSet::begin()
{
	if (!_pBegin)
		_pBegin = new RowIterator(*this);

	return *_pBegin;
}


Row& RecordSet::row(std::size_t pos)
{
	if (pos > rowCount() - 1)
		throw RangeException("Invalid recordset row requested.");

	RowMap::iterator it = _rowMap.find(pos);
	Row* pRow = 0;
	if (it == _rowMap.end())
	{
		if (_rowMap.size())//reuse first row column names to save some memory 
		{
			pRow = new Row(_rowMap.begin()->second->names());
			for (std::size_t i = 0; i < columnCount(); ++i)
				pRow->set(i, value(i, pos));
		}
		else 
		{
			pRow = new Row;
			for (std::size_t i = 0; i < columnCount(); ++i)
				pRow->append(metaColumn(static_cast<UInt32>(pos)).name(), value(i, pos));
		}

		_rowMap.insert(RowMap::value_type(pos, pRow));
	}
	else
		pRow = it->second;

	poco_check_ptr (pRow);
	return *pRow;
}


bool RecordSet::moveFirst()
{
	if (rowCount() > 0)
	{
		_currentRow = 0;
		return true;
	}
	else return false;
}


bool RecordSet::moveNext()
{
	if (_currentRow >= rowCount() - 1) return false;
	++_currentRow;
	return true;
}


bool RecordSet::movePrevious()
{
	if (0 == _currentRow) return false;
	--_currentRow;
	return true;
}


bool RecordSet::moveLast()
{
	if (rowCount() > 0)
	{
		_currentRow = rowCount() - 1;
		return true;
	}
	else return false;
}


DynamicAny RecordSet::nvl(const std::string& name, const DynamicAny& deflt) const
{
	if (isNull(name))
		return deflt;
	else
		return value(name, _currentRow);
}


DynamicAny RecordSet::nvl(std::size_t index, const DynamicAny& deflt) const
{
	if (isNull(index, _currentRow))
		return deflt;
	else
		return value(index, _currentRow);
}


} } // namespace Poco::Data
