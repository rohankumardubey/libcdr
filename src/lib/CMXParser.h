/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* libcdr
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#ifndef __CMXPARSER_H__
#define __CMXPARSER_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <libwpd/libwpd.h>
#include <libwpd-stream/libwpd-stream.h>

namespace libcdr
{

class CDRCollector;

class CMXParser
{
public:
  explicit CMXParser(WPXInputStream *input, CDRCollector *collector);
  virtual ~CMXParser();
  bool parseRecords(WPXInputStream *input, unsigned level = 0);

private:
  CMXParser();
  CMXParser(const CMXParser &);
  CMXParser &operator=(const CMXParser &);
  bool parseRecord(WPXInputStream *input, unsigned level = 0);
  void readRecord(WPXString fourCC, unsigned length, WPXInputStream *input);

  void readCMXHeader(WPXInputStream *input);

  WPXInputStream *m_input;
  CDRCollector *m_collector;

  bool m_bigEndian;
  unsigned short m_coordSize;
  unsigned short m_unit;
  double m_scale;
  double m_xmin, m_xmax, m_ymin, m_ymax;
};
} // namespace libcdr

#endif // __CMXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */