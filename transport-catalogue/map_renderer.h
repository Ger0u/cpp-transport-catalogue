#pragma once

#include <map_renderer.pb.h>
#include "svg.h"
#include "domain.h"
#include "geo.h"
#include <string>
#include <iostream>
#include <memory>
#include <vector>
   
namespace map_renderer {

struct VectorDrawables {
    std::vector<std::unique_ptr<svg::Drawable>> drawables;

    proto::Drawables OutProto() const;
    void InProto(const proto::Drawables& proto_drawables);
};
    
class PolylineOfRoute : public svg::Drawable {
public:
    friend struct VectorDrawables;

    PolylineOfRoute() = default;
    PolylineOfRoute(std::vector<svg::Point> stops, double line_width, svg::Color color);
    
    void Draw(svg::ObjectContainer& container) const override;

    proto::PolylineOfRoute OutProto() const;
    void InProto(const proto::PolylineOfRoute& proto_polyline_of_route);
    
private:
    std::vector<svg::Point> stops_;
    double line_width_;
    svg::Color color_;
};
    
class NameOfRoute : public svg::Drawable {
public:
    friend struct VectorDrawables;

    NameOfRoute() = default;
    NameOfRoute(std::string name, svg::Point pos,
        int label_font_size, svg::Point label_offset,
        svg::Color underlayer_color, double underlayer_width,
        svg::Color fill_color);
    
    void Draw(svg::ObjectContainer& container) const override;

    proto::NameOfRoute OutProto() const;
    void InProto(const proto::NameOfRoute& proto_name_of_route);
    
private:
    std::string name_;
    svg::Point pos_;
    int label_font_size_;
    svg::Point label_offset_;
    svg::Color underlayer_color_;
    double underlayer_width_;
    svg::Color fill_color_;
};

class CircleOfStop : public svg::Drawable {
public:
    friend struct VectorDrawables;

    CircleOfStop() = default;
    CircleOfStop(svg::Point center, double radius);
    
    void Draw(svg::ObjectContainer& container) const override;

    proto::CircleOfStop OutProto() const;
    void InProto(const proto::CircleOfStop& proto_circle_of_stop);
    
private:
    svg::Point center_;
    double radius_;
};
   
class NameOfStop : public svg::Drawable {
public:
    friend struct VectorDrawables;

    NameOfStop() = default;
    NameOfStop(std::string name, svg::Point pos,
        int label_font_size, svg::Point label_offset,
        svg::Color underlayer_color, double underlayer_width);
    
    void Draw(svg::ObjectContainer& container) const override;

    proto::NameOfStop OutProto() const;
    void InProto(const proto::NameOfStop& proto_name_of_stop);
    
private:
    std::string name_;
    svg::Point pos_;
    int label_font_size_;
    svg::Point label_offset_;
    svg::Color underlayer_color_;
    double underlayer_width_;
};
    
struct RenderSettings {
    double width;
    double height;
    
    double padding;
    
    double line_width;
    double stop_radius;

    int bus_label_font_size;
    svg::Point bus_label_offset;

    int stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};
    
class ScalingPoints {
public:
    ScalingPoints() = default;
    ScalingPoints(double width, double height, double padding,
                  double min_lon, double max_lon,
                  double min_lat, double max_lat);
    
    svg::Point ScalePoint(const geo::Coordinates& coordinates) const;
   
private:
    double padding_;
    double min_lon_;
    double max_lat_;
    double zoom_coef_;
    
    static double SetZoomCoef(double width, double height, double padding,
                        double min_lon, double max_lon,
                        double min_lat, double max_lat);
};
    
} // namespace map_renderer