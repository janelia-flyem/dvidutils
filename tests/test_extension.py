from itertools import product
import pytest
import numpy as np
import dvidutils

import faulthandler
faulthandler.enable()

INT_DTYPES = [np.uint8, np.uint16, np.uint32, np.uint64,
              np.int8, np.int16, np.int32, np.int64]


params = list(product(INT_DTYPES, [1,2,3]))
ids = [f'{ndim}d-{dtype.__name__}' for (dtype, ndim) in params]
@pytest.fixture(params=params, ids=ids)
def apply_mapping_args(request):
    """
    pytest "fixture" to cause the test functions below to be called
    with all desired combinations of ndim and dtype.
    """
    dtype, ndim = request.param
    mapping = {k: k+100 for k in range(10)}
    original = np.random.randint(0, 10, (3,)*ndim, dtype=dtype)
    expected = original + 100
    yield (original, mapping, expected)


def test_apply_mapping(apply_mapping_args):
    original, mapping, expected = apply_mapping_args
    remapped = dvidutils.apply_mapping(original, mapping)
    assert remapped.dtype == original.dtype, f"Wrong dtype: Expected {original.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"


def test_apply_incomplete_mapping(apply_mapping_args):
    original, mapping, expected = apply_mapping_args
    
    original.flat[0] = 255
    expected.flat[0] = 255
    
    remapped = dvidutils.apply_mapping(original, mapping, True)
    assert remapped.dtype == original.dtype, f"Wrong dtype: Expected {original.dtype}, got {remapped.dtype}"
    assert remapped.shape == original.shape, f"Wrong shape: Expected {original.shape}, got {remapped.shape}"
    assert (remapped == expected).all(), f"Mapping was not applied correctly!"


if __name__ == "__main__":
    pytest.main()
