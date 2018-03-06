#ifndef DVIDUTILS_PYDRACO_HPP
#define DVIDUTILS_PYDRACO_HPP

#include <cstdint>

#include "draco/mesh/mesh.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h"

#include "pybind11/pybind11.h"

#include "xtensor-python/pytensor.hpp"

using std::uint32_t;
using std::size_t;

namespace py = pybind11;

typedef xt::pytensor<float, 2> vertices_array_t;
typedef xt::pytensor<uint32_t, 2> faces_array_t;

int DRACO_SPEED = 5; // See encode.h

// Encode the given vertices and faces arrays from python
// into a buffer (bytes object) encoded via draco.
//
// Note: No support for vertex normals.
// Note: The vertices are expected to be passed in X,Y,Z order
py::bytes encode_faces_to_drc_bytes( vertices_array_t const & vertices,
                                     faces_array_t const & faces )
{
    //auto vertex_count = vertices.shape()[0];
    auto face_count = faces.shape()[0];
    
    draco::TriangleSoupMeshBuilder mesh_builder;
    mesh_builder.Start(face_count);

    const int32_t pos_att_id = mesh_builder.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);

    for (size_t f = 0; f < faces.shape()[0]; ++f)
    {
        auto vi0 = faces(f, 0);
        auto vi1 = faces(f, 1);
        auto vi2 = faces(f, 2);
        
        std::array<float, 3> v0{{ vertices(vi0, 0), vertices(vi0, 1), vertices(vi0, 2) }};
        std::array<float, 3> v1{{ vertices(vi1, 0), vertices(vi1, 1), vertices(vi1, 2) }};
        std::array<float, 3> v2{{ vertices(vi2, 0), vertices(vi2, 1), vertices(vi2, 2) }};
        
        mesh_builder.SetAttributeValuesForFace(
            pos_att_id, draco::FaceIndex(f), v0.data(), v1.data(), v2.data() );
    }
    std::unique_ptr<draco::Mesh> pMesh = mesh_builder.Finalize();

    draco::EncoderBuffer buf;
    draco::Encoder encoder;
    encoder.SetSpeedOptions(DRACO_SPEED, DRACO_SPEED);
    encoder.EncodeMeshToBuffer(*pMesh, &buf);
    
    return py::bytes(buf.data(), buf.size());
}

// Decode a draco-encoded buffer (given as a python bytes object)
// into a xtensor-python arrays for the vertices and faces
// (which are converted to numpy arrays on the python side).
//
// Note: normals are not decoded (currently)
// Note: The vertexes are returned in X,Y,Z order.
std::pair<vertices_array_t, faces_array_t> decode_drc_bytes_to_faces( py::bytes const & drc_bytes )
{
    using namespace draco;

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
