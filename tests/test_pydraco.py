import sys
from itertools import product
import pytest
import numpy as np
import pandas as pd
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

    from vol2mesh import Mesh
    Mesh(vertices, faces).serialize('/tmp/orig-hexagon.obj')
    Mesh(rt_vertices, rt_faces).serialize('/tmp/rt-hexagon.obj')

    # draco changes the order of the faces and vertices,
    # so verifying that the mesh hasn't changed is a little tricky.
    _compare(vertices, faces, rt_vertices, rt_faces, drop_duplicates=False)
    

def test_random():
    vertices = np.random.randint(0,100, size=(10, 3)).astype(np.float32)

    # Choose carefully to ensure no degenerate faces
    faces = np.zeros((50,3), dtype=np.uint32)
    for face in faces:
        face[:] = np.random.choice(list(range(10)), size=(3,), replace=False)
    drc_bytes = encode_faces_to_drc_bytes(vertices, faces)
    rt_vertices, rt_faces = decode_drc_bytes_to_faces(drc_bytes)
     
     # Output may have duplicates because the encoding method I'm using.
     # (I should fix this.  I suspect the 'TriangleSoupMeshBuilder' was a bad option to use.)
    _compare(vertices, faces, rt_vertices, rt_faces, drop_duplicates=True)


def _compare(vertices, faces, rt_vertices, rt_faces, drop_duplicates=False): 
    # Vertexes are easy to check -- just sort first.
    sorted_verts = np.asarray((sorted(vertices.tolist())))
    sorted_rt_verts = np.asarray((sorted(rt_vertices.tolist())))
    
    if drop_duplicates:
        sorted_verts = pd.DataFrame(sorted_verts)
        sorted_verts.drop_duplicates(inplace=True)
        sorted_verts = sorted_verts.values
    
        sorted_rt_verts = pd.DataFrame(sorted_rt_verts)
        sorted_rt_verts.drop_duplicates(inplace=True)
        sorted_rt_verts = sorted_rt_verts.values

    #print(sorted_verts)
    #print('--------------')
    #print(sorted_rt_verts)

    assert (sorted_verts == sorted_rt_verts).all()

    # Faces are a little trickier.
    # Generate a triangle (3 vertices) for every face and original face,
    # And make sure the set of triangles matches.    
    def to_triangle_set(v, f):
        triangles = v[f]
        triangle_set = set()
        for triangle in triangles:
            triangle_set.add(tuple(sorted(map(tuple, triangle))))
        return triangle_set

    orig_triangles = to_triangle_set(vertices, faces)
    rt_triangles = to_triangle_set(rt_vertices, rt_faces)
    
    #print(len(orig_triangles))
    #print(sorted(orig_triangles))
    #print('--------------')
    #print(len(rt_triangles))
    #print(sorted(rt_triangles))
    
    assert orig_triangles == rt_triangles


if __name__ == "__main__":
    pytest.main()
