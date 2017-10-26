import sys
from itertools import product
import pytest
import numpy as np
import dvidutils

import faulthandler
faulthandler.enable()

UINT_DTYPES = [np.uint8, np.uint16, np.uint32, np.uint64]
INT_DTYPES = [np.int8, np.int16, np.int32, np.int64]
 
params = list(product(INT_DTYPES + UINT_DTYPES, [1,2,3]))
ids = [f'{ndim}d-{dtype.__name__}' for (dtype, ndim) in params]
@pytest.fixture(params=params, ids=ids)
def apply_mapping_from_dict_args(request):
    """
    pytest "fixture" to cause the test functions below to be called
    with all desired combinations of ndim and dtype.
    """
    dtype, ndim = request.param
      
    if dtype == np.int64 or dtype == np.uint64:
        pytest.skip("Skipping (u)int64 tests until xtensor-python #116 is fixed.")
      
    mapping = {k: k+100 for k in range(10)}
    original = np.random.randint(0, 10, (3,)*ndim, dtype=dtype)
    expected = original + 100
    yield (original, mapping, expected)
  
  
def test_apply_mapping_from_dict(apply_mapping_from_dict_args):
    original, mapping, expected = apply_mapping_from_dict_args
    remapped = dvidutils.apply_mapping(original, mapping)
    assert remapped.dtype == original.dtype, f"Wrong dtype: Expected {original.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"
  
  
def test_apply_incomplete_mapping(apply_mapping_from_dict_args):
    original, mapping, expected = apply_mapping_from_dict_args
      
    original.flat[0] = 255
    expected.flat[0] = 255

    remapped = dvidutils.apply_mapping(original, mapping, True)
    assert remapped.dtype == original.dtype, f"Wrong dtype: Expected {original.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"

 
dtype_pairs = list(zip(UINT_DTYPES,UINT_DTYPES)) + [(np.uint64, np.uint32)] + [(np.uint32, np.uint16)]
params2 = list(product(dtype_pairs, [1,2,3]))
ids = [f'{ndim}d-{dtype_in.__name__}-{dtype_out.__name__}' for ((dtype_in, dtype_out), ndim) in params2]
@pytest.fixture(params=params2, ids=ids)
def apply_mapping_from_arrays_args(request):
    """
    pytest "fixture" to cause the test functions below to be called
    with all desired combinations of ndim and dtype.
    """
    (dtype_in, dtype_out), ndim = request.param
     
    if (dtype_in, dtype_out) == (np.uint64, np.uint32):
        pytest.skip("Skipping uint64-uint32 combo until xtensor-python #116 is fixed.")
      
    mapping = {k: k+100 for k in range(10)}
    original = np.random.randint(0, 10, (3,)*ndim, dtype=dtype_in)
    expected = (original + 100).astype(dtype_out)
    yield (original, mapping, expected)
  
  
def test_apply_mapping_from_arrays(apply_mapping_from_arrays_args):
    original, mapping, expected = apply_mapping_from_arrays_args
    domain = np.fromiter(mapping.keys(), dtype=original.dtype)
    codomain = np.fromiter(mapping.values(), dtype=expected.dtype)
      
    remapped = dvidutils.apply_mapping(original, domain, codomain)
    assert remapped.dtype == expected.dtype, f"Wrong dtype: Expected {expected.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"


if __name__ == "__main__":
    pytest.main()
