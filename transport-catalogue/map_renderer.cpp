#include "map_renderer.h"

#include <algorithm>
using namespace std;

namespace map_renderer {
    
// ---------- PolylineOfRoute ------------------
    
PolylineOfRoute::PolylineOfRoute(
    vector<svg::Point> stops, double line_width, svg::Color color)
: stops_(move(stops))
, line_width_(line_width)
, color_(move(color)) {
}
    
void PolylineOfRoute::Draw(svg::ObjectContainer& container) const {
    svg::Polyline route;
    for (const svg::Point& stop : stops_) {
        route.AddPoint(stop);
    }
    container.Add(move(route
                  .SetFillColor(svg::NoneColor)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                  .SetStrokeWidth(line_width_)
                  .SetStrokeColor(color_)));
}
    
// ---------- NameOfRoute ------------------

NameOfRoute::NameOfRoute(string name, svg::Point pos,
    int label_font_size, svg::Point label_offset,
    svg::Color underlayer_color, double underlayer_width,
    svg::Color fill_color)
: name_(move(name))
, pos_(move(pos))
, label_font_size_(label_font_size)
, label_offset_(move(label_offset))
, underlayer_color_(move(underlayer_color))
, underlayer_width_(underlayer_width)
, fill_color_(fill_color) {
}
    
void NameOfRoute::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Text()
                  .SetPosition(pos_)
                  .SetOffset(label_offset_)
                  .SetFontSize(label_font_size_)
                  .SetFontFamily("Verdana"s)
                  .SetFontWeight("bold"s)
                  .SetData(name_)
                  .SetFillColor(underlayer_color_)
                  .SetStrokeColor(underlayer_color_)
                  .SetStrokeWidth(underlayer_width_)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    container.Add(svg::Text()
                  .SetPosition(pos_)
                  .SetOffset(label_offset_)
                  .SetFontSize(label_font_size_)
                  .SetFontFamily("Verdana"s)
                  .SetFontWeight("bold"s)
                  .SetData(name_)
                  .SetFillColor(fill_color_));
}
    
// ---------- CircleOfStop ------------------
    
CircleOfStop::CircleOfStop(svg::Point center, double radius)
: center_(move(center))
, radius_(radius) {
}
    
void CircleOfStop::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Circle()
                  .SetCenter(center_)
                  .SetRadius(radius_)
                  .SetFillColor("white"s));
}
    
// ---------- NameOfStop ------------------
    
NameOfStop::NameOfStop(string name, svg::Point pos,
    int label_font_size, svg::Point label_offset,
    svg::Color underlayer_color, double underlayer_width)
: name_(move(name))
, pos_(move(pos))
, label_font_size_(label_font_size)
, label_offset_(move(label_offset))
, underlayer_color_(move(underlayer_color))
, underlayer_width_(underlayer_width) {
}
    
void NameOfStop::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Text()
                  .SetPosition(pos_)
                  .SetOffset(label_offset_)
                  .SetFontSize(label_font_size_)
                  .SetFontFamily("Verdana"s)
                  .SetData(name_)
                  .SetFillColor(underlayer_color_)
                  .SetStrokeColor(underlayer_color_)
                  .SetStrokeWidth(underlayer_width_)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    container.Add(svg::Text()
                  .SetPosition(pos_)
                  .SetOffset(label_offset_)
                  .SetFontSize(label_font_size_)
                  .SetFontFamily("Verdana"s)
                  .SetData(name_)
                  .SetFillColor("black"s));
}

// ---------- ScalingPoints ------------------
    
ScalingPoints::ScalingPoints(
    double width, double height, double padding,
    double min_lon, double max_lon,
    double min_lat, double max_lat)
: padding_(padding)
, min_lon_(min_lon)
, max_lat_(max_lat)
, zoom_coef_(SetZoomCoef(width, height, padding, min_lon, max_lon,
            min_lat, max_lat)) {
}
    
svg::Point ScalingPoints::ScalePoint(const geo::Coordinates& coordinates) const {
    return {(coordinates.lng - min_lon_) * zoom_coef_ + padding_,
            (max_lat_ - coordinates.lat) * zoom_coef_ + padding_}; 
}
    
double ScalingPoints::SetZoomCoef(
    double width, double height, double padding,
    double min_lon, double max_lon,
    double min_lat, double max_lat)
{
    if (max_lon == min_lon) {
        if (max_lat == min_lat) {
            return 0;
        } else {
            return (height - 2 * padding) / (max_lat - min_lat);
        }
    } else {
        if (max_lat == min_lat) {
            return (width - 2 * padding) / (max_lon - min_lon);
        } else {
            return min((width - 2 * padding) / (max_lon - min_lon),
                       (height - 2 * padding) / (max_lat - min_lat));
        }
    }
}
    
} // namespace map_renderer