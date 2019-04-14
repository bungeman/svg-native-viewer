/*
Copyright 2019 Adobe. All rights reserved.
This file is licensed to you under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License. You may obtain a copy
of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
OF ANY KIND, either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

#ifndef SVGViewer_SVGRenderer_h
#define SVGViewer_SVGRenderer_h

#include <array>
#include <boost/variant.hpp>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace SVGNative
{
/**
 * Line caps as described in:
 * https://www.w3.org/TR/SVG2/painting.html#LineCaps
 */
enum class LineCap
{
    kButt,
    kRound,
    kSquare
};

/**
 * Line joins as described in:
 * https://www.w3.org/TR/SVG2/painting.html#LineJoin
 */
enum class LineJoin
{
    kMiter,
    kRound,
    kBevel
};

/**
 * Winding rules as described in:
 * https://www.w3.org/TR/SVG2/painting.html#WindingRule
 */
enum class WindingRule
{
    kNonZero,
    kEvenOdd
};

/**
 * Gradient type. SVG Native supports the 2 gradient types
 * * linear gradient and
 * * radial gradient.
 */
enum class GradientType
{
    kLinearGradient,
    kRadialGradient
};

/**
 * Gradient spread method.
 * * pad
 * * reflect
 * * repeat
 *
 * @note See https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementSpreadMethodAttribute
 */
enum class SpreadMethod
{
    kPad,
    kReflect,
    kRepeat
};

struct Gradient;
class Transform;
class Path;
class Shape;

using Color = std::array<float, 4>;
using Paint = boost::variant<Color, Gradient>;
using ColorStop = std::pair<float, Color>;
using ColorMap = std::map<std::string, Color>;

struct Rect
{
    float x = std::numeric_limits<float>::quiet_NaN();
    float y = std::numeric_limits<float>::quiet_NaN();
    float width = std::numeric_limits<float>::quiet_NaN();
    float height = std::numeric_limits<float>::quiet_NaN();
};

/**
 * Representation of a linear gradient paint server.
 */
struct Gradient
{
    GradientType type = GradientType::kLinearGradient;
    SpreadMethod method = SpreadMethod::kPad;
    std::vector<ColorStop> colorStops; /** Color stops with offset-color pairs **/
    float x1 = std::numeric_limits<float>::quiet_NaN(); /** x1 for linearGradient **/
    float y1 = std::numeric_limits<float>::quiet_NaN(); /** y1 for linearGradient **/
    float x2 = std::numeric_limits<float>::quiet_NaN(); /** x2 for linearGradient **/
    float y2 = std::numeric_limits<float>::quiet_NaN(); /** y2 for linearGradient **/
    float cx = std::numeric_limits<float>::quiet_NaN(); /** cx for radialGradient **/
    float cy = std::numeric_limits<float>::quiet_NaN(); /** cy for radialGradient **/
    float fx = std::numeric_limits<float>::quiet_NaN(); /** fx for radialGradient **/
    float fy = std::numeric_limits<float>::quiet_NaN(); /** fy for radialGradient **/
    float r = std::numeric_limits<float>::quiet_NaN(); /** r for radialGradient **/
    std::shared_ptr<Transform> transform; /** Joined transformation matrix based to the "transform" attribute. **/
};

/**
 * Stroke style information.
 */
struct StrokeStyle
{
    bool hasStroke = false;
    float strokeOpacity = 1.0;
    float lineWidth = 1.0;
    LineCap lineCap = LineCap::kButt;
    LineJoin lineJoin = LineJoin::kMiter;
    float miterLimit = 4.0;
    std::vector<float> dashArray;
    float dashOffset = 0.0;
    Paint paint = Color{{0, 0, 0, 1.0}};
};

/**
 * Fill style information.
 */
struct FillStyle
{
    bool hasFill = true;
    WindingRule fillRule = WindingRule::kNonZero;
    float fillOpacity = 1.0;
    Paint paint = Color{{0, 0, 0, 1.0}};
};

/**
 * Representation of a 2D affine transform with 6 values.
 */
class Transform
{
public:
    virtual ~Transform() = default;

    virtual void Set(float a, float b, float c, float d, float tx, float ty) = 0;
    virtual void Rotate(float r) = 0;
    virtual void Translate(float tx, float ty) = 0;
    virtual void Scale(float sx, float sy) = 0;
    virtual void Concat(const Transform& other) = 0;
};

/**
 * All compositing related properties. With the exception of the
 */
struct GraphicStyle
{
    // Add blend modes and other graohic style options here.
    float opacity = 1.0; /** Corresponds to the "opacty" CSS property. **/
    std::shared_ptr<Transform> transform; /** Joined transformation matrix based to the "transform" attribute. **/
    std::shared_ptr<Shape> clippingPath;
};

/**
 * A presentation of a path.
 */
class Path
{
public:
    virtual ~Path() = default;

    virtual void Rect(float x, float y, float width, float height) = 0;
    virtual void RoundedRect(float x, float y, float width, float height, float cornerRadius) = 0;
    virtual void Ellipse(float cx, float cy, float rx, float ry) = 0;

    virtual void MoveTo(float x, float y) = 0;
    virtual void LineTo(float x, float y) = 0;
    virtual void CurveTo(float x1, float y1, float x2, float y2, float x3, float y3) = 0;
    virtual void CurveToV(float x2, float y2, float x3, float y3) = 0;
    virtual void ClosePath() = 0;
};

/**
 * A shape is the combination of one or more Path objects with winding rules for each path
 * object.
 * Transforms and unions only apply to Shapes.
 */
class Shape
{
public:
    virtual ~Shape() = default;

    virtual void Transform(const Transform& transform) = 0;
    virtual void Union(const Shape& shape) = 0;
};

/**
 * An image object generated from a base64 string.
 * The port needs to decode the Base64 string and provide
 * information about the dimensions of the image.
 **/
class ImageData
{
public:
    virtual ~ImageData() = default;

    virtual float Width() const = 0;
    virtual float Height() const = 0;
};

/**
 * Base class for deriving, platform dependent renderer classes with immediate
 * graphic library calls.
 */
class SVGRenderer
{
public:
    virtual ~SVGRenderer() = default;

    virtual std::unique_ptr<ImageData> CreateImageData(const std::string& base64) = 0;
    virtual std::unique_ptr<Path> CreatePath() = 0;
    virtual std::unique_ptr<Shape> CreateShape(const Path& path, WindingRule windingRule = WindingRule::kNonZero) = 0;
    virtual std::unique_ptr<Transform> CreateTransform(
        float a = 1.0, float b = 0.0, float c = 0.0, float d = 1.0, float tx = 0.0, float ty = 0.0) = 0;

    virtual void Save(const GraphicStyle& graphicStyle) = 0;
    virtual void Restore() = 0;

    virtual void DrawPath(
        const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle) = 0;
    virtual void DrawImage(const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea) = 0;
};

} // namespace SVGNative

#endif // SVGViewer_SVGRenderer_h