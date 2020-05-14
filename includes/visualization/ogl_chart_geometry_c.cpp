#include <includes/visualization/ogl_chart_geometry_c.h>

OGLChartGeometry_C::OGLChartGeometry_C(PositionType_TP screen_pos_x_S, 
                                       PositionType_TP screen_pos_y_S,
                                       PositionType_TP width_S,
                                       PositionType_TP height_S)
    : 
    _screen_pos_x_S(screen_pos_x_S),
    _screen_pos_y_S(screen_pos_y_S),
    _width_S(width_S),
    _height_S(height_S)
{
    // calculate corner points of the bounding box
    _left_bottom._x = _screen_pos_x_S;
    _left_bottom._y = _screen_pos_y_S;
    _left_bottom._z = _z_pos;
    
    _left_top._x = _screen_pos_x_S;
    _left_top._y = _screen_pos_y_S + _height_S;
    _left_top._z = _z_pos;

    _right_bottom._x = _screen_pos_x_S + _width_S;
    _right_bottom._y = _screen_pos_y_S;
    _right_bottom._z = _z_pos;

    _right_top._x = _screen_pos_x_S + _width_S;
    _right_top._y = _screen_pos_y_S + _height_S;
    _right_top._z = _z_pos;
}

OGLChartGeometry_C::OGLChartGeometry_C(const OGLChartGeometry_C & geometry)
{
    _left_bottom  = geometry._left_bottom;
    _left_top     = geometry._left_top;
    _right_bottom = geometry._right_bottom;
    _right_top    = geometry._right_top;

    _width_S  = _right_bottom._x - _left_bottom._x;
    _height_S = _left_top._y     - _left_bottom._y;
    
    _z_pos = geometry._z_pos;
    
    _screen_pos_x_S = _left_bottom._x;
    _screen_pos_y_S = _left_bottom._y;
}

const 
Position3D_TC<PositionType_TP>&
OGLChartGeometry_C::GetLeftTop() const
{
    return _left_top;
}

const 
Position3D_TC<PositionType_TP>&
OGLChartGeometry_C::GetLeftBottom() const
{
    return _left_bottom;
}

const 
Position3D_TC<PositionType_TP>&
OGLChartGeometry_C::GetRightTop() const
{
    return _right_top;
}

const 
Position3D_TC<PositionType_TP>&
OGLChartGeometry_C::GetRightBottom() const
{
    return _right_bottom;
}


void OGLChartGeometry_C::SetLeftTop(const Position3D_TC<PositionType_TP>& left_top)
{
    _left_top._x = left_top._x; 
    _left_top._y = left_top._y;
    _left_top._z = left_top._z;

}

void OGLChartGeometry_C::SetLeftBottom(const Position3D_TC<PositionType_TP>& left_bottom)
{
    _left_bottom._x = left_bottom._x;
    _left_bottom._y = left_bottom._y;
    _left_bottom._z = left_bottom._z;
    RecalculateHeight();
    RecalculateWidth();
}

void OGLChartGeometry_C::SetRightTop(const Position3D_TC<PositionType_TP>& right_top)
{
    _right_top._x = right_top._x;
    _right_top._y = right_top._y;
    _right_top._z = right_top._z;
}

void OGLChartGeometry_C::SetRightBottom(const Position3D_TC<PositionType_TP>& right_bottom)
{
    _right_bottom._x = right_bottom._x;
    _right_bottom._y = right_bottom._y;
    _right_bottom._z = right_bottom._z;
    RecalculateHeight();
    RecalculateWidth();
}

int OGLChartGeometry_C::GetZPosition() const
{
    return _z_pos;
}


int OGLChartGeometry_C::GetChartWidth() const
{
    return _width_S;
}


int OGLChartGeometry_C::GetChartHeight() const
{
    return _height_S;
}

void OGLChartGeometry_C::RecalculateWidth()
{
    _width_S = _right_bottom._x - _left_bottom._x;
}

void OGLChartGeometry_C::RecalculateHeight()
{
    _height_S = _left_top._y - _left_bottom._y;
}
