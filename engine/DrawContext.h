#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Angle.hpp>
#include <cstdint>
#include "MathUtil.h"
namespace CMPUT350
{

struct RGBColor
{
    uint8_t r, g, b;
    RGBColor(uint8_t r, uint8_t g, uint8_t b)
    :r(r), g(g), b(b){}
};

namespace Colors
{
    const RGBColor red(255, 0, 0);
    const RGBColor green(0, 255, 0);
    const RGBColor blue(0, 0, 255);
    const RGBColor yellow(255, 255, 0);
    const RGBColor cyan(0, 255, 255);
    const RGBColor magenta(255, 0, 255);
    const RGBColor white(255, 255, 255);
    const RGBColor black(0, 0, 0);
    const RGBColor gray(100, 100, 100);
}

class DrawContext
{
public:

   DrawContext(std::shared_ptr<sf::RenderWindow> mWindow, std::shared_ptr<sf::Font> font)
   : mWindow(mWindow), mFont(font), mCenter(0, 0), mViewSize(1, 1)
   {
       // Default viewport is full window
       mViewport = sf::FloatRect({0.0f, 0.0f}, {1.0f, 1.0f});
       mView = mWindow->getView();
   }
    ~DrawContext() {
        // Explicitly release -- removes 200 bytes from "definitely lost" in vg
        mWindow.reset();
        mFont.reset();
    }
   void DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c);
   void DrawCircle(Point2D p, float radius, RGBColor c);
   void DrawRect(Rect r, RGBColor c);
   void DrawLine(Point2D from, Point2D to, float width, RGBColor c);
   void SetContextCenter(Point2D p);

   // Viewport configuration (normalized coordinates: 0-1)
   void SetViewport(float left, float top, float width, float height);

   // View size configuration (world units to show)
   void SetViewSize(float width, float height);

   // Apply this context's view to the window (call once per frame before rendering)
   void ApplyView();

   // Get window size (for resize detection)
   Point2D GetWindowSize() const;

private:

    std::shared_ptr<sf::RenderWindow> mWindow;
    std::shared_ptr<sf::Font> mFont;
    Point2D mCenter;
    Point2D mViewSize;  // Size of the view in world coordinates
    sf::View mView;
    sf::FloatRect mViewport;  // Portion of window this context uses
};

}

#endif // RENDERTARGET_H
