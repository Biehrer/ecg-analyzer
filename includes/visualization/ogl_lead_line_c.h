#pragma once
// Project includes
#include "ogl_chart_geometry_c.h"

// Qt includes
#include <qopenglshaderprogram.h>
#include <qopenglbuffer.h>
#include <qopenglfunctions.h>

class OGLLeadLine_C {

public:
    //! \param the plot area of the plot to which this lead line belongs
    OGLLeadLine_C(const OGLChartGeometry_C& plot_area);
    ~OGLLeadLine_C();

public:
    //! Draws the lead line
    void DrawLeadLine(QOpenGLShaderProgram& shader, float position_x_ms);
    
    //! (Re-)Sets the plot area
    void SetPlotArea(const OGLChartGeometry_C& plot_area);

    //! Set the color in which the line is drawn
    void SetColor(const QVector3D& color_rgb);

private:
    //! Creates the vbo used to draw the lead line indicating the most current datapoint
    void CreateLeadLineVbo();

    //! Update the current position of the lead line. 
    //! Used to assign the last visualized point as lead-line position
    void UpdateLeadLinePosition(float x_value_new);

private:
    //! Vertex buffer object for the lead line 
    QOpenGLBuffer _lead_line_vbo;

    //! Vertices for the lead line
    QVector<float> _lead_line_vertices;

    //! Number of bytes for the lead line used by the vbo. 
    //! This should be equal to six, when the lead line is a line and no point or other shape.
    int _number_of_bytes_lead_line = 0;

    //! The y component of the last value plotted
    float _last_plotted_y_value_S = 0;

    //! The x component of the last value plotted
    float _last_plotted_x_value_S = 0;

    QVector3D _lead_line_color;
};