#pragma once

// Project includes
#include <includes/chart_types.h>

//! Defines which datatype to use for position calculations
using PositionType_TP = int;

struct BoundingBoxArea_TP {

    BoundingBoxArea_TP(const Position3D_TC<PositionType_TP>& left_top, 
                        const Position3D_TC<PositionType_TP>& left_bottom,
                        const Position3D_TC<PositionType_TP>& right_top,
                        const Position3D_TC<PositionType_TP>& right_bottom)
        : 
        _left_top(left_top),
        _left_bottom(left_bottom),
        _right_top(right_top),
        _right_bottom(right_bottom)
    {
       _width  = _left_bottom._x - _right_bottom._x;
       _height = _left_bottom._y - _left_top._y;
    }

    BoundingBoxArea_TP(PositionType_TP screen_pos_x_S,
                       PositionType_TP screen_pos_y_S,
                       PositionType_TP width_S,
                       PositionType_TP height_S)
            : 
        _width(width_S),
        _height(height_S)

    {
            // calculate corner points of the bounding box
            _left_bottom._x = screen_pos_x_S;
            _left_bottom._y = screen_pos_y_S;
            _left_bottom._z = _z_pos;

            _left_top._x = screen_pos_x_S;
            _left_top._y = screen_pos_y_S + _height;
            _left_top._z = _z_pos;

            _right_bottom._x = screen_pos_x_S + _width;
            _right_bottom._y = screen_pos_y_S;
            _right_bottom._z = _z_pos;

            _right_top._x = screen_pos_x_S + _width;
            _right_top._y = screen_pos_y_S + _height;
            _right_top._z = _z_pos;
    }

    BoundingBoxArea_TP()
    {
    }

    
public:
    //! Returns the left top of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetLeftTop() const { return _left_top; }

    //! Returns the left bottom of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetLeftBottom() const { return _left_bottom; }

    //! Returns the right top of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetRightTop() const { return _right_top; }

    //! Returns the right bottom of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetRightBottom() const { return _right_bottom; }

    //! Returns the with of the area
    const PositionType_TP GetWidth() const { return _width; }

    //! Returns the height of the area
    const PositionType_TP GetHeight() const { return _height; }

    //! Returns the z position
    const PositionType_TP GetZPosition() const { return _left_top._z; }

public:
    Position3D_TC<PositionType_TP> _left_top;

    Position3D_TC<PositionType_TP> _left_bottom;
    
    Position3D_TC<PositionType_TP> _right_top;
    
    Position3D_TC<PositionType_TP> _right_bottom;

    PositionType_TP _width = 0;

    PositionType_TP _height = 0;

    PositionType_TP _z_pos = 0;
};


class OGLChartGeometry_C {

    // Construction / Destruction / Copying / ...
public:
    OGLChartGeometry_C(PositionType_TP screen_pos_x_S,
                       PositionType_TP screen_pos_y_S,
                       PositionType_TP width_S,
                       PositionType_TP height_S);

    OGLChartGeometry_C(const OGLChartGeometry_C& geometry);

    // Public access functions
public:
    //! Returns the left top of the chart bounding box in screen coordinates
    const Position3D_TC<PositionType_TP>& GetLeftTop() const;

    //! Returns the left bottom of the chart bounding box in screen coordinates
    const Position3D_TC<PositionType_TP>& GetLeftBottom() const;

    //! Returns the right top of the chart bounding box in screen coordinates
    const Position3D_TC<PositionType_TP>& GetRightTop() const;

    //! Returns the right bottom of the chart bounding box in screen coordinates
    const Position3D_TC<PositionType_TP>& GetRightBottom() const;

    //! Returns the z position of the chart in screen coordinates
    PositionType_TP GetZPosition() const;

    //! Returns the chart width in screen coordinates
    PositionType_TP GetChartWidth() const;

    //! Returns the chart height in screen coordinates
    PositionType_TP GetChartHeight() const;

    // The drawing area of the series
    const BoundingBoxArea_TP& GetPlotArea() const;

// Private attributes
private:
    //! x-coordinate of left bottom
    PositionType_TP _screen_pos_x_S;
    
    //! y-coordinate of left bottom
    PositionType_TP _screen_pos_y_S;
    
    //! width of the chart in screen coordinates
    PositionType_TP _width_S;

    //! height of the chart in screen coordinates
    PositionType_TP _height_S;

    //! z coordinate of the chart
    PositionType_TP _z_pos = 1;

    //! left top corner position in screen coordinates
    Position3D_TC<PositionType_TP> _left_top;
    
    //! left bottom corner position in screen coordinates
    Position3D_TC<PositionType_TP> _left_bottom;

    //! right top corner position in screen coordinates
    Position3D_TC<PositionType_TP> _right_top;
    
    //! right bottom corner position in screen coordinates
    Position3D_TC<PositionType_TP> _right_bottom;

    //! The plot area
    BoundingBoxArea_TP _plot_area;
};