#include "DrawContext.h"
#include <cstdint>
namespace CMPUT350
{

void DrawContext::SetContextCenter(Point2D p)
{
    // Set the center of the view to follow the given point (e.g., player position)
    mCenter = p;
    mView.setCenter(sf::Vector2f(p.x, p.y));
}

void DrawContext::SetViewport(float left, float top, float width, float height)
{
    mViewport = sf::FloatRect({left, top}, {width, height});
    mView.setViewport(mViewport);
}

void DrawContext::SetViewSize(float width, float height)
{
    mViewSize = Point2D(width, height);
    mView.setSize(sf::Vector2f(width, height));
}

void DrawContext::ApplyView()
{
    mWindow->setView(mView);
}

Point2D DrawContext::GetWindowSize() const
{
    sf::Vector2u size = mWindow->getSize();
    return Point2D(static_cast<float>(size.x), static_cast<float>(size.y));
}

void DrawContext::DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c)
{
    sf::Text text_obj(*mFont, text, pixelSize);
    text_obj.setFillColor(sf::Color(c.r, c.g, c.b));
    text_obj.setPosition(sf::Vector2f(p.x, p.y));
    mWindow->draw(text_obj);
}

void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c)
{    
    sf::CircleShape circle(radius);
    circle.setPosition(sf::Vector2f(p.x - radius, p.y - radius));
    circle.setFillColor(sf::Color(c.r, c.g, c.b));
    mWindow->draw(circle);
}

void DrawContext::DrawRect(Rect r, RGBColor c)
{
    sf::RectangleShape rect(sf::Vector2f(r.width, r.height));
    rect.setFillColor(sf::Color(c.r, c.g, c.b));
    rect.setPosition(sf::Vector2f(r.topLeft.x, r.topLeft.y));
    mWindow->draw(rect);
}

void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c)
{    
    // get length of line
    Point2D l_vec = to - from;
    float length = l_vec.length();

    // create rectangle representation of line
    sf::RectangleShape line(sf::Vector2f(length, width));
    line.setFillColor(sf::Color(c.r, c.g, c.b));
    line.setPosition(sf::Vector2f(from.x, from.y));

    // rotate the line
    float angle_rad = atan2(l_vec.y, l_vec.x);
    line.setOrigin(sf::Vector2f(0.f, width / 2.f)); // rotate about the center of the edge of the line
    line.setRotation(sf::radians(angle_rad));
    
    mWindow->draw(line);
}

}
