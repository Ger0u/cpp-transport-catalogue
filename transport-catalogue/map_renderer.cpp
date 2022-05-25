#include "map_renderer.h"
#include <svg.pb.h>

#include <algorithm>
#include <variant>
using namespace std;

namespace map_renderer {
    
proto::Color ColorToProtoColor(const svg::Color& color) {
    proto::Color proto_color;

    if (const string* str = get_if<string>(&color)) {
        proto_color.set_str(*str);
    }
    else if (const svg::Rgb* rgb = get_if<svg::Rgb>(&color)) {
        proto::Rgb proto_rgb;

        proto_rgb.set_red(rgb->red);
        proto_rgb.set_green(rgb->green);
        proto_rgb.set_blue(rgb->blue);

        *proto_color.mutable_rgb() = move(proto_rgb);
    }
    else if (const svg::Rgba* rgba = get_if<svg::Rgba>(&color)) {
        proto::Rgba proto_rgba;

        proto_rgba.set_red(rgba->red);
        proto_rgba.set_green(rgba->green);
        proto_rgba.set_blue(rgba->blue);
        proto_rgba.set_opacity(rgba->opacity);

        *proto_color.mutable_rgba() = move(proto_rgba);
    }

    return proto_color;
}

svg::Color ProtoColorToColor(const proto::Color& proto_color) {
    if (!proto_color.str().empty()) {
        return proto_color.str();
    } else if (proto_color.has_rgb()) {
        return svg::Rgb{(uint8_t)proto_color.rgb().red(),
                        (uint8_t)proto_color.rgb().green(),
                        (uint8_t)proto_color.rgb().blue()};
    }
    else if (proto_color.has_rgba()) {
        return svg::Rgba{(uint8_t)proto_color.rgba().red(),
                         (uint8_t)proto_color.rgba().green(),
                         (uint8_t)proto_color.rgba().blue(),
                         proto_color.rgba().opacity()};
    }
    return {};
}

proto::Point PointToProtoPoint(const svg::Point& point) {
    proto::Point proto_point;

    proto_point.set_x(point.x);
    proto_point.set_y(point.y);

    return proto_point;
}

svg::Point ProtoPointToPoint(const proto::Point& proto_point) {
    return {proto_point.x(), proto_point.y()};
}

// ---------- VectorDrawables ------------------

proto::Drawables VectorDrawables::OutProto() const {
    proto::Drawables proto_drawables;

    for (int i = 0; i < drawables.size(); ++i) {
        proto::Drawable proto_drawable;

        if (const PolylineOfRoute* polyline_of_route = dynamic_cast<const PolylineOfRoute*>(drawables[i].get())) {
            *proto_drawable.mutable_polyline_of_route() = polyline_of_route->OutProto();
        } else if (const NameOfRoute* name_of_route = dynamic_cast<const NameOfRoute*>(drawables[i].get())) {
            *proto_drawable.mutable_name_of_route() = name_of_route->OutProto();
        }
        else if(const CircleOfStop* circle_of_stop = dynamic_cast<const CircleOfStop*>(drawables[i].get())) {
            *proto_drawable.mutable_circle_of_stop() = circle_of_stop->OutProto();
        }
        else if(const NameOfStop* name_of_stop = dynamic_cast<const NameOfStop*>(drawables[i].get())) {
            *proto_drawable.mutable_name_of_stop() = name_of_stop->OutProto();
        }

        proto_drawables.add_drawable();
        *proto_drawables.mutable_drawable(i) = move(proto_drawable);
    }

    return proto_drawables;
}

void VectorDrawables::InProto(const proto::Drawables& proto_drawables) {
    drawables.resize(proto_drawables.drawable_size());
    for (int i = 0; i < proto_drawables.drawable_size(); ++i) {
        const proto::Drawable& proto_drawable = proto_drawables.drawable(i);

        if (proto_drawable.has_polyline_of_route()) {
            drawables[i] = make_unique<PolylineOfRoute>();
            static_cast<PolylineOfRoute*>(drawables[i].get())->InProto(proto_drawable.polyline_of_route());
        } else if (proto_drawable.has_name_of_route()) {
            drawables[i] = make_unique<NameOfRoute>();
            static_cast<NameOfRoute*>(drawables[i].get())->InProto(proto_drawable.name_of_route());
        } else if (proto_drawable.has_circle_of_stop()) {
            drawables[i] = make_unique<CircleOfStop>();
            static_cast<CircleOfStop*>(drawables[i].get())->InProto(proto_drawable.circle_of_stop());
        } else if (proto_drawable.has_name_of_stop()) {
            drawables[i] = make_unique<NameOfStop>();
            static_cast<NameOfStop*>(drawables[i].get())->InProto(proto_drawable.name_of_stop());
        }
    }
}

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

proto::PolylineOfRoute PolylineOfRoute::OutProto() const {
    proto::PolylineOfRoute proto_polyline_of_route;

    for (int i = 0; i < stops_.size(); ++i) {
        proto_polyline_of_route.add_stop();
        *proto_polyline_of_route.mutable_stop(i) = PointToProtoPoint(stops_[i]);
    }
    proto_polyline_of_route.set_line_width(line_width_);
    *proto_polyline_of_route.mutable_color() = ColorToProtoColor(color_);

    return proto_polyline_of_route;
}

void PolylineOfRoute::InProto(const proto::PolylineOfRoute& proto_polyline_of_route) {
    stops_.resize(proto_polyline_of_route.stop_size());
    for (int i = 0; i < proto_polyline_of_route.stop_size(); ++i) {
        stops_[i] = ProtoPointToPoint(proto_polyline_of_route.stop(i));
    }
    line_width_ = proto_polyline_of_route.line_width();
    color_ = ProtoColorToColor(proto_polyline_of_route.color());
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

proto::NameOfRoute NameOfRoute::OutProto() const {
    proto::NameOfRoute proto_name_of_route;

    proto_name_of_route.set_name(name_);
    *proto_name_of_route.mutable_pos() = PointToProtoPoint(pos_);
    proto_name_of_route.set_label_font_size(label_font_size_);
    *proto_name_of_route.mutable_label_offset() = PointToProtoPoint(label_offset_);
    *proto_name_of_route.mutable_underlayer_color() = ColorToProtoColor(underlayer_color_);
    proto_name_of_route.set_underlayer_width(underlayer_width_);
    *proto_name_of_route.mutable_fill_color() = ColorToProtoColor(fill_color_);

    return proto_name_of_route;
}

void NameOfRoute::InProto(const proto::NameOfRoute& proto_name_of_route) {
    name_ = proto_name_of_route.name();
    pos_ = ProtoPointToPoint(proto_name_of_route.pos());
    label_font_size_ = proto_name_of_route.label_font_size();
    label_offset_ = ProtoPointToPoint(proto_name_of_route.label_offset());
    underlayer_color_ = ProtoColorToColor(proto_name_of_route.underlayer_color());
    underlayer_width_ = proto_name_of_route.underlayer_width();
    fill_color_ = ProtoColorToColor(proto_name_of_route.fill_color());
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

proto::CircleOfStop CircleOfStop::OutProto() const {
    proto::CircleOfStop proto_circle_of_stop;

    *proto_circle_of_stop.mutable_center() = PointToProtoPoint(center_);
    proto_circle_of_stop.set_radius(radius_);

    return proto_circle_of_stop;
}

void CircleOfStop::InProto(const proto::CircleOfStop& proto_circle_of_stop) {
    center_ = ProtoPointToPoint(proto_circle_of_stop.center());
    radius_ = proto_circle_of_stop.radius();
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

proto::NameOfStop NameOfStop::OutProto() const {
    proto::NameOfStop proto_name_of_stop;

    proto_name_of_stop.set_name(name_);
    *proto_name_of_stop.mutable_pos() = PointToProtoPoint(pos_);
    proto_name_of_stop.set_label_font_size(label_font_size_);
    *proto_name_of_stop.mutable_label_offset() = PointToProtoPoint(label_offset_);
    *proto_name_of_stop.mutable_underlayer_color() = ColorToProtoColor(underlayer_color_);
    proto_name_of_stop.set_underlayer_width(underlayer_width_);

    return proto_name_of_stop;
}

void NameOfStop::InProto(const proto::NameOfStop& proto_name_of_stop) {
    name_ = proto_name_of_stop.name();
    pos_ = ProtoPointToPoint(proto_name_of_stop.pos());
    label_font_size_ = proto_name_of_stop.label_font_size();
    label_offset_ = ProtoPointToPoint(proto_name_of_stop.label_offset());
    underlayer_color_ = ProtoColorToColor(proto_name_of_stop.underlayer_color());
    underlayer_width_ = proto_name_of_stop.underlayer_width();
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