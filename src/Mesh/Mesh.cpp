#include "Mesh.h"

using glm::vec3, glm::vec4, glm::mat4;

void Mesh::Generate() {
    VertexArray.Generate();
    ColorBuffer.Generate();
    TransformBuffer.Generate();
    Triangles.Generate();
    EnableVertexAttributes();
}

void Mesh::Delete() const {
    VertexArray.Delete();
    TransformBuffer.Delete();
    ColorBuffer.Delete();
    Triangles.Delete();
}

void Mesh::EnableVertexAttributes() const {
    VertexArray.Bind();
    Triangles.EnableVertexAttributes();

    ColorBuffer.Bind();
    static const GLuint ColorSlot = 2;
    glEnableVertexAttribArray(ColorSlot);
    glVertexAttribPointer(ColorSlot, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glVertexAttribDivisor(ColorSlot, 1); // Attribute is updated once per instance.

    TransformBuffer.Bind();
    // Since a `mat4` is actually 4 `vec4`s, we need to enable four attributes for it.
    for (int i = 0; i < 4; i++) {
        static const GLuint TransformSlot = 3;
        glEnableVertexAttribArray(TransformSlot + i);
        glVertexAttribPointer(TransformSlot + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid *)(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(TransformSlot + i, 1); // Attribute is updated once per instance.
    }
    VertexArray.Unbind();
}

void Mesh::BindData() const {
    VertexArray.Bind();
    Triangles.BindData();

    if (Dirty) {
        TransformBuffer.SetData(Transforms);
        ColorBuffer.SetData(Colors);
    }
    Dirty = false;

    VertexArray.Unbind();
}

void Mesh::Render() const {
    if (Transforms.empty()) return;

    BindData(); // Only rebinds the data if it has changed.
    VertexArray.Bind();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    uint num_indices = Triangles.Indices.size();
    if (Transforms.size() == 1) {
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0, Transforms.size());
    }
}
