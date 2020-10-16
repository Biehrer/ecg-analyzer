#pragma once

// Project includes
#include "ogl_chart_geometry_c.h"
#include "chart_types.h"

// Qt includes
#include <qvector.h>

template<typename VertexType_TP>
class ChartShapes_C  {

public:
    static 
    const 
    QVector<VertexType_TP> 
    MakeBoundingBoxVertices(const OGLChartGeometry_C& geometry) 
    {
        QVector<VertexType_TP> bb_vertices;
        // Draw bottom side
        // Bottom right corner
        bb_vertices.push_back(geometry.GetRightBottom()._x);
        bb_vertices.push_back(geometry.GetRightBottom()._y);
        bb_vertices.push_back(geometry.GetRightBottom()._z);
        // Bottom left corner
        bb_vertices.push_back(geometry.GetLeftBottom()._x);
        bb_vertices.push_back(geometry.GetLeftBottom()._y);
        bb_vertices.push_back(geometry.GetLeftBottom()._z);

        // Draw right side 
        // Bottom right corner
        bb_vertices.push_back(geometry.GetRightBottom()._x);
        bb_vertices.push_back(geometry.GetRightBottom()._y);
        bb_vertices.push_back(geometry.GetRightBottom()._z);
        // Top right corner
        bb_vertices.push_back(geometry.GetRightTop()._x);
        bb_vertices.push_back(geometry.GetRightTop()._y);
        bb_vertices.push_back(geometry.GetRightTop()._z);

        // Draw top side
        // Top left corner
        bb_vertices.push_back(geometry.GetLeftTop()._x);
        bb_vertices.push_back(geometry.GetLeftTop()._y);
        bb_vertices.push_back(geometry.GetLeftTop()._z);
        // Top right corner
        bb_vertices.push_back(geometry.GetRightTop()._x);
        bb_vertices.push_back(geometry.GetRightTop()._y);
        bb_vertices.push_back(geometry.GetRightTop()._z);

        // Draw left side 
        // Top left corner
        bb_vertices.push_back(geometry.GetLeftTop()._x);
        bb_vertices.push_back(geometry.GetLeftTop()._y);
        bb_vertices.push_back(geometry.GetLeftTop()._z);
        // Bottom left corner        
        bb_vertices.push_back(geometry.GetLeftBottom()._x);
        bb_vertices.push_back(geometry.GetLeftBottom()._y);
        bb_vertices.push_back(geometry.GetLeftBottom()._z);
        return bb_vertices;
    }


   static 
   const 
   std::pair< QVector<VertexType_TP>, QVector<VertexType_TP>>
       CreateMajTickSurfaceGridVertices(const OGLChartGeometry_C& geometry,
                                 VertexType_TP time_range_ms,
                                 VertexType_TP max_y_val,
                                 VertexType_TP min_y_val,
                                 VertexType_TP x_major_tick_dist_ms,
                                 VertexType_TP y_major_tick_dist_unit)
    {
        // x_major_tick_dist_ms and y_major_tick_dist_unit 
       // = major ticks in the same unit like the member values : min_y_axis_value and max_y_axis_value
        //p1(from)--------------------------------------------->>>>
        // |                                            |
        // ----------------------------------------------

        // Horizontal (spanning) lines (length of each line = chart_width )
        //if( (max_y_val - min_y_val) % y_major_tick_dist_unit != 0 ){
        //    // change any of the three values (the tick value?) so it works again
        //    // BUT REPORT THIS CHANGE TO THE OUTSIDE?
        //    // or just return empty, so the user knows, the values do not work
        //    y_major_tick_dist_unit = (max_y_val - min_y_val) / std::ceil(y_major_tick_dist_unit);
        //}

         VertexType_TP y_axis_major_tick_value = max_y_val;
         int number_of_horizontal_grid_lines = (max_y_val - min_y_val) / y_major_tick_dist_unit;
         QVector<VertexType_TP> horizontal_line_pos;
         horizontal_line_pos.reserve(number_of_horizontal_grid_lines * 6);

        /*int number_of_horizontal_grid_lines = (max_y_val - min_y_val) / y_major_tick_dist_unit;
        VertexType_TP y_axis_major_tick_value = max_y_val;
        */
        for ( int line_idx = 0; line_idx <= number_of_horizontal_grid_lines; ++line_idx ) {

            VertexType_TP major_tick_y_pos_S 
                = GetScreenCoordsFromYChartValue(y_axis_major_tick_value, max_y_val, min_y_val, geometry);
            
            y_axis_major_tick_value -= y_major_tick_dist_unit;
            // Point from
            horizontal_line_pos.push_back(geometry.GetLeftBottom()._x);
            horizontal_line_pos.push_back(major_tick_y_pos_S);
            horizontal_line_pos.push_back(geometry.GetZPosition() );

            // Point to
            horizontal_line_pos.push_back(geometry.GetLeftBottom()._x + geometry.GetChartWidth() );
            horizontal_line_pos.push_back(major_tick_y_pos_S);
            horizontal_line_pos.push_back(geometry.GetZPosition());
        }

        // Vertical (spanning) lines ( length of each line = chart height )
        int number_of_vertical_grid_lines = time_range_ms / x_major_tick_dist_ms;
        QVector<VertexType_TP> vertical_line_pos;
        vertical_line_pos.reserve(number_of_vertical_grid_lines);

        VertexType_TP x_axis_major_tick_value = time_range_ms;
        for ( int line_idx = 0; line_idx <= number_of_vertical_grid_lines; ++line_idx ) {
            VertexType_TP major_tick_x_pos_S = GetScreenCoordsFromXChartValue(x_axis_major_tick_value, time_range_ms, geometry);
            x_axis_major_tick_value -= x_major_tick_dist_ms;
            // Point from
            vertical_line_pos.push_back(major_tick_x_pos_S);
            vertical_line_pos.push_back(geometry.GetLeftBottom()._y);
            vertical_line_pos.push_back(geometry.GetZPosition());
            // Point to
            vertical_line_pos.push_back(major_tick_x_pos_S);
            vertical_line_pos.push_back(geometry.GetLeftBottom()._y + geometry.GetChartHeight() );
            vertical_line_pos.push_back(geometry.GetZPosition());
        }

        return { horizontal_line_pos, vertical_line_pos }; 
    }



   static
       const
       std::pair< QVector<VertexType_TP>, QVector<VertexType_TP>>
       CreateMinorTickSurfaceGridVertices(const OGLChartGeometry_C& geometry,
           VertexType_TP time_range_ms,
           VertexType_TP max_y_val,
           VertexType_TP min_y_val,
           VertexType_TP x_minor_tick_dist_ms,
           VertexType_TP y_minor_tick_dist_unit)
   {
       // x_major_tick_dist_ms and y_major_tick_dist_unit 
      // = major ticks in the same unit like the member values : min_y_axis_value and max_y_axis_value
       //p1(from)--------------------------------------------->>>>
       // |                                            |
       // ----------------------------------------------

       VertexType_TP y_axis_major_tick_value = max_y_val;
       int number_of_horizontal_grid_lines = (max_y_val - min_y_val) / y_minor_tick_dist_unit;
       QVector<VertexType_TP> horizontal_line_pos;
       horizontal_line_pos.reserve(number_of_horizontal_grid_lines * 6);

       // Lines which are parallel to the x axis 
       PositionType_TP length_line_x = 10.0; //geometry.GetRightBottom()._x - geometry.GetLeftBottom()._x;

       for ( int line_idx = 0; line_idx <= number_of_horizontal_grid_lines; ++line_idx ) {

           VertexType_TP major_tick_y_pos_S
               = GetScreenCoordsFromYChartValue(y_axis_major_tick_value, max_y_val, min_y_val, geometry);

           y_axis_major_tick_value -= y_minor_tick_dist_unit;
           // Point from
           horizontal_line_pos.push_back(geometry.GetLeftBottom()._x);
           horizontal_line_pos.push_back(major_tick_y_pos_S);
           horizontal_line_pos.push_back(geometry.GetZPosition());

           // Point to
           horizontal_line_pos.push_back(geometry.GetLeftBottom()._x + length_line_x);
           horizontal_line_pos.push_back(major_tick_y_pos_S);
           horizontal_line_pos.push_back(geometry.GetZPosition());
       }

       // Lines, which are parallel to the y axis
       // Vertical (spanning) lines ( length of each line = chart height )
       int number_of_vertical_grid_lines = time_range_ms / x_minor_tick_dist_ms;
       QVector<VertexType_TP> vertical_line_pos;
       vertical_line_pos.reserve(number_of_vertical_grid_lines);

       PositionType_TP height_line_y = geometry.GetLeftTop()._y - geometry.GetLeftBottom()._y;//10.0;

       VertexType_TP x_axis_major_tick_value = time_range_ms;
       for ( int line_idx = 0; line_idx <= number_of_vertical_grid_lines; ++line_idx ) {
           VertexType_TP major_tick_x_pos_S = GetScreenCoordsFromXChartValue(x_axis_major_tick_value, time_range_ms, geometry);
           x_axis_major_tick_value -= x_minor_tick_dist_ms;
           // Point from
           vertical_line_pos.push_back(major_tick_x_pos_S);
           vertical_line_pos.push_back(geometry.GetLeftTop()._y);
           vertical_line_pos.push_back(geometry.GetZPosition());
           // Point to
           vertical_line_pos.push_back(major_tick_x_pos_S);
           vertical_line_pos.push_back(geometry.GetLeftTop()._y - height_line_y);
           vertical_line_pos.push_back(geometry.GetZPosition());
       }

       return { horizontal_line_pos, vertical_line_pos };
   }
    // This is not in use currently.
    // Look at MakeSurfaceGridVertices() if you want to know how the axes/grid is created. 
    // The surface grid does look much better than these axes
    //! Creates vertices used to draw the x- and y-axis
    //!
    //! \param geometry the chart geometry
    //! \param size the size of the x and y axis.
    //!               In case of x axis this means the 'height'.
    //!               In case of the y axis this means the 'width'.
    //! \returns a struct containing the vertices for the x- and y-axis
    static 
    const 
    XYAxisVertices_TP 
    MakesAxesVertices(const OGLChartGeometry_C& geometry, float size = 5.0f) 
    {
        // x-axis from two triangles
        // 2d drawing of x axis
        //P1--------------------------------------------------- P4
        // |													| <- height
        //P2--------------------------------------------------- P3
        //						width
        float axes_scale = size;
        float axis_pos_z = geometry.GetZPosition();
        float x_axis_height = axes_scale;
        float x_axis_width  = geometry.GetChartWidth();
        // initialize P1 inside the half height of the plot area 
        // This is the position which splits the plot area in two identical areas
        Position3D_TC<float> p1_x(geometry.GetLeftBottom()._x, 
                                  geometry.GetLeftBottom()._y + geometry.GetChartHeight() / 2, 
                                  axis_pos_z);
        Position3D_TC<float> p2_x(p1_x._x, p1_x._y + x_axis_height, axis_pos_z);
        Position3D_TC<float> p3_x(p2_x._x + x_axis_width, p2_x._y, axis_pos_z);
        Position3D_TC<float> p4_x(p1_x._x + x_axis_width, p1_x._y, axis_pos_z);

        QVector<float> x_axis_vertices;
        x_axis_vertices.push_back(p1_x._x);
        x_axis_vertices.push_back(p1_x._y);
        x_axis_vertices.push_back(p1_x._z);
        x_axis_vertices.push_back(p2_x._x);
        x_axis_vertices.push_back(p2_x._y);
        x_axis_vertices.push_back(p2_x._z);
        x_axis_vertices.push_back(p4_x._x);
        x_axis_vertices.push_back(p4_x._y);
        x_axis_vertices.push_back(p4_x._z);

        // second triang P3 - P4 - P2
        x_axis_vertices.push_back(p3_x._x);
        x_axis_vertices.push_back(p3_x._y);
        x_axis_vertices.push_back(p3_x._z);
        x_axis_vertices.push_back(p4_x._x);
        x_axis_vertices.push_back(p4_x._y);
        x_axis_vertices.push_back(p4_x._z);
        x_axis_vertices.push_back(p2_x._x);
        x_axis_vertices.push_back(p2_x._y);
        x_axis_vertices.push_back(p2_x._z);

        // Triangles
        // 2d drawing of y axis
        // width
        // P1|--|P4
        //	 |  |
        //	 |  |
        //	 |  |
        //	 |  | } length
        //	 |  |
        //	 |  |
        //	 |  |
        // P2|--|P3

        float y_axis_width  = axes_scale;
        float y_axis_height = geometry.GetChartHeight();
        Position3D_TC<float> p1_y(geometry.GetLeftBottom()._x,
                                  geometry.GetLeftBottom()._y ,
                                  axis_pos_z);
        Position3D_TC<float> p2_y(p1_y._x, p1_y._y + y_axis_height, axis_pos_z);
        Position3D_TC<float> p3_y(p1_y._x + y_axis_width, p2_y._y, axis_pos_z);
        Position3D_TC<float> p4_y(p1_y._x + y_axis_width, p1_y._y, axis_pos_z);

        QVector<float> y_axis_vertices;
        // first triang: P1 - P2 - P4
        y_axis_vertices.push_back(p1_y._x);
        y_axis_vertices.push_back(p1_y._y);
        y_axis_vertices.push_back(p1_y._z);
        y_axis_vertices.push_back(p2_y._x);
        y_axis_vertices.push_back(p2_y._y);
        y_axis_vertices.push_back(p2_y._z);        
        y_axis_vertices.push_back(p4_y._x);
        y_axis_vertices.push_back(p4_y._y);
        y_axis_vertices.push_back(p4_y._z);
        // second triang: P3 - P4 - P2
        y_axis_vertices.push_back(p3_y._x);
        y_axis_vertices.push_back(p3_y._y);
        y_axis_vertices.push_back(p3_y._z);
        y_axis_vertices.push_back(p4_y._x);
        y_axis_vertices.push_back(p4_y._y);
        y_axis_vertices.push_back(p4_y._z);
        y_axis_vertices.push_back(p2_y._x);
        y_axis_vertices.push_back(p2_y._y);
        y_axis_vertices.push_back(p2_y._z);
        return XYAxisVertices_TP(x_axis_vertices, y_axis_vertices);
    }



    private:
       static float GetScreenCoordsFromYChartValue(float y_value, float max_y_val, float min_y_val, const OGLChartGeometry_C& geometry)
       {
            return static_cast<float>(geometry.GetLeftTop()._y) - ((y_value - min_y_val) / (max_y_val - min_y_val)) * geometry.GetChartHeight();
        }

       static float GetScreenCoordsFromXChartValue(float x_value_ms, float time_range_ms, const OGLChartGeometry_C& geometry)
        {
            // calculate x-value without wrapping
            return static_cast<float>(geometry.GetLeftBottom()._x) + (x_value_ms / time_range_ms) * geometry.GetChartWidth();
        }

};