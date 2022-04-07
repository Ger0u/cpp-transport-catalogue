#include "svg.h"

namespace svg {

using namespace std::literals;

// ---------- Rgb ------------------
    
Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue)
: red(red)
, green(green)
, blue(blue) {
}
    
// ---------- Rgba ------------------
    
Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
: red(red)
, green(green)
, blue(blue)
, opacity(opacity) {
}
 
// ---------- ColorToOstream ------------------
    
void ColorToOstream::operator()(std::monostate) const {
    out << "none"sv;
}
    
void ColorToOstream::operator()(const std::string& str) const {
    out << str;
}
    
void ColorToOstream::operator()(Rgb rgb) const {
    out << "rgb("sv << std::to_string(rgb.red) << ","sv
        << std::to_string(rgb.green) << ","sv << std::to_string(rgb.blue) << ")"sv;
}
    
void ColorToOstream::operator()(Rgba rgba) const {
    out << "rgba("sv << std::to_string(rgba.red) << ","sv
        << std::to_string(rgba.green) << ","sv << std::to_string(rgba.blue)
        << ","sv << rgba.opacity << ")"sv;
}
    
// ---------- operator<< ------------------
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorToOstream{out}, color);
    return out;
}
    
std::ostream& operator<<(std::ostream& out, StrokeLineCap value) {
    out << (value == StrokeLineCap::BUTT ? "butt"sv
          : value == StrokeLineCap::ROUND ? "round"sv
          : value == StrokeLineCap::SQUARE ? "square"sv
          : ""sv);
    return out;
}
    
std::ostream& operator<<(std::ostream& out, StrokeLineJoin value) {
    out << (value == StrokeLineJoin::ARCS ? "arcs"sv
          : value == StrokeLineJoin::BEVEL ? "bevel"sv
          : value == StrokeLineJoin::MITER ? "miter"sv
          : value == StrokeLineJoin::MITER_CLIP ? "miter-clip"sv
          : value == StrokeLineJoin::ROUND ? "round"sv
          : ""sv);
    return out;
}
    
// ---------- Point ------------------
    
Point::Point(double x, double y)
: x(x)
, y(y) {
}
    
// ---------- RenderContext ------------------
   
RenderContext::RenderContext(std::ostream& out)
: out(out) {
}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent = 0)
: out(out)
, indent_step(indent_step)
, indent(indent) {
}

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}
    
// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}
    
// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ---------- Polyline ------------------
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (auto point = points_.begin(); point != points_.end(); ++point) {
        if (point != points_.begin()) {
            out << ' ';
        }
        out << point->x << ',' << point->y;
    }
    out << '\"';
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ---------- Text ------------------
    
Text& Text::SetPosition(Point pos) {
    pos_ = std::move(pos);
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv
        << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv
        << " font-size=\"" << font_size_ << '\"';
    if (!font_family_.empty()) {
        out << " font-family=\"" << font_family_ << '\"';
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\"" << font_weight_ << '\"';
    }
    //RenderAttrs(context.out);
    out << '>' << data_ << "</text>"sv;
} 
    
// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}
    
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext ctx(out, 2, 2);
    for (const auto& object : objects_) {
        object->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg