#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<< (std::ostream& out, const Color color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }



    std::ostream& operator<< (std::ostream& out, const StrokeLineCap slc) {
        using namespace std::literals;
        switch (slc)
        {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;

        default:
            break;
        }
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const StrokeLineJoin slj) {
        using namespace std::literals;
        switch (slj)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;

        default:
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
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- PolyLine ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (!points_.empty()) {
            for (auto iter = points_.begin(); iter != points_.end() - 1; ++iter) {
                out << iter->x << ',' << iter->y << ' ';
            }
            out << points_.back().x << ',' << points_.back().y;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv
            << " x=\""sv << position_.x << '\"'
            << " y=\""sv << position_.y << '\"'

            << " dx=\""sv << offset_.x << '\"'
            << " dy=\""sv << offset_.y << '\"'

            << " font-size="sv << '\"' << size_ << '\"';

        if (!font_family_.empty()) {
            out << " font-family="sv << '\"' << font_family_ << '\"';
        }
        if (!font_weight_.empty()) {
            out << " font-weight="sv << '\"' << font_weight_ << '\"';
        }

        RenderAttrs(context.out);

        out << '>';

        for (const auto& ch : data_) {
            switch (ch)
            {
            case '\"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;

            default:
                out << ch;
                break;
            }
        }

        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_ptrs_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext cont(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv
            << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        for (const auto& obj : objects_ptrs_) {
            out << ' ' << ' ';//'\t';
            obj->Render(cont);
        }
        out << "</svg>"sv;
    }

}  // namespace svg