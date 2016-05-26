/*****************************************************************************
* Copyright 2015-2016 Alexander Barthel albar965@mailbox.org
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

#ifndef MAPHTMLINFOBUILDER_H
#define MAPHTMLINFOBUILDER_H

#include <QCoreApplication>
#include <QLocale>

class RouteMapObject;
class MorseCode;
class MapQuery;
class InfoQuery;
class WeatherReporter;
class RouteMapObjectList;
class HtmlBuilder;

namespace maptypes {
struct MapAirport;

struct MapVor;

struct MapNdb;

struct MapWaypoint;

struct MapAirway;

struct MapMarker;

struct MapAirport;

struct MapParking;

struct MapHelipad;

struct MapUserpoint;

}

namespace atools {
namespace sql {
class SqlRecord;
}
}

class MapHtmlInfoBuilder
{
  Q_DECLARE_TR_FUNCTIONS(MapHtmlInfoBuilder)

public:
  MapHtmlInfoBuilder(MapQuery *mapQuery, InfoQuery *infoDbQuery, bool formatInfo);

  virtual ~MapHtmlInfoBuilder();

  void airportText(const maptypes::MapAirport& airport, HtmlBuilder& html,
                   const RouteMapObjectList *routeMapObjects,
                   WeatherReporter *weather, QColor background);
  void comText(const maptypes::MapAirport& airport, HtmlBuilder& html, QColor background);

  void vorText(const maptypes::MapVor& vor, HtmlBuilder& html, QColor background);
  void ndbText(const maptypes::MapNdb& ndb, HtmlBuilder& html, QColor background);
  void waypointText(const maptypes::MapWaypoint& waypoint, HtmlBuilder& html, QColor background);

  void airwayText(const maptypes::MapAirway& airway, HtmlBuilder& html);
  void markerText(const maptypes::MapMarker& m, HtmlBuilder& html);
  void towerText(const maptypes::MapAirport& airport, HtmlBuilder& html);
  void parkingText(const maptypes::MapParking& parking, HtmlBuilder& html);
  void helipadText(const maptypes::MapHelipad& helipad, HtmlBuilder& html);
  void userpointText(const maptypes::MapUserpoint& userpoint, HtmlBuilder& html);

private:
  MapQuery *mapQuery;
  InfoQuery *infoQuery;
  MorseCode *morse;
  bool info;
  QLocale locale;
  void addScenery(const atools::sql::SqlRecord *rec, HtmlBuilder& html);
  void addCoordinates(const atools::sql::SqlRecord *rec, HtmlBuilder& html);
  void head(HtmlBuilder& html, const QString& text);

  void title(HtmlBuilder& html, const QString& text);

  void airportTitle(const maptypes::MapAirport& airport, HtmlBuilder& html, QColor background);

};

#endif // MAPHTMLINFOBUILDER
