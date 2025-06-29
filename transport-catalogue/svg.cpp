#include "svg.h"
#include <iomanip>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
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
    context.RenderIndent();
    out << "<circle cx=\""sv << std::setprecision(6) << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    context.RenderIndent();
    out << "<polyline points=\""sv;
    for (size_t i = 0; i < points_.size(); ++i) {
        out << std::setprecision(6) << points_[i].x << "," << points_[i].y;
        if (i != points_.size() - 1) {
            out << " ";
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

std::string Text::EscapeXml(const std::string& text) {
    std::string result;
    result.reserve(text.size() * 2); // Reserve space for potential escaping
    
    for (char c : text) {
        switch (c) {
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '&':
                result += "&amp;";
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    context.RenderIndent();
    out << "<text"sv;
    
    // Сначала рендерим атрибуты fill, stroke и связанные с ними
    RenderAttrs(out);
    
    // Затем рендерим позиционные атрибуты
    out << " x=\""sv << std::setprecision(6) << position_.x << "\" y=\""sv << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    
    // Затем рендерим атрибуты шрифта
    out << "font-size=\""sv << font_size_ << "\""sv;
    
    // Only output font-family if it was explicitly set
    if (this->font_family_.has_value()) {
        out << " font-family=\""sv << Text::EscapeXml(*this->font_family_) << "\""sv;
    }
    
    // Only output font-weight if it was explicitly set
    if (this->font_weight_.has_value()) {
        out << " font-weight=\""sv << Text::EscapeXml(*this->font_weight_) << "\""sv;
    }
    
    out << " >"sv << Text::EscapeXml(data_) << "</text>"sv;
}

// ---------- Document ------------------

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& obj : objects_) {
        obj->Render(RenderContext(out, 2).Indented());
    }
    out << "</svg>"sv;
}

void Document::RenderObject(const RenderContext& context) const {
    for (const auto& obj : objects_) {
        obj->Render(context);
    }
}

void Document::Draw(std::ostream& out) const {
    for (const auto& obj : objects_) {
        obj->Render(RenderContext(out));
    }
}

}  // namespace svg