#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace svg {

    struct Rgb {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1;
    };

    //using Color = std::string;
    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ "none" };

    std::ostream& operator<< (std::ostream& out, const Color color);


    class ColorPrinter {
    public:
        std::ostream& out;

        void operator()(std::monostate) const {
            out << NoneColor;
        }
        void operator()(std::string str) const {
            out << str;
        }
        void operator()(Rgb color) const {
            out << "rgb" << '(' << +color.red << ',' << +color.green << ',' << +color.blue << ")";
        }
        void operator()(Rgba color) const {
            out << "rgba" << '(' << +color.red << ',' << +color.green << ',' << +color.blue << ',' << +color.opacity << ")";
        }
    };

    /*class ColorPrinter {
    public:
        std::string operator()(std::monostate) const {
            return std::string("none");
        }
        std::string operator()(std::string str) const {
            return str;
        }
        Rgb operator()(Rgb color) const {
            return color;
        }
        Rgba operator()(Rgba color) const {
            return color;
        }
    };*/


    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<< (std::ostream& out, const StrokeLineCap slc);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<< (std::ostream& out, const StrokeLineJoin slj);

    class Object;

    class ObjectContainer {
    public:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
    protected:
        ~ObjectContainer() = default;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = std::move(line_join);
            return AsOwner();
        }
    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                visit(ColorPrinter{ out }, *fill_color_);
                //out << visit(ColorPrinter{}, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                visit(ColorPrinter{ out }, *stroke_color_);
                //out << visit(ColorPrinter{}, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    private:
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

        /*
         * ������ ������ � ������, ����������� ��� ���������� �������� <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);

        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(std::string data);

        // ������ ������ � ������, ����������� ��� ���������� �������� <text>
    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class Document : public ObjectContainer {
    public:
        /*
         ����� Add ��������� � svg-�������� ����� ������-��������� svg::Object.
         ������ �������������:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */
        // void Add(???);

        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;

        // ������ ������ � ������, ����������� ��� ���������� ������ Document

    private:
        std::vector<std::unique_ptr<Object>> objects_ptrs_;
    };

}  // namespace svg