#pragma once

// Project includes
#include <includes/CircularBuffer.h>

//! Defines which datatype to use for position calculations
using PositionType_TP = int;


struct PlotArea_TP {

    PlotArea_TP(const Position3D_TC<PositionType_TP>& left_top, 
                const Position3D_TC<PositionType_TP>& left_bottom,
                const Position3D_TC<PositionType_TP>& right_top,
                const Position3D_TC<PositionType_TP>& right_bottom)
        : 
        _left_top(left_top),
        _left_bottom(left_bottom),
        _right_top(right_top),
        _right_bottom(right_bottom)
    {
    }

    PlotArea_TP()
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
    const PositionType_TP GetWidth() const { return _left_bottom._x - _right_bottom._x; }

    //! Returns the height of the area
    const PositionType_TP GetHeight() const { return _left_top._x - _left_bottom._x; }

    //! Returns the z position
    const PositionType_TP GetZPosition() const { return _left_top._z; }

public:
    Position3D_TC<PositionType_TP> _left_top;

    Position3D_TC<PositionType_TP> _left_bottom;
    
    Position3D_TC<PositionType_TP> _right_top;
    
    Position3D_TC<PositionType_TP> _right_bottom;
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
    const PlotArea_TP& GetPlotArea() const;

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
    PlotArea_TP _plot_area;
};