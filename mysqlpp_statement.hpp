//
// Copyright (c) 2014-2015 WASPP (waspp.org@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MYSQLPP_STATEMENT_HPP
#define MYSQLPP_STATEMENT_HPP

#include <iomanip>
#include <vector>
#include <string>
#include <istream>
#include <sstream>
#include <limits>

#include <mysql/mysql.h>

#include "mysqlpp_exception.hpp"
#include "mysqlpp_result.hpp"

namespace mysqlpp
{

struct st_mysql_param
{
	st_mysql_param() : is_null_(0), length_(0), buffer_(0)
	{
	    field_type_ = MYSQL_TYPE_NULL;
	}

	void set(enum_field_types field_type, const char* begin, const char* end)
	{
		field_type_ = field_type;
		length_ = end - begin;
		buffer_ = const_cast<char*>(begin);
	}

	void set(enum_field_types field_type, const std::string& str)
	{
		field_type_ = field_type;
		value_ = str;
		length_ = value_.size();
		buffer_ =  const_cast<char*>(value_.c_str());
	}

	void make_bind(st_mysql_bind* bind)
	{
		if (field_type_ == MYSQL_TYPE_NULL)
		{
			is_null_ = 1;
			bind->is_null = &is_null_;

			bind->buffer_type = field_type_;
			return;
		}

		is_null_ = 0;
		bind->is_null = &is_null_;

		bind->buffer_type = field_type_;
		bind->buffer = buffer_;
		bind->buffer_length = length_;
		bind->length = &length_;
	}

	my_bool is_null_;
	enum_field_types field_type_;

	std::string value_;
	unsigned long length_;

	void *buffer_;
};

class statement
{
public:
	statement(st_mysql* mysql, const std::string& query);
	~statement();

	void param_null()
	{
		if (param_index == param_count)
		{
			throw exception("invalid param_index");
		}

		params[param_index] = st_mysql_param();
		++param_index;
	}

	void param(const std::string& value)
	{
		if (param_index == param_count)
		{
			throw exception("invalid param_index");
		}

		params[param_index].set(MYSQL_TYPE_STRING, value.c_str(), value.c_str() + value.size());
		++param_index;
	}

	void param(std::istream& value)
	{
		if (param_index == param_count)
		{
			throw exception("invalid param_index");
		}

		std::ostringstream ss;
		ss << value.rdbuf();

		params[param_index].set(MYSQL_TYPE_BLOB, ss.str());
		++param_index;
	}

	void param(signed char value)
	{
	    set_param(MYSQL_TYPE_TINY, value);
	}

	void param(short int value)
	{
	    set_param(MYSQL_TYPE_SHORT, value);
	}

	void param(int value)
	{
	    set_param(MYSQL_TYPE_LONG, value);
	}

	void param(long long int value)
	{
	    set_param(MYSQL_TYPE_LONGLONG, value);
	}

	void param(float value)
	{
	    set_param(MYSQL_TYPE_FLOAT, value);
	}

	void param(double value)
	{
	    set_param(MYSQL_TYPE_DOUBLE, value);
	}

	unsigned long long execute();
	result* execute_query();

private:
    template<typename T> void set_param(enum_field_types field_type, T value)
	{
		if (param_index == param_count)
		{
			throw exception("invalid param_index");
		}

		oss.str(std::string());
		if (!std::numeric_limits<T>::is_integer)
		{
			oss << std::setprecision(std::numeric_limits<T>::digits10 + 1);
		}
		oss << value;

		params[param_index].set(field_type, oss.str());
		++param_index;
	}

	st_mysql_stmt* stmt;

	int param_count;
	int param_index;

	std::vector<st_mysql_param> params;
	std::vector<st_mysql_bind> binds;

	std::ostringstream oss;
};

} // namespace mysqlpp

#endif // MYSQLPP_STATEMENT_HPP
