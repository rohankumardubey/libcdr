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
 * Copyright (C) 2012 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
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

#include "CDRTypes.h"
#include "CDRPath.h"
#include "libcdr_utils.h"

void libcdr::CDRTransform::applyToPoint(double &x, double &y) const
{
  double tmpX = m_v0*x + m_v1*y+m_x0;
  y = m_v3*x + m_v4*y+m_y0;
  x = tmpX;
}

void libcdr::CDRTransform::applyToArc(double &rx, double &ry, double &rotation, bool &sweep, double &x, double &y) const
{
  // First transform the end-point, which is the easiest
  applyToPoint(x, y);

  // represent ellipse as a transformed unit circle
  double v0 = m_v0*rx*cos(rotation) - m_v1*rx*sin(rotation);
  double v1 = m_v1*ry*cos(rotation) + m_v0*ry*sin(rotation);
  double v3 = m_v3*rx*cos(rotation) - m_v4*rx*sin(rotation);
  double v4 = m_v4*ry*cos(rotation) + m_v3*ry*sin(rotation);

  // centered implicit equation
  double A = v0*v0 + v1*v1;
  double C = v3*v3 + v4*v4;
  double B = 2.0*(v0*v3  +  v1*v4);

  double r1, r2;
  // convert implicit equation to angle and halfaxis:
  if (CDR_ALMOST_ZERO(B))
  {
    rotation = 0;
    r1 = A;
    r2 = C;
  }
  else
  {
    if (CDR_ALMOST_ZERO(A-C))
    {
      r1 = A + B / 2.0;
      r2 = A - B / 2.0;
      rotation = M_PI / 4.0;
    }
    else
    {
      double radical = 1.0 + B*B /((A-C)*(A-C));
      radical = radical < 0.0 ? 0.0 : sqrt (radical);

      r1 = (A+C + radical*(A-C)) / 2.0;
      r2 = (A+C - radical*(A-C)) / 2.0;
      rotation = atan2(B,(A-C)) / 2.0;
    }
  }

  // Prevent sqrt of a negative number, however small it might be.
  r1 = r1 < 0.0 ? 0.0 : sqrt(r1);
  r2 = r2 < 0.0 ? 0.0 : sqrt(r2);

  // now r1 and r2 are half-axis:
  if ((A-C) <= 0)
  {
    ry = r1;
    rx = r2;
  }
  else
  {
    ry = r2;
    rx = r1;
  }

  // sweep is inversed each time the arc is flipped
  sweep = (m_v0*m_v4 < m_v3*m_v1);
}

void libcdr::CDRPolygon::create(libcdr::CDRPath &path) const
{
  libcdr::CDRPath tmpPath(path);
  double step = 2*M_PI / (double)m_numAngles;
  if (m_numAngles % m_nextPoint)
  {
    libcdr::CDRTransform tmpTrafo(cos(m_nextPoint*step), sin(m_nextPoint*step), 0.0, -sin(m_nextPoint*step), cos(m_nextPoint*step), 0.0);
    for (unsigned i = 1; i < m_numAngles; ++i)
    {
      tmpPath.transform(tmpTrafo);
      path.appendPath(tmpPath);
    }
  }
  else
  {
    libcdr::CDRTransform tmpTrafo(cos(m_nextPoint*step), sin(m_nextPoint*step), 0.0, -sin(m_nextPoint*step), cos(m_nextPoint*step), 0.0);
    libcdr::CDRTransform tmpShift(cos(step), sin(step), 0.0, -sin(step), cos(step), 0.0);
    for (unsigned i = 0; i < m_nextPoint; ++i)
    {
      if (i)
      {
        tmpPath.transform(tmpShift);
        path.appendPath(tmpPath);
      }
      for (unsigned j=1; j < m_numAngles / m_nextPoint; ++j)
      {
        tmpPath.transform(tmpTrafo);
        path.appendPath(tmpPath);
      }
      path.appendClosePath();
    }
  }
  path.appendClosePath();
  libcdr::CDRTransform trafo(m_rx, 0.0, m_cx, 0.0, m_ry, m_cy);
  path.transform(trafo);
}

void libcdr::CDRSplineData::create(libcdr::CDRPath &path) const
{
  if (points.empty() || knotVector.empty())
    return;
  path.appendMoveTo(points[0].first, points[0].second);
  std::vector<std::pair<double, double> > tmpPoints;
  tmpPoints.push_back(points[0]);
  for (unsigned i = 1; i<points.size() && i<knotVector.size(); ++i)
  {
    tmpPoints.push_back(points[i]);
    if (knotVector[i])
    {
      if (tmpPoints.size() == 2)
        path.appendLineTo(tmpPoints[1].first, tmpPoints[1].second);
      else if (tmpPoints.size() == 3)
        path.appendQuadraticBezierTo(tmpPoints[1].first, tmpPoints[1].second,
                                     tmpPoints[2].first, tmpPoints[3].second);
      else
        path.appendSplineTo(tmpPoints);
      tmpPoints.clear();
      tmpPoints.push_back(points[i]);
    }
  }
  if (tmpPoints.size() == 2)
    path.appendLineTo(tmpPoints[1].first, tmpPoints[1].second);
  else if (tmpPoints.size() == 3)
    path.appendQuadraticBezierTo(tmpPoints[1].first, tmpPoints[1].second,
                                 tmpPoints[2].first, tmpPoints[3].second);
  else if (tmpPoints.size() > 3)
    path.appendSplineTo(tmpPoints);
}

libcdr::CDRCMYKColor::CDRCMYKColor(unsigned colorValue, bool percentage)
  : c((double)(colorValue & 0xff) / (percentage ? 100.0 : 255.0)),
    m((double)((colorValue & 0xff00) >> 8) / (percentage ? 100.0 : 255.0)),
    y((double)((colorValue & 0xff0000) >> 16) / (percentage ? 100.0 : 255.0)),
    k((double)((colorValue & 0xff000000) >> 24) / (percentage ? 100.0 : 255.0))
{
}

void libcdr::CDRCMYKColor::applyTint(double tint)
{
  if (tint < 0.0)
    tint = 0.0;
  if (tint > 1.0)
    tint = 1.0;
  c *= tint;
  m *= tint;
  y *= tint;
  k *= tint;
}

unsigned libcdr::CDRCMYKColor::getColorValue() const
{
  return 0;
}


libcdr::CDRRGBColor::CDRRGBColor(unsigned colorValue)
  : r((double)(colorValue & 0xff) / 255.0),
    g((double)((colorValue & 0xff00) >> 8) / 255.0),
    b((double)((colorValue & 0xff0000) >> 16) / 255.0)
{
}

void libcdr::CDRRGBColor::applyTint(double tint)
{
  if (tint < 0.0)
    tint = 0.0;
  if (tint > 1.0)
    tint = 1.0;
  r = 1.0 + r*tint - tint;
  g = 1.0 + g*tint - tint;
  b = 1.0 + b*tint - tint;
}

unsigned libcdr::CDRRGBColor::getColorValue() const
{
  return 0;
}


libcdr::CDRLab2Color::CDRLab2Color(unsigned colorValue)
  : L((double)(colorValue & 0xff)*100.0/255.0),
    a((double)(signed char)((colorValue & 0xff00) >> 8)),
    b((double)(signed char)((colorValue & 0xff0000) >> 16))
{
}

void libcdr::CDRLab2Color::applyTint(double tint)
{
  if (tint < 0.0)
    tint = 0.0;
  if (tint > 1.0)
    tint = 1.0;
  L = (1.0 - tint)*100.0 + tint*L;
  a *= tint;
  b *= tint;
}

unsigned libcdr::CDRLab2Color::getColorValue() const
{
  return 0;
}


libcdr::CDRLab4Color::CDRLab4Color(unsigned colorValue)
  : L((double)(colorValue & 0xff)*100.0/255.0),
    a((double)((signed char)(((colorValue & 0xff00) >> 8) - 0x80))),
    b((double)((signed char)(((colorValue & 0xff0000) >> 16) - 0x80)))
{
}

void libcdr::CDRLab4Color::applyTint(double tint)
{
  if (tint < 0.0)
    tint = 0.0;
  if (tint > 1.0)
    tint = 1.0;
  L = (1.0 - tint)*100.0 + tint*L;
  a *= tint;
  b *= tint;
}

unsigned libcdr::CDRLab4Color::getColorValue() const
{
  return 0;
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
