import sys
from itertools import product
import pytest
import numpy as np
from dvidutils import encode_faces_to_drc_bytes, decode_drc_bytes_to_faces

import faulthandler
faulthandler.enable()

def test_hexagon_roundtrip():
    # This map is correctly labeled with the vertex indices
    _ = -1
    hexagon = [[[_,_,_,_,_,_,_],
                [_,_,0,_,1,_,_],
                [_,_,_,_,_,_,_],
                [_,2,_,3,_,4,_],
                [_,_,_,_,_,_,_],
                [_,_,5,_,6,_,_],
                [_,_,_,_,_,_,_]]]

    hexagon = 1 + np.array(hexagon)
    vertices = np.transpose(hexagon.nonzero()).astype(np.float32)
    faces = [[3,1,4],
             [3,4,6],
             [3,6,5],
             [3,5,2],
             [3,2,0],
             [3,0,1]]
    faces = np.asarray(faces, dtype=np.uint32)
    
    drc_bytes = encode_faces_to_drc_bytes(vertices[:,::-1], faces) # ZYX -> XYZ
    rt_vertices, rt_faces = decode_drc_bytes_to_faces(drc_bytes)
    rt_vertices = rt_vertices[:,::-1] # XYZ -> ZYX

    # draco changes the order of the faces and vertices,
    # so verifying that the mesh hasn't changed is a little tricky.

    # Vertexes are easy to check -- just sort first.
    sorted_verts = np.asarray((sorted(rt_vertices.tolist())))
    assert (sorted_verts == vertices).all()

    # Faces are a little trickier.
    # Generate a triangle (3 vertices) for every face and original face,
    # And make sure the set of triangles matches.    
    def to_triangle_set(vertices, faces):
        triangles = vertices[faces]
        triangle_set = set()
        for triangle in triangles:
            triangle_set.add(tuple(map(tuple, triangle)))

    orig_triangles = to_triangle_set(vertices, faces)
    rt_triangles = to_triangle_set(rt_vertices, rt_faces)
    assert orig_triangles == rt_triangles

if __name__ == "__main__":
    pytest.main()
