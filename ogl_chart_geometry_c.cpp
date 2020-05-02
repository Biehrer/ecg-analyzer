#include <ogl_chart_geometry_c.h>

OGLChartGeometry_C::OGLChartGeometry_C(int screen_pos_x_S, 
                                      int screen_pos_y_S, 
                                      int width_S, 
                                      int height_S)
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
