#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cassert>

#ifndef M_PI
#define M_PI 3.1415926
#endif

// found: https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

namespace CMPUT350
{

// forward declarations
struct Point2D;
struct Rect;
struct Circle;
struct Line;

struct Point2D
{
    float x, y;
    Point2D(float x=0.f, float y=0.f)
    :x(x), y(y) {}
    float Distance(const Point2D &other) const
    {
        // Returns the signed Euclidean distance between this and other
        float dx = this->x - other.x;
        float dy = this->y - other.y;
        return sqrtf(dx*dx + dy*dy);
    }

    inline float Distance(const Line &l) const;

    void Rotate(const Point2D pivot, const float angle = 0.f)
    {   
        // Rotate point about pivot by angle radian
        // Translate rotation to be about the origin
        float x = this->x - pivot.x;
        float y = this->y - pivot.y;

        float cosine = cos(angle);
        float sine   = sin(angle);

        // rotate point
        float x_rot = x*cosine - y*sine;
        float y_rot = x*sine   + y*cosine;

        // Reverse translation
        this->x = x_rot + pivot.x;
        this->y = y_rot + pivot.y;
    }

    Point2D operator+(const Point2D &other) const
    {
        return Point2D( this->x + other.x, this->y + other.y );
    }
    Point2D operator+(const float &other) const
    {
        return Point2D( this->x + other, this->y + other );
    }
    Point2D operator-(const Point2D &other) const
    {
        return Point2D( this->x - other.x, this->y - other.y );
        
    }
    Point2D operator-(const float &other) const
    {
        return Point2D( this->x - other, this->y - other );
    }
    Point2D operator*(const float &scalar) const
    {
        return Point2D( this->x * scalar, this->y * scalar );
    }
    Point2D operator/(const float &scalar) const
    {
        return Point2D( this->x / scalar, this->y / scalar );
    }
    Point2D &operator+=(const float &scalar)
    {
        this->x = this->x + scalar;
        this->y = this->y + scalar;
        return *this;
    }
    Point2D &operator+=(const Point2D &other)
    {
        this->x = this->x + other.x;
        this->y = this->y + other.y;
        return *this;
    }
    Point2D &operator-=(const Point2D &other)
    {
        this->x = this->x - other.x;
        this->y = this->y - other.y;
        return *this;
    }
    bool operator==(const Point2D &other) const
    {
        return this->x == other.x && this->y == other.y;
    }
    Point2D &operator*=(const int &scalar)
    {
        this->x = this->x * scalar;
        this->y = this->y * scalar;
        return *this;
    }
    Point2D &operator/=(const int &scalar)
    {
        this->x = this->x / scalar;
        this->y = this->y / scalar;
        return *this;
    }
    float operator*(const Point2D &other) const
    {
        // equivalent to "Dot"
        return (this->x * other.x) + (this->y * other.y);
    }
    float Dot(Point2D b) const
    {
        return (this->x * b.x) + (this->y * b.y);
    }
    float Cross(Point2D b) const
    {
        // determinant of M = [this, b]
        return (this->x * b.y) - (this->y * b.x);
    }
    static float Dot(Point2D a, Point2D b)
    {
        return (a.x * b.x) + (a.y * b.y);
    }
    static float Cross(Point2D a, Point2D b)
    {
        return (a.x * b.y) - (a.y * b.x);
    }
    float length() const
    {
        float result = this->x*this->x + this->y*this->y;
        return sqrtf(result);
    }
    float lengthSq() const
    {
        return this->x*this->x + this->y*this->y;
    }
    void Normalize()
    {   
        // Sets vector ("point") to unit length
        float len = this->length();
        if (len != 0.0f)
        {
            this->x /= len;
            this->y /= len;
        }
    }
};

const Point2D ORIGIN(0.f, 0.f);

static std::ostream &operator<<(std::ostream &os, const Point2D &p) 
{
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

// added for line 84 in bouncing ball demo: float * point wasn't defined
inline Point2D operator*(const float& scalar, const Point2D& point) 
{
    return Point2D(scalar * point.x, scalar * point.y);
}

struct Rect
{
    Point2D topLeft;
    float width, height;

    Rect() : topLeft(ORIGIN), width(0.f), height(0.f) {}
    Rect(float left, float top, float width, float height)
    :topLeft(Point2D(left, top)), width(width), height(height)
    {}
    Rect(Point2D tl, float w,float h)
    :topLeft(tl), width(w), height(h)
    {}
    Rect(Point2D p1, Point2D p2)
    :topLeft(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
    width(fabs(p1.x-p2.x)), height(fabs(p1.y-p2.y))
    {}
    Rect(Point2D center, float radius)
    :topLeft(center.x-radius, center.y-radius),
    width(2*radius), height(2*radius)
    {}
    Rect(const Line& l);  // Defined after Line struct
    Rect &operator|=(const Rect &other)
    {
        /**
         * Union operator: returns the rectangle which fully encompases 
         * both this and other.
         * 
         * e.g  +-----+          +---------+
         *      |  +--+---+      |         |
         *      |  |  |   |  |=  |         |
         *      |  +--+---+      |         |
         *      +-----+ other    +---------+
         *       this              result
         * 
         * If either operand fully encloses the other, returns the larger of
         * the two
         */

        // Get bounds of result
        float top = std::min(this->topLeft.y, other.topLeft.y);
        float bottom = std::max(this->topLeft.y + this->height, other.topLeft.y + other.height);
        float left = std::min(this->topLeft.x, other.topLeft.x);
        float right = std::max(this->topLeft.x + this->width, other.topLeft.x + other.width);
        
        // Get width and height of result
        float width = right - left;
        float height = bottom - top;
        
        // Set members and return
        this->topLeft.y = top;
        this->topLeft.x = left;
        this->width = width;
        this->height = height;

        return *this;
        
    }
    Rect &operator|=(const Point2D &other)
    {
        /**
         * Union operation similar to above, but with a Point2D. Simply returns
         * this if the point is inside this, otherwise the Rect which contains
         * both this and the Point2D
         */

         if (this->IsInside(other))
         {
            return *this;
         }

         // Get bounds of result
        float top = std::min(this->topLeft.y, other.y);
        float bottom = std::max(this->topLeft.y + this->height, other.y);
        float left = std::min(this->topLeft.x, other.x);
        float right = std::max(this->topLeft.x + this->width, other.x);
        
        // Get width and height of result
        float width = right - left;
        float height = bottom - top;
        
        // Set members and return
        this->topLeft.y = top;
        this->topLeft.x = left;
        this->width = width;
        this->height = height;

        return *this;

    }
    Rect &operator|=(const Line &other); 
    Rect &operator&=(const Rect &other)
    {
        /**
         * Intersection operator: returns the rectangle fully encompasesed
         * by both this and other.
         * 
         * e.g  +-----+
         *      |  +--+----+      +--+
         *      |  |  |    |  &=  |  |
         *      |  +--+----+      +--+
         *      +-----+ other    result
         *       this
         * 
         * If there is no intersection, return a zero-rect i.e.
         * tl= {0,0}, w=0, h=0
         */

        // Ensure intersection exists, otherwise return zero-rect
        bool intersect =!(this->topLeft.x + this->width < other.topLeft.x     // to the right
                        || other.topLeft.x + other.width < this->topLeft.x    // to the left
                        || this->topLeft.y + this->height < other.topLeft.y   // above
                        || other.topLeft.y + other.height < this->topLeft.y); // below

        if ( !intersect )
        {
            *this = Rect(topLeft+Point2D(width, height), 0.f, 0.f);
            return *this;
        }

        // Get bounds of result
        float top = std::max(this->topLeft.y, other.topLeft.y);
        float bottom = std::min(this->topLeft.y + this->height, other.topLeft.y + other.height);
        float left = std::max(this->topLeft.x, other.topLeft.x);
        float right = std::min(this->topLeft.x + this->width, other.topLeft.x + other.width);
        
        // Get width and height of result
        float width = right - left;
        float height = bottom - top;
        
        // Set members and return
        this->topLeft.y = top;
        this->topLeft.x = left;
        this->width = width;
        this->height = height;

        return *this;
    }
    void Inset(float inset)
    {
        /**
         * Reduces the bounds of this by inset, with a min of 0.
         */
        
        // Get updated inset values with 0 as a lower bound on width and height
        this->topLeft.y += inset;
        this->topLeft.x += inset;
        this->width = std::max(this->width - (inset*2), 0.0f);
        this->height = std::max(this->height - (inset*2), 0.0f);
    }
    bool IsInside(const Point2D &p) const
    {
        // Checks if p is in this. Assumes boundary inclusion.
        bool in_x = p.x >= this->topLeft.x && p.x <= this->topLeft.x + this->width;
        bool in_y = p.y >= this->topLeft.y && p.y <= this->topLeft.y + this->height;
        return in_x && in_y;
    }
};

static std::ostream &operator<<(std::ostream &os, const Rect &r)
{
    os << "Rect(" << r.topLeft << ", w=" << r.width << ", h=" << r.height << ")";
    return os;
}

struct Circle
{
    Point2D center;
    float radius;

    Circle(Point2D center={0,0}, float radius=0)
    : center(center), radius(radius) {}

    Circle(float x, float y, float r)
    : center(x, y), radius(r) {}
};

static std::ostream &operator<<(std::ostream &os, const Circle &c)
{
    os << "Circle(" << c.center << ", r=" << c.radius << ")";
    return os;
}


struct Line
{
    Point2D p1, p2;
    float width;

    // Spec-required constructors
    Line(Point2D p1={0,0}, Point2D p2={0,0})
    : p1(p1), p2(p2), width(0.f) {}

    Line(float x1, float y1, float x2, float y2)
    : p1(x1, y1), p2(x2, y2), width(0.f) {}

    // Original constructors
    Line(Point2D p1, Point2D p2, float width)
    : p1(p1), p2(p2), width(width)
    {
        // Don't swap points - preserve the order passed in
        // Point swapping breaks the Crosses() algorithm
    }

    Line(Point2D p1, float length, float width = 0.f, float angle = 0.f)
    : p1(p1), width(width)
    {
        // Initialize p2 to have the same y as p1, the rotate it about p1
        Point2D _p2(p1.x+length, p1.y);
        _p2.Rotate(p1, angle);
        p2 = _p2;
    }

    // Spec-required methods
    float Length() const
    {
        return p1.Distance(p2);
    }

    Point2D ClosestPoint(const Point2D &p) const
    {
        // Find closest point on line segment to point p
        Point2D v = p2 - p1;
        Point2D t = p - p1;

        if (v.lengthSq() == 0) { return p1; }

        float proj_t = t.Dot(v) / v.lengthSq();
        proj_t = std::max(0.0f, std::min(1.0f, proj_t)); // clamp to [0,1]

        return Point2D(p1.x + proj_t * v.x, p1.y + proj_t * v.y);
    }

    bool Crosses(Line other, Point2D &crossingPoint) const
    {
        // Check if two line segments intersect and return crossing point
        // From Graphic Gems 3, Ch. 4.6: Faster Line Segment Intersection
        // (thank you to the Moving Out of Athabasca Book Bonanza)
        Point2D A = this->p2 - this->p1;
        Point2D B = other.p1 - other.p2;
        Point2D C = this->p1 - other.p1;

        float denom = B.Cross(A);
        // std::cout << __FILE__ << __LINE__ << "\tdenom: " << denom << "\n";
        // Collinear lines, handle fp error
        if (std::abs(denom) < 1e-10f) { return false; }

        float a_numerator = C.Cross(B);
        float b_numerator = A.Cross(C);
        // std::cout << __FILE__ << __LINE__ << "\ta numerator: " << a_numerator << "\n";
        // std::cout << __FILE__ << __LINE__ << "\tb numerator: " << b_numerator << "\n";
        float alpha = a_numerator / denom;
        float beta = b_numerator / denom;

        // std::cout << __FILE__ << __LINE__ << "\talpha: " << alpha << "\n";
        // std::cout << __FILE__ << __LINE__ << "\t beta: " << beta << "\n";
        
        Point2D PStarA = this->p1 + A * alpha;
        Point2D PStarB = other.p1 + (other.p2 -other.p1) * beta;

        if (PStarA == PStarB)
        {
            crossingPoint = PStarA;
            return true;
        }

        // std::cout << __FILE__ << __LINE__ << "\tP* alpha: " << PStarA << "\n";
        // std::cout << __FILE__ << __LINE__ << "\tP*  beta: " << PStarB << "\n";
        if (denom > 0)
        {
                 if (a_numerator < 0 || a_numerator > denom) return false;
            else if (a_numerator > 0 || a_numerator < denom) return false;
                 if (b_numerator < 0 || b_numerator > denom) return false;
            else if (b_numerator > 0 || b_numerator < denom) return false;
        }
        // std::cout << __FILE__ << __LINE__ << "\tdenom check passes \n";

        // Check if intersection is within both line segments
        if (alpha >= 0.0f && alpha <= 1.0f && beta >= 0.0f && beta <= 1.0f)
        {   
            crossingPoint = this->p1 + A * alpha;
            // std::cout << __FILE__ << __LINE__ << "\tLines cross at " << crossingPoint << "\n";
            return true;
        }

        return false;
    }
    Line &operator+=(const float &scalar)
    {
        p1 += scalar;
        p2 += scalar;
        return *this;
    }
    Line &operator+=(const Point2D &p)
    {
        p1 += p;
        p2 += p;
        return *this;
    }
    Line &operator-=(const float &scalar)
    {
        p1 -= scalar;
        p2 -= scalar;
        return *this;
    }
    Line &operator-=(const Point2D &p)
    {
        p1 -= p;
        p2 -= p;
        return *this;
    }
    void Rotate(Point2D pivot, float angle)
    {
        this->p1.Rotate(pivot, angle);
        this->p2.Rotate(pivot, angle);
    }
};

static std::ostream &operator<<(std::ostream &os, const Line &l)
{
    os << "[" << l.p1 << " -> " << l.p2 << "]";
    return os;
}
inline Rect& Rect::operator|=(const Line &other)
{
    if (this->IsInside(other.p1) && this->IsInside(other.p2))
    {
        return *this;
    }
    *this |= other.p1;
    *this |= other.p2;
    return *this;
}

inline Rect::Rect(const Line& l)
    : topLeft(std::min(l.p1.x, l.p2.x), std::min(l.p1.y, l.p2.y))
    , width(abs(l.p1.x - l.p2.x))
    , height(abs(l.p1.y - l.p2.y))
    {}

inline float Point2D::Distance(const Line& l) const
{
    // Distance from point to line segment
    // https://paulbourke.net/geometry/pointlineplane/
    Point2D v = l.p2 - l.p1;
    Point2D t = *this - l.p1;

    if (v.length() == 0) { return this->Distance(l.p1); }

    float proj_t = t.Dot(v) / v.lengthSq();

    // Clamp to line segment
    if (proj_t < 0.f) return this->Distance(l.p1);
    if (proj_t > 1.f) return this->Distance(l.p2);

    float x = l.p1.x + proj_t * v.x;
    float y = l.p1.y + proj_t * v.y;

    return this->Distance(Point2D(x, y));
}

} // namespace CMPUT350

#endif // MATHUTIL_H
