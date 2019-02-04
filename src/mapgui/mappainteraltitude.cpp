/*****************************************************************************
* Copyright 2015-2019 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "mapgui/mappainteraltitude.h"

#include "mapgui/mapscale.h"
#include "mapgui/maplayer.h"
#include "query/mapquery.h"
#include "common/mapcolors.h"
#include "mapgui/mapwidget.h"
#include "util/paintercontextsaver.h"
#include "navapp.h"
#include "atools.h"
#include "fs/common/morareader.h"

#include <QElapsedTimer>

#include <marble/GeoDataLineString.h>
#include <marble/GeoPainter.h>

using namespace Marble;
using namespace atools::geo;
using map::MapIls;

MapPainterAltitude::MapPainterAltitude(MapWidget *mapWidget, MapScale *mapScale)
  : MapPainter(mapWidget, mapScale)
{
}

MapPainterAltitude::~MapPainterAltitude()
{
}

void MapPainterAltitude::render(PaintContext *context)
{
  using atools::fs::common::MoraReader;

  if(!context->objectDisplayTypes.testFlag(map::MINIMUM_ALTITUDE))
    return;

  if(context->mapLayer->isMinimumAltitude())
  {
    MoraReader *moraReader = NavApp::getMoraReader();
    if(moraReader->isDataAvailable())
    {
      atools::util::PainterContextSaver paintContextSaver(context->painter);

      context->painter->setPen(mapcolors::minimumAltitudeGridPen);

      // Get covered one degree coordinate rectangles
      const GeoDataLatLonBox& curBox = context->viewport->viewLatLonAltBox();
      int west = static_cast<int>(curBox.west(DEG));
      int east = static_cast<int>(curBox.east(DEG));
      int north = static_cast<int>(curBox.north(DEG));
      int south = static_cast<int>(curBox.south(DEG));

      // Split at anit-meridian if needed
      QVector<std::pair<int, int> > ranges;
      if(west <= east)
        ranges.append(std::make_pair(west - 1, east));
      else
      {
        ranges.append(std::make_pair(west - 1, 179));
        ranges.append(std::make_pair(-180, east));
      }

      // Altitude values
      QVector<int> altitudes;
      // Minimum rectangle width on screen in pixel
      float minWidth = std::numeric_limits<float>::max();
      // Center points for rectangles for text placement
      QVector<GeoDataCoordinates> centers;

      // Draw rectangles and collect other values for text placement ================================
      Marble::GeoDataLineString line(Marble::Tessellate | Marble::RespectLatitudeCircle);
      for(int laty = south; laty <= north + 1; laty++)
      {
        // Iterate over anti-meridian split
        for(const std::pair<int, int>& range : ranges)
        {
          for(int lonx = range.first; lonx <= range.second; lonx++)
          {
            int moraFt100 = moraReader->getMoraFt(lonx, laty);
            if(moraFt100 > 10 && moraFt100 != MoraReader::OCEAN && moraFt100 != MoraReader::UNKNOWN &&
               moraFt100 != MoraReader::ERROR)
            {
              // Build rectangle
              line.clear();
              line.append(GeoDataCoordinates(lonx, laty, 0, DEG));
              line.append(GeoDataCoordinates(lonx + 1, laty, 0, DEG));
              line.append(GeoDataCoordinates(lonx + 1, laty - 1, 0, DEG));
              line.append(GeoDataCoordinates(lonx, laty - 1, 0, DEG));
              line.append(GeoDataCoordinates(lonx, laty, 0, DEG));
              context->painter->drawPolyline(line);

              if(!context->drawFast)
              {
                // Calculate rectangle screen width
                bool visibleDummy;
                QPointF leftPt = wToSF(GeoDataCoordinates(lonx, laty - .5, 0, DEG), DEFAULT_WTOS_SIZE, &visibleDummy);
                QPointF rightPt =
                  wToSF(GeoDataCoordinates(lonx + 1., laty - .5, 0, DEG), DEFAULT_WTOS_SIZE, &visibleDummy);

                minWidth = std::min(static_cast<float>(QLineF(leftPt, rightPt).length()), minWidth);

                centers.append(GeoDataCoordinates(lonx + .5, laty - .5, 0, DEG));
                altitudes.append(moraFt100);
              }
            }
          } // for(int lonx = range.first; lonx <= range.second; lonx++)
        } // for(const std::pair<int, int>& range : ranges)
      } // for(int laty = south; laty <= north + 1; laty++)

      // Draw texts =================================================================
      if(!context->drawFast && minWidth > 20.f)
      {
        // Adjust minmum and maximum font height based on rectangle width
        minWidth = std::max(minWidth * 0.6f, 25.f);
        minWidth = std::min(minWidth * 0.6f, 150.f);

        context->painter->setPen(QPen(mapcolors::minimumAltitudeNumberColor, 1.f));
        QFont font = context->painter->font();
        font.setItalic(true);
        font.setPixelSize(atools::roundToInt(minWidth));
        context->painter->setFont(font);
        QFontMetricsF fontmetrics = context->painter->fontMetrics();

        // Draw big thousands numbers ===============================
        bool visible, hidden;
        QVector<double> xpos;
        for(int i = 0; i < centers.size(); i++)
        {
          QString txt = QString::number(altitudes.at(i) / 10);
          QPointF pt = wToSF(centers.at(i), DEFAULT_WTOS_SIZE, &visible, &hidden);

          qreal w = fontmetrics.width(txt);
          pt += QPointF(-w * 0.7, fontmetrics.height() / 2. - fontmetrics.descent());
          xpos.append(pt.x() + w);
          if(!hidden)
            context->painter->drawText(pt, txt);
        }

        // Draw smaller hundreds numbers ==============================
        font.setPixelSize(font.pixelSize() * 7 / 10);
        context->painter->setFont(font);
        fontmetrics = context->painter->fontMetrics();

        for(int i = 0; i < centers.size(); i++)
        {
          QString txt = QString::number(altitudes.at(i) / 10);
          QPointF pt = wToSF(centers.at(i), DEFAULT_WTOS_SIZE, &visible, &hidden);

          if(!hidden)
          {
            pt += QPointF(0., fontmetrics.height() - fontmetrics.descent());
            pt.setX(xpos.at(i));
            context->painter->drawText(pt, QString::number(altitudes.at(i) - (altitudes.at(i) / 10 * 10)));
          }
        }
      } // if(!context->drawFast)
    } // if(moraReader->isDataAvailable())
  } // if(context->mapLayer->isMinimumAltitude())
}
