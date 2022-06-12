#include "../mesh.h"


void Mesh2::draw() const
{
    glBindVertexArray(vao);
    glDrawElements(primitive_mode, (GLsizei) index_cnt, index_component_type,
                   (void *) index_offset);
}