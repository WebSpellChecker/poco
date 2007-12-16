//
// Diagnostics.h
//
// $Id: //poco/1.3/Data/ODBC/include/Poco/Data/ODBC/Diagnostics.h#2 $
//
// Library: ODBC
// Package: ODBC
// Module:  Diagnostics
//
// Definition of Diagnostics.
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


#ifndef ODBC_Diagnostics_INCLUDED
#define ODBC_Diagnostics_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include <vector>
#ifdef POCO_OS_FAMILY_WINDOWS
#include <windows.h>
#endif
#include <sqlext.h>


namespace Poco {
namespace Data {
namespace ODBC {


template <typename H, SQLSMALLINT handleType>
class Diagnostics
	/// Utility class providing functionality for retrieving ODBC diagnostic
	/// records. Diagnostics object must be created with corresponding handle
	/// as constructor argument. During construction, diagnostic records with corresponding
	/// fields are populated and the object is ready for querying on various diagnostic fields.
{
public:

	static const unsigned int SQL_STATE_SIZE = SQL_SQLSTATE_SIZE + 1;
	static const unsigned int SQL_MESSAGE_LENGTH = SQL_MAX_MESSAGE_LENGTH + 1;
	static const unsigned int SQL_NAME_LENGTH = 128;
	static const std::string DATA_TRUNCATED;

	struct DiagnosticFields
	{
		/// SQLGetDiagRec fields
		POCO_SQLCHAR     _sqlState[SQL_STATE_SIZE];
		POCO_SQLCHAR     _message[SQL_MESSAGE_LENGTH];
		SQLINTEGER  _nativeError;
	};

	typedef std::vector<DiagnosticFields> FieldVec;
	typedef typename FieldVec::const_iterator Iterator;

	explicit Diagnostics(H& rHandle): _rHandle(rHandle)
		/// Creates and initializes the Diagnostics.
	{
		memset(_connectionName, 0, sizeof(_connectionName));
		memset(_serverName, 0, sizeof(_serverName));
		diagnostics();
	}

	~Diagnostics()
		/// Destroys the Diagnostics.
	{
	}

	std::string sqlState(int index) const
		/// Returns SQL state.
	{
		poco_assert (index < count());
		return std::string((char*) _fields[index]._sqlState);
	}

	std::string message(int index) const
		/// Returns error message.
	{
		poco_assert (index < count());
		return std::string((char*) _fields[index]._message);
	}

	long nativeError(int index) const
		/// Returns native error code.
	{
		poco_assert (index < count());
		return _fields[index]._nativeError;
	}

	std::string connectionName() const
		/// Returns the connection name. 
		/// If there is no active connection, connection name defaults to NONE.
		/// If connection name is not applicable for query context (such as when querying environment handle),
		/// connection name defaults to NOT_APPLICABLE.
	{
		return std::string((char*) _connectionName);
	}

	std::string serverName() const
		/// Returns the server name.
		/// If the connection has not been established, server name defaults to NONE.
		/// If server name is not applicable for query context (such as when querying environment handle),
		/// connection name defaults to NOT_APPLICABLE.
	{
		return std::string((char*) _serverName);
	}

	int count() const
		/// Returns the number of contained diagnostic records.
	{
		return (int) _fields.size();
	}

	void reset()
		/// Resets the diagnostic fields container.
	{
		_fields.clear();
	}

	const FieldVec& fields() const
	{
		return _fields;
	}

	const Iterator begin() const
	{
		return _fields.begin();
	}

	const Iterator end() const
	{
		return _fields.end();
	}

	const Diagnostics& diagnostics()
	{
		DiagnosticFields df;
		SQLSMALLINT count = 1;
		SQLSMALLINT messageLength = 0;
		static const std::string none = "None";
		static const std::string na = "Not applicable";

		reset();

		while (!Utility::isError(SQLGetDiagRec(handleType, 
			_rHandle, 
			count, 
			df._sqlState, 
			&df._nativeError, 
			df._message, 
			SQL_MESSAGE_LENGTH, 
			&messageLength))) 
		{
			if (1 == count)
			{
				poco_assert (sizeof(_connectionName) > none.length());
				poco_assert (sizeof(_connectionName) > na.length());
				poco_assert (sizeof(_serverName) > none.length());
				poco_assert (sizeof(_serverName) > na.length());

				// success of the following two calls is optional
				// (they fail if connection has not been established yet
				//  or return empty string if not applicable for the context)
				if (Utility::isError(SQLGetDiagField(handleType, 
					_rHandle, 
					count, 
					SQL_DIAG_CONNECTION_NAME, 
					_connectionName, 
					SQL_NAME_LENGTH, 
					&messageLength)))
						memcpy(_connectionName, none.c_str(), none.length());
				else if (0 == _connectionName[0]) 
						memcpy(_connectionName, na.c_str(), na.length());
				
				if (Utility::isError(SQLGetDiagField(handleType, 
					_rHandle, 
					count, 
					SQL_DIAG_SERVER_NAME, 
					_serverName, 
					SQL_NAME_LENGTH, 
					&messageLength)))
						memcpy(_serverName, none.c_str(), none.length());
				else if (0 == _serverName[0]) 
						memcpy(_serverName, na.c_str(), na.length());
			}

			_fields.push_back(df);

			memset(df._sqlState, 0, SQL_STATE_SIZE);
			memset(df._message, 0, SQL_MESSAGE_LENGTH);
			df._nativeError = 0;

			++count;
		}

		return *this;
	}

private:

	Diagnostics();

	/// SQLGetDiagField fields
	POCO_SQLCHAR _connectionName[SQL_NAME_LENGTH];
	POCO_SQLCHAR _serverName[SQL_NAME_LENGTH];

	/// Diagnostics container
	FieldVec _fields;

	/// Context handle
	H& _rHandle;
};


typedef Diagnostics<SQLHENV, SQL_HANDLE_ENV> EnvironmentDiagnostics;
typedef Diagnostics<SQLHDBC, SQL_HANDLE_DBC> ConnectionDiagnostics;
typedef Diagnostics<SQLHSTMT, SQL_HANDLE_STMT> StatementDiagnostics;
typedef Diagnostics<SQLHDESC, SQL_HANDLE_DESC> DescriptorDiagnostics;


} } } // namespace Poco::Data::ODBC


#endif
