import sys
from itertools import product
import pytest
import numpy as np
from dvidutils import downsample_labels

import faulthandler
faulthandler.enable()


def test_downsample_labels_2D():
    a = [[0,0, 1,1, 2,2, 3,3],
         [0,0, 1,0, 2,0, 3,0],

         [0,0, 0,0, 2,2, 3,3],
         [0,0, 8,9, 8,9, 8,9]]
    #           ^
    #           |
    #           `-- Undefined in the suppress_zero case:
    #               Could end up as either 8 or 9 (since there's one of each)

    a = np.array(a)
    a_copy = a.copy()
    d = downsample_labels(a, 2, suppress_zero=False)
    assert (a == a_copy).all(), "input was overwritten!"

    assert (d == [[0, 1, 2, 3],
                  [0, 0, 2, 3]]).all()
    
    d = downsample_labels(a, 2, suppress_zero=True)
    assert (a == a_copy).all(), "input was overwritten!"
    
    assert d[1,1] in (8,9) # See note above
    d[1,1] = 9 # Overwrite to ensure deterministic check
    
    assert (d == [[0, 1, 2, 3],
                  [0, 9, 2, 3]]).all()


def test_downsample_labels_3D():
    a = [[[0,0, 1,1],
          [2,2, 3,3]],
         
         [[0,0, 1,0],
          [2,0, 3,3]]]

    a = np.array(a)
    a_copy = a.copy()
    
    d = downsample_labels(a, 2, suppress_zero=False)
    assert (a == a_copy).all(), "input was overwritten!"
    assert (d == [[[0, 3]]]).all()
    
    d = downsample_labels(a, 2, suppress_zero=True)
    assert (a == a_copy).all(), "input was overwritten!"
    assert (d == [[[2, 3]]]).all()

def test_input_doesnt_change():
    a = np.zeros((64,64,64), dtype=np.uint32)
    d = downsample_labels(a, 2, suppress_zero=False)
    
    assert a.shape == (64,64,64), "Shape of a changed!"

if __name__ == "__main__":
    pytest.main()
