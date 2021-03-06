/*
Copyright (c) 2007, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of Rasterbar Software nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef LIBTORRENT_BUFFER_HPP
#define LIBTORRENT_BUFFER_HPP

#include <memory>
#include <cstring>
#include "libtorrent/invariant_check.hpp"
#include "libtorrent/assert.hpp"

namespace libtorrent {

class buffer
{
public:
	struct interval
	{
	   interval(char* begin, char* end)
		  : begin(begin)
		  , end(end)
		{}

		char operator[](int index) const
		{
			TORRENT_ASSERT(begin + index < end);
			return begin[index];
		}
		  
		int left() const { TORRENT_ASSERT(end >= begin); return end - begin; }

		char* begin;
		char* end;
	};

	struct const_interval
	{
	   const_interval(char const* begin, char const* end)
		  : begin(begin)
		  , end(end)
		{}

		char operator[](int index) const
		{
			TORRENT_ASSERT(begin + index < end);
			return begin[index];
		}

		bool operator==(const const_interval& p_interval)
		{
			return (begin == p_interval.begin
				&& end == p_interval.end);
		}

		int left() const { TORRENT_ASSERT(end >= begin); return end - begin; }

		char const* begin;
		char const* end;
	};

	buffer(std::size_t n = 0)
		: m_begin(0)
		, m_end(0)
		, m_last(0)
	{
		if (n) resize(n);
	}

	buffer(buffer const& b)
		: m_begin(0)
		, m_end(0)
		, m_last(0)
	{
		if (b.size() == 0) return;
		resize(b.size());
		std::memcpy(m_begin, b.begin(), b.size());
	}

	buffer& operator=(buffer const& b)
	{
		resize(b.size());
		std::memcpy(m_begin, b.begin(), b.size());
		return *this;
	}

	~buffer()
	{
		::operator delete (m_begin);
	}

	buffer::interval data() { return interval(m_begin, m_end); }
	buffer::const_interval data() const { return const_interval(m_begin, m_end); }
	
	void resize(std::size_t n)
	{
		reserve(n);
		m_end = m_begin + n;
	}

	void insert(char* point, char const* first, char const* last)
	{
		std::size_t p = point - m_begin;
		if (point == m_end)
		{
			resize(size() + last - first);
			std::memcpy(m_begin + p, first, last - first);
			return;
		}
		
		resize(size() + last - first);
		std::memmove(m_begin + p + (last - first), m_begin + p, last - first);
		std::memcpy(m_begin + p, first, last - first);
	}

	void erase(char* begin, char* end)
	{
		TORRENT_ASSERT(end <= m_end);
		TORRENT_ASSERT(begin >= m_begin);
		TORRENT_ASSERT(begin <= end);
	 	if (end == m_end)
		{
			resize(begin - m_begin);
			return;
		}
		std::memmove(begin, end, m_end - end);
		m_end = begin + (m_end - end);
	 }

	void clear() { m_end = m_begin; }
	std::size_t size() const { return m_end - m_begin; }
	std::size_t capacity() const { return m_last - m_begin; }
	void reserve(std::size_t n)
	{
		if (n <= capacity()) return;
		TORRENT_ASSERT(n > 0);

		char* buf = (char*)::operator new(n);
		std::size_t s = size();
		std::memcpy(buf, m_begin, s);
		::operator delete (m_begin);
		m_begin = buf;
		m_end = buf + s;
		m_last = m_begin + n;
	}

	bool empty() const { return m_begin == m_end; }
	char& operator[](std::size_t i) { TORRENT_ASSERT(i < size()); return m_begin[i]; }
	char const& operator[](std::size_t i) const { TORRENT_ASSERT(i < size()); return m_begin[i]; }

	char* begin() { return m_begin; }
	char const* begin() const { return m_begin; }
	char* end() { return m_end; }
	char const* end() const { return m_end; }

	void swap(buffer& b)
	{
		using std::swap;
		swap(m_begin, b.m_begin);
		swap(m_end, b.m_end);
		swap(m_last, b.m_last);
	}
private:
	char* m_begin; // first
	char* m_end; // one passed end of size
	char* m_last; // one passed end of allocation
};


}

#endif // LIBTORRENT_BUFFER_HPP

