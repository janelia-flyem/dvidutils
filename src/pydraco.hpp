#ifndef DVIDUTILS_PYDRACO_HPP
#define DVIDUTILS_PYDRACO_HPP

#include <cstdint>

#include "draco/mesh/mesh.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h"

#include "pybind11/pybind11.h"

#include "xtensor-python/pytensor.hpp"

using std::uint32_t;
using std::size_t;

namespace py = pybind11;

typedef xt::pytensor<float, 2> vertices_array_t;
typedef xt::pytensor<uint32_t, 2> faces_array_t;

int DRACO_SPEED = 0; // Best compression. See encode.h

// Encode the given vertices and faces arrays from python
// into a buffer (bytes object) encoded via draco.
//
// Special case: If faces is empty, an empty buffer is returned.
//
// Note: No support for vertex normals.
// Note: The vertices are expected to be passed in X,Y,Z order
py::bytes encode_faces_to_drc_bytes( vertices_array_t const & vertices,
                                     faces_array_t const & faces )
{
    using namespace draco;

    auto face_count = faces.shape()[0];
    auto vertex_count = vertices.shape()[0];

    // Special case:
    // If faces is empty, an empty buffer is returned.
    if (face_count == 0)
    {
        return py::bytes();
    }

    Mesh mesh;
    mesh.set_num_points(vertex_count);
    mesh.SetNumFaces(face_count);
    
    // Init vertex attribute
    PointAttribute vert_att_template;
    vert_att_template.Init( GeometryAttribute::POSITION,    // attribute_type
                            nullptr,                        // buffer
                            3,                              // num_components
                            DT_FLOAT32,                     // data_type
                            false,                          // normalized
                            DataTypeLength(DT_FLOAT32) * 3, // byte_stride
                            0 );                            // byte_offset

    // Add vertex to mesh (makes a copy internally)
    int vert_att_id = mesh.AddAttribute(vert_att_template, true, vertex_count);
    mesh.SetAttributeElementType(vert_att_id, MESH_VERTEX_ATTRIBUTE);

    // Get a reference to the mesh's copy of the vertex attribute
    PointAttribute & vert_att = *(mesh.attribute(vert_att_id));

    // Load the vertices into the vertex attribute
    for (size_t vi = 0; vi < vertex_count; ++vi)
    {
        std::array<float, 3> v{{ vertices(vi, 0), vertices(vi, 1), vertices(vi, 2) }};
        vert_att.SetAttributeValue(AttributeValueIndex(vi), v.data());
    }
    
    // Load the faces
    for (size_t f = 0; f < face_count; ++f)
    {
        Mesh::Face face = {{ PointIndex(faces(f, 0)),
                             PointIndex(faces(f, 1)),
                             PointIndex(faces(f, 2)) }};
        
        for (auto vi : face)
        {
            assert(vi < vertex_count && "face has an out-of-bounds vertex");
        }
        
        mesh.SetFace(draco::FaceIndex(f), face);
    }

    draco::EncoderBuffer buf;
    draco::Encoder encoder;
    encoder.SetSpeedOptions(DRACO_SPEED, DRACO_SPEED);
    encoder.EncodeMeshToBuffer(mesh, &buf);
    
    return py::bytes(buf.data(), buf.size());
}

// Decode a draco-encoded buffer (given as a python bytes object)
// into a xtensor-python arrays for the vertices and faces
// (which are converted to numpy arrays on the python side).
//
// Special case: If drc_bytes is empty, return empty vertices and faces.
//
// Note: normals are not decoded (currently)
// Note: The vertexes are returned in X,Y,Z order.
std::pair<vertices_array_t, faces_array_t> decode_drc_bytes_to_faces( py::bytes const & drc_bytes )
{
    using namespace draco;
    
    // Special case:
    // If drc_bytes is empty, return empty vertices and faces.
    if (py::len(drc_bytes) == 0)
    {
        vertices_array_t::shape_type verts_shape = {{0, 3}};
        vertices_array_t vertices(verts_shape);

        faces_array_t::shape_type faces_shape = {{0, 3}};
        faces_array_t faces(faces_shape);
        
        return std::make_pair( std::move(vertices), std::move(faces) );
    }

    // Extract pointer to raw bytes (avoid copy)
    PyObject * pyObj = drc_bytes.ptr();
    char * raw_buf = nullptr;
    Py_ssize_t bytes_length = 0;
    PyBytes_AsStringAndSize(pyObj, &raw_buf, &bytes_length);
    
    // Wrap bytes in a DecoderBuffer
    DecoderBuffer buf;
    buf.Init( raw_buf, bytes_length );
    
    // Decode to Mesh
    Decoder decoder;
    auto pMesh = decoder.DecodeMeshFromBuffer(&buf).value();
    
    // Extract vertices
    const PointAttribute *const att = pMesh->GetNamedAttribute(GeometryAttribute::POSITION);
    vertices_array_t::shape_type::value_type vertex_count = 0;
    if (att != nullptr)
    {
        vertex_count = att->size();
    }
    
    vertices_array_t::shape_type verts_shape = {{vertex_count, 3}};
    vertices_array_t vertices(verts_shape);
    
    std::array<float, 3> value;
    for (AttributeValueIndex i(0); i < att->size(); ++i)
    {
        if (!att->ConvertValue<float, 3>(i, &value[0]))
        {
            std::ostringstream ssErr;
            ssErr << "Error reading vertex " << i << std::endl;
            throw std::runtime_error(ssErr.str());
        }
        float const * vertex = reinterpret_cast<float const *>(&value[0]);
        vertices(i.value(), 0) = vertex[0];
        vertices(i.value(), 1) = vertex[1];
        vertices(i.value(), 2) = vertex[2];
    }

    // Extract faces
    faces_array_t::shape_type faces_shape = {{pMesh->num_faces(), 3}};
    faces_array_t faces(faces_shape);
    
    for (auto i = 0; i < pMesh->num_faces(); ++i)
    {
        auto const & face = pMesh->face(FaceIndex(i));
        faces(i, 0) = att->mapped_index(face[0]).value();
        faces(i, 1) = att->mapped_index(face[1]).value();
        faces(i, 2) = att->mapped_index(face[2]).value();
    }
    
    return std::make_pair( std::move(vertices), std::move(faces) );
}

#endif
