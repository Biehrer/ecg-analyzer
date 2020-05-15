#pragma once

// Project includes
#include "chart_types.h"

//! Defines which datatype to use for position calculations
using PositionType_TP = int;

class OGLChartGeometry_C {

    // Construction / Destruction / Copying / ...
public:
    OGLChartGeometry_C(PositionType_TP screen_pos_x_S,
                       PositionType_TP screen_pos_y_S,
                       PositionType_TP width_S,
                       PositionType_TP height_S);

    OGLChartGeometry_C() = default;

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

    void SetLeftTop(const Position3D_TC<PositionType_TP>& left_top);

    void SetLeftBottom(const Position3D_TC<PositionType_TP>& left_top);

    void SetRightTop(const Position3D_TC<PositionType_TP>& left_top);

    void SetRightBottom(const Position3D_TC<PositionType_TP>& left_top);

    //! Returns the z position of the chart in screen coordinates
    PositionType_TP GetZPosition() const;

    //! Returns the chart width in screen coordinates
    PositionType_TP GetChartWidth() const;

    //! Returns the chart height in screen coordinates
    PositionType_TP GetChartHeight() const;

private:
    //! Uses right_bottom - left_bottom to calculate the width
    void RecalculateWidth();
    
    //! Uses left_top - left_bottom
    void RecalculateHeight();

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

};