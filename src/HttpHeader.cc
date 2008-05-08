/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "HttpHeader.h"
#include "Range.h"
#include "Util.h"
#include <istream>

namespace aria2 {

void HttpHeader::put(const std::string& name, const std::string& value) {
  std::multimap<std::string, std::string>::value_type vt(Util::toLower(name), value);
  table.insert(vt);
}

bool HttpHeader::defined(const std::string& name) const {
  return table.count(Util::toLower(name)) >= 1;
}

std::string HttpHeader::getFirst(const std::string& name) const {
  std::multimap<std::string, std::string>::const_iterator itr = table.find(Util::toLower(name));
  if(itr == table.end()) {
    return "";
  } else {
    return (*itr).second;
  }
}

std::deque<std::string> HttpHeader::get(const std::string& name) const {
  std::deque<std::string> v;
  std::string n(Util::toLower(name));
  for(std::multimap<std::string, std::string>::const_iterator i = table.find(n);
      i != table.end() && (*i).first == n; ++i) {
    v.push_back((*i).second);
  }
  return v;
}

unsigned int HttpHeader::getFirstAsUInt(const std::string& name) const {
  return getFirstAsULLInt(name);
}

uint64_t HttpHeader::getFirstAsULLInt(const std::string& name) const {
  std::string value = getFirst(name);
  if(value == "") {
    return 0;
  } else {
    return Util::parseULLInt(value);
  }
}

RangeHandle HttpHeader::getRange() const
{
  std::string rangeStr = getFirst("Content-Range");
  if(rangeStr == "") {
    std::string contentLengthStr = getFirst("Content-Length");
    if(contentLengthStr == "") {
      return SharedHandle<Range>(new Range());
    } else {
      uint64_t contentLength = Util::parseULLInt(contentLengthStr);
      if(contentLength == 0) {
	return SharedHandle<Range>(new Range());
      } else {
	return SharedHandle<Range>(new Range(0, contentLength-1, contentLength));
      }
    }
  }
  std::string byteRangeSpec;
  {
    // we expect that rangeStr looks like 'bytes 100-199/100'
    // but some server returns '100-199/100', omitting bytes-unit sepcifier
    // 'bytes'.
    std::pair<std::string, std::string> splist;
    Util::split(splist, rangeStr, ' ');
    if(splist.second.empty()) {
      // we assume bytes-unit specifier omitted.
      byteRangeSpec = splist.first;
    } else {
      byteRangeSpec = splist.second;
    }
  }
  std::pair<std::string, std::string> byteRangeSpecPair;
  Util::split(byteRangeSpecPair, byteRangeSpec, '/');

  std::pair<std::string, std::string> byteRangeRespSpecPair;
  Util::split(byteRangeRespSpecPair, byteRangeSpecPair.first, '-');

  off_t startByte = Util::parseLLInt(byteRangeRespSpecPair.first);
  off_t endByte = Util::parseLLInt(byteRangeRespSpecPair.second);
  uint64_t entityLength = Util::parseULLInt(byteRangeSpecPair.second);

  return SharedHandle<Range>(new Range(startByte, endByte, entityLength));
}

const std::string& HttpHeader::getResponseStatus() const
{
  return _responseStatus;
}

void HttpHeader::setResponseStatus(const std::string& responseStatus)
{
  _responseStatus = responseStatus;
}

const std::string& HttpHeader::getVersion() const
{
  return _version;
}

void HttpHeader::setVersion(const std::string& version)
{
  _version = version;
}

void HttpHeader::fill(std::istream& in)
{
  std::string line;
  while(std::getline(in, line)) {
    line = Util::trim(line);
    if(line.empty()) {
      break;
    }
    std::pair<std::string, std::string> hp;
    Util::split(hp, line, ':');
    put(hp.first, hp.second);
  }
}

} // namespace aria2
