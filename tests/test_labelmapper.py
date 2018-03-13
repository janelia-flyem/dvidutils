import sys
from itertools import product
import pytest
import numpy as np
from dvidutils import LabelMapper

import faulthandler
faulthandler.enable()

UINT_DTYPES = [np.uint8, np.uint16, np.uint32, np.uint64]
INT_DTYPES = [np.int8, np.int16, np.int32, np.int64]
 
dtype_pairs = list(zip(UINT_DTYPES,UINT_DTYPES))
dtype_pairs += [(np.uint64, np.uint32)] # Special case: Map uint64 down to uint32
dtype_pairs += [(np.uint32, np.uint64)] # Special case: Map uint32 up to uint64
params = list(product(dtype_pairs, [1,2,3]))
ids = [f'{ndim}d-{dtype_in.__name__}-{dtype_out.__name__}' for ((dtype_in, dtype_out), ndim) in params]

@pytest.fixture(params=params, ids=ids)
def labelmapper_args(request):
    """
    pytest "fixture" to cause the test functions below to be called
    with all desired combinations of ndim and dtype.
    """
    (dtype_in, dtype_out), ndim = request.param
    
    mapping = {k: k+100 for k in range(10)}
    original = np.random.randint(0, 10, (3,)*ndim, dtype=dtype_in)
    expected = (original + 100).astype(dtype_out)

    domain = np.fromiter(mapping.keys(), dtype=original.dtype)
    codomain = np.fromiter(mapping.values(), dtype=expected.dtype)

    yield (original, expected, mapping, domain, codomain)
   
   
def test_LabelMapper(labelmapper_args):
    original, expected, mapping, domain, codomain = labelmapper_args
       
    mapper = LabelMapper(domain, codomain)
    remapped = mapper.apply(original)

    assert remapped.dtype == expected.dtype, f"Wrong dtype: Expected {expected.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"

def test_LabelMapper_allow_unmapped(labelmapper_args):
    original, expected, mapping, domain, codomain = labelmapper_args

    original.flat[0] = 127
    expected.flat[0] = 127

    mapper = LabelMapper(domain, codomain)
    remapped = mapper.apply(original, allow_unmapped=True)

    assert remapped.dtype == expected.dtype, f"Wrong dtype: Expected {expected.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"

def test_LabelMapper_inplace(labelmapper_args):
    original, expected, mapping, domain, codomain = labelmapper_args
 
    mapper = LabelMapper(domain, codomain)
    remapped = original.copy()
    result = mapper.apply_inplace(remapped)
    assert result is None, "apply_inplace returns None"
 
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"

if __name__ == "__main__":
    pytest.main()
