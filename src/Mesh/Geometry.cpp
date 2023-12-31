#include "Geometry.h"

#include <unordered_set>

void Geometry::Generate() {
    VertexBuffer.Generate();
    NormalBuffer.Generate();
    IndexBuffer.Generate();
}

void Geometry::EnableVertexAttributes() const {
    VertexBuffer.Bind();
    static const GLuint VertexSlot = 0;
    glEnableVertexAttribArray(VertexSlot);
    glVertexAttribPointer(VertexSlot, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    NormalBuffer.Bind();
    static const GLuint NormalSlot = 1;
    glEnableVertexAttribArray(NormalSlot);
    glVertexAttribPointer(NormalSlot, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    Dirty = true;
}

void Geometry::Delete() const {
    VertexBuffer.Delete();
    NormalBuffer.Delete();
    IndexBuffer.Delete();
}

void Geometry::BindData() const {
    if (Dirty) {
        VertexBuffer.SetData(Vertices);
        NormalBuffer.SetData(Normals);
        IndexBuffer.SetData(Indices);
    }
    Dirty = false;
}
