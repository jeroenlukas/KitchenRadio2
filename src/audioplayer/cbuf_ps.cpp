/*
 cbuf.cpp - Circular buffer implementation
 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "audioplayer/cbuf_ps.h"
#include "esp_system.h"
#include "esp_himem.h"
#include "esp_heap_caps.h"

cbuf_ps::cbuf_ps(size_t size) : next(NULL), _size(size + 1), _buf(new char[size + 1]), _bufend(_buf + size + 1), _begin(_buf), _end(_begin)
{
}

cbuf_ps::~cbuf_ps()
{
	delete[] _buf;
}

size_t cbuf_ps::resizeAdd(size_t addSize)
{
	return resize(_size + addSize);
}

// Duplicated from esp32-hal-psram.c
void IRAM_ATTR *ps_malloc(size_t size){
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

size_t
cbuf_ps::resize(size_t newSize)
{
	size_t bytes_available = available();
	newSize += 1;
	// not lose any data
	// if data can be lost use remove or flush before resize
	if ((newSize < bytes_available) || (newSize == _size))
	{
		return _size;
	}

	// RSB Use PSRAM here if required
	char *newbuf;
	if (BOARD_HAS_PSRAM)
	{
		newbuf = (char *)ps_malloc(newSize);
	}
	else
	{
		newbuf = new char[newSize];
	}
	char *oldbuf = _buf;

	if (!newbuf)
	{
		return _size;
	}

	if (_buf)
	{
		read(newbuf, bytes_available);
		memset((newbuf + bytes_available), 0x00, (newSize - bytes_available));
	}

	_begin = newbuf;
	_end = newbuf + bytes_available;
	_bufend = newbuf + newSize;
	_size = newSize;

	_buf = newbuf;
	delete[] oldbuf;

	return _size;
}

size_t cbuf_ps::available() const
{
	if (_end >= _begin)
	{
		return _end - _begin;
	}
	return _size - (_begin - _end);
}

size_t cbuf_ps::size()
{
	return _size;
}

size_t cbuf_ps::room() const
{
	if (_end >= _begin)
	{
		return _size - (_end - _begin) - 1;
	}
	return _begin - _end - 1;
}

int cbuf_ps::peek()
{
	if (empty())
	{
		return -1;
	}

	return static_cast<int>(*_begin);
}

size_t cbuf_ps::peek(char *dst, size_t size)
{
	size_t bytes_available = available();
	size_t size_to_read = (size < bytes_available) ? size : bytes_available;
	size_t size_read = size_to_read;
	char *begin = _begin;
	if (_end < _begin && size_to_read > (size_t)(_bufend - _begin))
	{
		size_t top_size = _bufend - _begin;
		memcpy(dst, _begin, top_size);
		begin = _buf;
		size_to_read -= top_size;
		dst += top_size;
	}
	memcpy(dst, begin, size_to_read);
	return size_read;
}

int cbuf_ps::read()
{
	if (empty())
	{
		return -1;
	}

	char result = *_begin;
	_begin = wrap_if_bufend(_begin + 1);
	return static_cast<int>(result);
}

size_t cbuf_ps::read(char *dst, size_t size)
{
	size_t bytes_available = available();
	size_t size_to_read = (size < bytes_available) ? size : bytes_available;
	size_t size_read = size_to_read;
	if (_end < _begin && size_to_read > (size_t)(_bufend - _begin))
	{
		size_t top_size = _bufend - _begin;
		memcpy(dst, _begin, top_size);
		_begin = _buf;
		size_to_read -= top_size;
		dst += top_size;
	}
	memcpy(dst, _begin, size_to_read);
	_begin = wrap_if_bufend(_begin + size_to_read);
	return size_read;
}

size_t cbuf_ps::write(char c)
{
	if (full())
	{
		return 0;
	}

	*_end = c;
	_end = wrap_if_bufend(_end + 1);
	return 1;
}

size_t cbuf_ps::write(const char *src, size_t size)
{
	size_t bytes_available = room();
	size_t size_to_write = (size < bytes_available) ? size : bytes_available;
	size_t size_written = size_to_write;
	if (_end >= _begin && size_to_write > (size_t)(_bufend - _end))
	{
		size_t top_size = _bufend - _end;
		memcpy(_end, src, top_size);
		_end = _buf;
		size_to_write -= top_size;
		src += top_size;
	}
	memcpy(_end, src, size_to_write);
	_end = wrap_if_bufend(_end + size_to_write);
	return size_written;
}

void cbuf_ps::flush()
{
	_begin = _buf;
	_end = _buf;
}

size_t cbuf_ps::remove(size_t size)
{
	size_t bytes_available = available();
	if (size >= bytes_available)
	{
		flush();
		return 0;
	}
	size_t size_to_remove = (size < bytes_available) ? size : bytes_available;
	if (_end < _begin && size_to_remove > (size_t)(_bufend - _begin))
	{
		size_t top_size = _bufend - _begin;
		_begin = _buf;
		size_to_remove -= top_size;
	}
	_begin = wrap_if_bufend(_begin + size_to_remove);
	return available();
}