#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, Rgb color) {
    out << "rgb("sv << (unsigned short) color.red << ","sv << (unsigned short) color.green << ","sv
        << (unsigned short) color.blue << ")"sv;
    return out;
}

std::ostream& operator<<(std::ostream& out, Rgba color) {
    out << "rgba("sv << (unsigned short) color.red << ","sv << (unsigned short) color.green << ","sv
        << (unsigned short) color.blue << ","sv << color.opacity << ")"sv;
    return out;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    std::visit(ColorDataPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT :
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND :
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE :
            out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS :
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL :
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER :
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP :
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND :
            out << "round"sv;
            break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(svg::Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const svg::RenderContext& context) const {
    auto& out = context.out;

    out << "<polyline points=\""sv;
    if (!points_.empty()) {
        for (auto it = points_.begin(); it != points_.end(); ++it) {
            out << it->x << ","sv << it->y;
            if (!(next(it, 1) == points_.end())) {
                out << " "sv;
            }
        }
    }
    out << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Text& Text::SetPosition(svg::Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(svg::Point offset) {
    offset_ = offset;
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
    std::string_view raw_data(std::move(data));
    if (!raw_data.empty()) {
        for (const char c: raw_data) {
            switch (c) {
                case '\"':
                    data_ += "&quot;"s;
                    break;
                case '\'':
                    data_ += "&apos;"s;
                    break;
                case '<':
                    data_ += "&lt;"s;
                    break;
                case '>':
                    data_ += "&gt;"s;
                    break;
                case '&':
                    data_ += "&amp;"s;
                    break;
                default:
                    data_ += c;
            }
        }
    }
    return *this;
}

void Text::RenderObject(const svg::RenderContext& context) const {
    auto& out = context.out;

    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }
    if (!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }
    RenderAttrs(out);
    out << ">"sv;
    out << data_ << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    doc_data_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    if (!doc_data_.empty()) {
        for (const auto& obj_ptr: doc_data_) {
            obj_ptr->Render(ctx);
        }
    }
    out << "</svg>"sv;
}

}  // namespace svg