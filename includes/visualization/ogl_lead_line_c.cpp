#include "ogl_lead_line_c.h"


OGLLeadLine_C::OGLLeadLine_C(const OGLChartGeometry_C& plot_area)
{
    // a line consists of two points, each with three coordinates (x,y,z)
    int buffer_size = 2 * 3; 
    _lead_line_vertices.resize(buffer_size);
    SetPlotArea(plot_area);
    CreateLeadLineVbo();
}

OGLLeadLine_C::~OGLLeadLine_C()
{
    _lead_line_vbo.destroy();
}


void
OGLLeadLine_C::SetPlotArea(const OGLChartGeometry_C& plot_area) 
{
    // setup buffer for the vertices, used to draw the lead line: 
    // The most vertices are fixed and only the x value of the vertices needs to be adapted when new data was added.
    // The only value that changes when adding new data, is the x value of the lead line. 
    // The y values are fixed because the lead line is a vertical line, which is always drawn through the whole chart.
    // The z values are fixed anyway

    // point from:
    // y value
    _lead_line_vertices[1] = plot_area.GetLeftBottom()._y;
    // z value
    _lead_line_vertices[2] = plot_area.GetZPosition();
    // point to:
    // y value
    _lead_line_vertices[4] = plot_area.GetLeftTop()._y;
    // z value
    _lead_line_vertices[5] = plot_area.GetZPosition();
}

void OGLLeadLine_C::SetColor(const QVector3D& color_rgb/*, QOpenGLShaderProgram& shader*/)
{
    _lead_line_color = color_rgb;
    //QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    //shader.bind();
    //shader.setUniformValue("u_object_color", _lead_line_color);
}

void
OGLLeadLine_C::CreateLeadLineVbo()
{
    int buffer_size = 2 * 3;
    _number_of_bytes_lead_line = buffer_size * sizeof(float);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Setup OGL-Chart buffer - empty
    _lead_line_vbo.create();
    _lead_line_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 coordinates make one point  (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // the lead line consists of one line -> 2 points, each 3 vertices of the type float
    _lead_line_vbo.allocate(nullptr, buffer_size * sizeof(float));
    _lead_line_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    f->glDisableVertexAttribArray(0);
    _lead_line_vbo.release();

}

void
OGLLeadLine_C::UpdateLeadLinePosition(const float x_value_new)
{
    _lead_line_vertices[0] = x_value_new;
    _lead_line_vertices[3] = x_value_new;
    // Overwrite the whole vbo with the new data
    // alternative: write just two values, but because of caching this should not really result in differences..
    _lead_line_vbo.write(0, _lead_line_vertices.constData(), _number_of_bytes_lead_line);
}



void
OGLLeadLine_C::DrawLeadLine(QOpenGLShaderProgram& shader, const float position_x_ms)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    // It's not necessary to set this each draw. 
    shader.setUniformValue("u_object_color", _lead_line_color); 
    _lead_line_vbo.bind();
    // Update position vertices
    UpdateLeadLinePosition(position_x_ms);
    // Draw
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, 2);
    f->glDisableVertexAttribArray(0);
    _lead_line_vbo.release();
}