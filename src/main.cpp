#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "xtensor/xmath.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xnoalias.hpp"

#define FORCE_IMPORT_ARRAY
#include "xtensor-python/pyarray.hpp"
#include "xtensor-python/pyvectorize.hpp"

#include <iostream>
#include <numeric>
#include <cmath>
#include <unordered_map>

namespace py = pybind11;

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

typedef float float32_t;
typedef double float64_t;


template <typename T> std::string type_name() { return "Unknown type"; }
//template <typename T> std::string type_name() { return "Unknown type"; }

template <> std::string type_name<uint8_t>()  { return "uint8_t"; }
template <> std::string type_name<uint16_t>() { return "uint16_t"; }
template <> std::string type_name<uint32_t>() { return "uint32_t"; }
template <> std::string type_name<uint64_t>() { return "uint64_t"; }

template<typename src_voxel_t>
xt::pyarray<src_voxel_t> times_two( xt::pyarray<src_voxel_t> & src )
{
	std::cout << "times_two(): " << type_name<src_voxel_t>() << std::endl;

	xt::pyarray<src_voxel_t> res;
	res = static_cast<src_voxel_t>(2) * src;
	return res;
}

inline xt::pyarray<uint8_t> example2(xt::pyarray<uint8_t> &m)
{
    return m + 2;
}

template<typename src_voxel_t>
xt::pyarray<src_voxel_t> apply_mapping( xt::pyarray<src_voxel_t> const &src,
				   	   	   	   	    	   std::unordered_map<src_voxel_t, src_voxel_t> & labelmap,
									   bool allow_incomplete_mapping = true )
{
	std::cout << "apply_mapping(): " << type_name<src_voxel_t>() << std::endl;

	typename xt::pyarray<src_voxel_t>::shape_type shape(src.shape().begin(), src.shape().end());
	xt::pyarray<src_voxel_t> res(shape);

	{
		py::gil_scoped_release nogil_context;

		auto lookup_voxel = [&](src_voxel_t px) -> src_voxel_t {
			auto iter = labelmap.find(px);
			if (iter != labelmap.end())
			{
				return iter->second;
			}

			if (allow_incomplete_mapping)
			{
				// Key is missing. Return the original value.
				return static_cast<src_voxel_t>(px);
			}

			throw py::key_error("Label not found in mapping: " + std::to_string(+px));
			return 0; // unreachable line
		};

		xt::noalias(res) = xt::vectorize(lookup_voxel)(src);
	}

	return res;
}

// Python Module and Docstrings

PYBIND11_PLUGIN(dvidutils)
{
	using namespace pybind11::literals;
    xt::import_numpy();

    py::module m("dvidutils", R"docu(
        A collection of utility functions for dealing with dvid data

        .. currentmodule:: dvidutils

        .. autosummary::
           :toctree: _generate
	
		   apply_mapping

    )docu");


    m.def("example2", example2, "m"_a);
    m.def("times_two", times_two<uint8_t>, "src"_a);

    m.def("apply_mapping", apply_mapping<uint8_t>,  "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=true, "Remap a label array.");
    m.def("apply_mapping", apply_mapping<uint16_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=true, "Remap a label array.");
    m.def("apply_mapping", apply_mapping<uint32_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=true, "Remap a label array.");
    m.def("apply_mapping", apply_mapping<uint64_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=true, "Remap a label array.");

    return m.ptr();
}
