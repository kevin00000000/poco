//
// Statement.h
//
// $Id: //poco/Main/Data/include/Poco/Data/Statement.h#18 $
//
// Library: Data
// Package: DataCore
// Module:  Statement
//
// Definition of the Statement class.
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


#ifndef Data_Statement_INCLUDED
#define Data_Statement_INCLUDED


#include "Poco/Data/Data.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/Range.h"
#include "Poco/Data/Step.h"
#include "Poco/SharedPtr.h"
#include "Poco/Mutex.h"
#include "Poco/ActiveMethod.h"
#include "Poco/ActiveResult.h"


namespace Poco {
namespace Data {


class AbstractBinding;
class AbstractExtraction;
class Session;
class Limit;


class Data_API Statement
	/// A Statement is used to execute SQL statements. 
	/// It does not contain code of its own.
	/// Its main purpose is to forward calls to the concrete StatementImpl stored inside.
	/// Statement execution can be synchronous or asynchronous.
	/// Synchronous ececution is achieved through execute() call, while asynchronous is
	/// achieved through executeAsync() method call.
	/// An asynchronously executing statement should not be copied during the execution. 
	/// Copying is not prohibited, however the benefits of the asynchronous call shall 
	/// be lost for that particular call since the synchronizing call shall internally be 
	/// called in the copy constructor.
	/// 
	/// For example, in the following case, although the execution is asyncronous, the
	/// synchronization part happens in the copy constructor, so the asynchronous nature 
	/// of the statement is lost for the end user:
	/// 
	///		Statement stmt = (session << "SELECT * FROM Table", async, now);
	/// 
	/// while in this case it is preserved:
	/// 
	///		Statement stmt = session << "SELECT * FROM Table", async, now;
	/// 
	/// There are two ways to preserve the asynchronous nature of a statement:
	/// 
	/// 1) Call executeAsync() method directly:
	///
	///		Statement stmt = session << "SELECT * FROM Table"; // no execution yet
	///		stmt.executeAsync(); // asynchronous execution
	///		// do something else ...
	///		stmt.wait(); // synchronize
	///
	/// 2) Ensure asynchronous execution through careful syntax constructs:
	/// 
	///		Statement stmt(session);
	///		stmt = session << "SELECT * FROM Table", async, now;
	///		// do something else ...
	///		stmt.wait(); // synchronize
	///
	/// Note:
	///
	/// Once set as asynchronous through 'async' manipulator, statement remains
	/// asynchronous for all subsequent execution calls, both execute() and executeAsync().
	/// However, calling executAsync() on a synchronous statement shall execute 
	/// asynchronously but without altering the underlying statement's synchronous nature.
	///
	/// Once asyncronous, a statement can be reverted back to synchronous state in two ways:
	/// 
	///	1) By calling setAsync(false)
	///	2) By means of 'sync' or 'reset' manipulators
	///
	/// See individual functions documentation for more details.
	///
{
public:
	typedef void (*Manipulator)(Statement&);

	typedef Poco::UInt32                                  ResultType;
	typedef ActiveResult<ResultType>                      Result;
	typedef SharedPtr<Result>                             ResultPtr;
	typedef ActiveMethod<ResultType, void, StatementImpl> AsyncExecMethod;
	typedef SharedPtr<AsyncExecMethod>                    AsyncExecMethodPtr;

	enum Storage
	{
		STORAGE_DEQUE   = StatementImpl::STORAGE_DEQUE_IMPL,
		STORAGE_VECTOR  = StatementImpl::STORAGE_VECTOR_IMPL,
		STORAGE_LIST    = StatementImpl::STORAGE_LIST_IMPL,
		STORAGE_UNKNOWN = StatementImpl::STORAGE_UNKNOWN_IMPL
	};

	Statement(StatementImpl* pImpl);
		/// Creates the Statement.

	explicit Statement(Session& session);
		/// Creates the Statement for the given Session.
		///
		/// The following:
		///
		///     Statement stmt(sess);
		///     stmt << "SELECT * FROM Table", ...
		///
		/// is equivalent to:
		/// 
		///     Statement stmt(sess << "SELECT * FROM Table", ...);
		///
		/// but in some cases better readable.

	~Statement();
		/// Destroys the Statement.

	Statement(const Statement& stmt);
		/// Copy constructor. 
		/// If the statement has been executed asynchronously and has not been
		/// synchronized prior to copy operation (i.e. is copied while executing), 
		/// this constructor shall synchronize it.

	Statement& operator = (const Statement& stmt);
		/// Assignment operator.

	void swap(Statement& other);
		/// Swaps the statement with another one.

	template <typename T> 
	Statement& operator << (const T& t)
		/// Concatenates data with the SQL statement string.
	{
		_pImpl->add(t);
		return *this;
	}

	Statement& operator , (Manipulator manip);
		/// Handles manipulators, such as now.

	Statement& operator , (AbstractBinding* info);
		/// Registers the Binding at the Statement

	Statement& operator , (AbstractExtraction* extract);
		/// Registers objects used for extracting data at the Statement.
		/// the position argument is used by connectors that support multilple
		/// recordsets to specify which recordset this extraction belongs to.

	Statement& operator , (const Limit& extrLimit);
		/// Sets a limit on the maximum number of rows a select is allowed to return.
		///
		/// Set per default to zero to Limit::LIMIT_UNLIMITED, which disables the limit.

	Statement& operator , (const Range& extrRange);
		/// Sets a an extraction range for the maximum number of rows a select is allowed to return.
		///
		/// Set per default to Limit::LIMIT_UNLIMITED which disables the range.

	Statement& operator , (const Step& extrStep);
		/// Sets a an extraction step (the number of rows a select is allowed to return 
		/// on every fetch attempt).
		///
		/// Set per default to Step::DEFAULT_STEP (1 row at a time).

	std::string toString() const;
		/// Creates a string from the accumulated SQL statement

	ResultType execute();
		/// Executes the statement synchronously or asynchronously. 
		/// Stops when either a limit is hit or the whole statement was executed.
		/// Returns the number of rows extracted from the database.
		/// If isAsync() returns  true, the statement is executed asynchronously 
		/// and the return value from this function is zero.
		/// The number of extracted rows from the query can be obtained by calling 
		/// wait().

	const Result& executeAsync();
		/// Executes the statement asynchronously. 
		/// Stops when either a limit is hit or the whole statement was executed.
		/// Returns immediately. For statements returning data, the number of rows extracted is 
		/// available by calling wait() method on either the returned value or the statement itself.
		/// When executed on a synchronous statement, this method does not alter the
		/// statement's synchronous nature.

	void setAsync(bool async = true);
		/// Sets the asynchronous flag. If this flag is true, executeAsync() is called 
		/// from the now() manipulator. This setting does not affect the statement's
		/// capability to be executed synchronously by directly calling execute().

	bool isAsync() const;
		/// Returns true if statement was marked for asynchronous execution.

	Statement::ResultType wait(long milliseconds = WAIT_FOREVER);
		/// Waits for the execution completion for asynchronous statements or
		/// returns immediately for synchronous ones. The return value for 
		/// asynchronous statement is the execution result (i.e. number of 
		/// rows retrieved). For synchronous statements, the return value is zero.

	bool initialized();
		/// Returns true if the statement was initialized (i.e. not executed yet).

	bool paused();
		/// Returns true if the statement was paused (a range limit stopped it
		/// and there is more work to do).

	bool done();
		/// Returns true if the statement was completely executed or false if a range limit stopped it
		/// and there is more work to do. When no limit is set, it will always return true after calling execute().

	Statement& reset(Session& session);
		/// Resets the Statement so that it can be filled with a new SQL command.

	bool canModifyStorage();
		/// Returns true if statement is in a state that allows the internal storage to be modified.

	Storage storage() const;
		/// Returns the internal storage type for the stamement.

	void setStorage(const std::string& storage);
		/// Sets the internal storage type for the stamement.

	const std::string& getStorage() const;
		/// Returns the internal storage type for the stamement.

	std::size_t extractionCount() const;
		/// Returns the number of extraction storage buffers associated
		/// with the statement.

protected:
	const AbstractExtractionVec& extractions() const;
		/// Returns the extractions vector.

	const MetaColumn& metaColumn(std::size_t pos) const;
		/// Returns the type for the column at specified position.

	const MetaColumn& metaColumn(const std::string& name) const;
		/// Returns the type for the column with specified name.

	 bool isNull(std::size_t col, std::size_t row) const;
		/// Returns true if the current row value at column pos is null.

private:
	typedef Poco::SharedPtr<StatementImpl> StatementImplPtr;

	static const int WAIT_FOREVER = -1;

	const Result& doAsyncExec();
		/// Asynchronously executes the statement.

	StatementImplPtr _pImpl;

	// asynchronous execution related members
	bool               _async;
	mutable ResultPtr  _pResult;
	Mutex              _mutex;
	AsyncExecMethodPtr _pAsyncExec;
};


//
// Manipulators
//

void Data_API now(Statement& statement);
	/// Enforces immediate execution of the statement.
	/// If _isAsync flag has been set, execution is invoked asynchronously.


void Data_API sync(Statement& statement);
	/// Sets the _isAsync flag to false, signalling synchronous execution.
	/// Synchronous execution is default, so specifying this manipulator
	/// only makes sense if async() was called for the statement before.


void Data_API async(Statement& statement);
	/// Sets the _isAsync flag to true, signalling asynchronous execution.


void Data_API deque(Statement& statement);
	/// Sets the internal storage to std::deque.
	/// std::deque is default storage, so specifying this manipulator
	/// only makes sense if list() or deque() were called for the statement before.


void Data_API vector(Statement& statement);
	/// Sets the internal storage to std::vector.
	

void Data_API list(Statement& statement);
	/// Sets the internal storage to std::list.


void Data_API reset(Statement& statement);
	/// Sets all internal settings to their respective default values.


//
// inlines
//

inline Statement& Statement::operator , (Manipulator manip)
{
	manip(*this);
	return *this;
}


inline Statement& Statement::operator , (AbstractBinding* info)
{
	_pImpl->addBinding(info);
	return *this;
}


inline Statement& Statement::operator , (AbstractExtraction* extract)
{
	_pImpl->addExtract(extract);
	return *this;
}


inline Statement& Statement::operator , (const Limit& extrLimit)
{
	_pImpl->setExtractionLimit(extrLimit);
	return *this;
}


inline Statement& Statement::operator , (const Range& extrRange)
{
	_pImpl->setExtractionLimit(extrRange.lower());
	_pImpl->setExtractionLimit(extrRange.upper());
	return *this;
}


inline Statement& Statement::operator , (const Step& extrStep)
{
	_pImpl->setStep(extrStep.value());
	return *this;
}


inline std::string Statement::toString() const
{
	return _pImpl->toString();
}


inline const AbstractExtractionVec& Statement::extractions() const
{
	return _pImpl->extractions();
}


inline const MetaColumn& Statement::metaColumn(std::size_t pos) const
{
	return _pImpl->metaColumn(static_cast<UInt32>(pos));
}


inline const MetaColumn& Statement::metaColumn(const std::string& name) const
{
	return _pImpl->metaColumn(name);
}


inline void Statement::setStorage(const std::string& storage)
{
	_pImpl->setStorage(storage);
}


inline std::size_t Statement::extractionCount() const
{
	return _pImpl->extractionCount();
}


inline Statement::Storage Statement::storage() const
{
	return static_cast<Storage>(_pImpl->getStorage());
}


inline bool Statement::canModifyStorage()
{
	return (0 == extractionCount()) && (initialized() || done());
}


inline bool Statement::initialized()
{
	return _pImpl->getState() == StatementImpl::ST_INITIALIZED;
}


inline bool Statement::paused()
{
	return _pImpl->getState() == StatementImpl::ST_PAUSED;
}


inline bool Statement::done()
{
	return _pImpl->getState() == StatementImpl::ST_DONE;
}


inline bool Statement::isNull(std::size_t col, std::size_t row) const
{
	return _pImpl->isNull(col, row);
}


inline bool Statement::isAsync() const
{
	return _async;
}


inline void swap(Statement& s1, Statement& s2)
{
	s1.swap(s2);
}


} } // namespace Poco::Data


namespace std
{
	template<>
	inline void swap<Poco::Data::Statement>(Poco::Data::Statement& s1, 
		Poco::Data::Statement& s2)
		/// Full template specalization of std:::swap for Statement
	{
		s1.swap(s2);
	}
}


#endif // Data_Statement_INCLUDED
