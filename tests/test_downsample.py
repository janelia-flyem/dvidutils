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
    #           `-- In the suppress_zero case, we choose the LOWER
    #               of the two tied values (in this case, 8).

    a = np.array(a)
    a_copy = a.copy()
    d = downsample_labels(a, 2, suppress_zero=False)
    assert (a == a_copy).all(), "input was overwritten!"

    assert (d == [[0, 1, 2, 3],
                  [0, 0, 2, 3]]).all()
    
    d = downsample_labels(a, 2, suppress_zero=True)
    assert (a == a_copy).all(), "input was overwritten!"
    
    assert (d == [[0, 1, 2, 3],
                  [0, 8, 2, 3]]).all()


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


def test_downsample_with_ties():
    """
    In the event of a tie between two voxels,
    we choose the lesser of the two values
    (or lesser of four values).
    """
    a = [[1,1, 3,0, 2,3],
         [2,2, 3,0, 1,0]]
    
    d = downsample_labels(a, 2)
    assert (d == [[1, 0, 0]]).all()

    d = downsample_labels(a, 2, suppress_zero=True)
    assert (d == [[1, 3, 1]]).all()


if __name__ == "__main__":
    pytest.main()
