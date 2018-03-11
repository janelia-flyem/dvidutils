#include <numeric>
#include <cmath>
#include <unordered_map>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "xtensor/xarray.hpp"

#define FORCE_IMPORT_ARRAY
#include "xtensor-python/pyarray.hpp"
#include "xtensor-python/pytensor.hpp"
#include "xtensor-python/pyvectorize.hpp"

#include "utils.hpp"
#include "labelmapper.hpp"
#include "downsample_labels.hpp"
#include "remap_duplicates.hpp"
#include "pydraco.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

namespace dvidutils
{
    // The LabelMapper constructor, but wrapped in a normal function
    template<typename domain_t, typename codomain_t>
    LabelMapper<domain_t, codomain_t> make_label_mapper( xt::pyarray<domain_t> domain,
                                                         xt::pyarray<codomain_t> codomain )
    {
        return LabelMapper<domain_t, codomain_t>(domain, codomain);
    }

    // Exports LabelMapper<D,C> as a Python class,
    // And add a Python overload of LabelMapper()
    //
    // FIXME: LabelMapper's KeyError type is translated to an ordinary Python
    //        exception, not a Python KeyError in particular.
    //        We could fix this with a translation function that throws a py::key_error
    template<typename domain_t, typename codomain_t>
    auto export_label_mapper(py::module m)
    {
        typedef LabelMapper<domain_t, codomain_t> LabelMapper_t;
        std::string name = "LabelMapper_" + dtype_pair_name<domain_t, codomain_t>();

        auto cls = py::class_<LabelMapper_t>(m, name.c_str());
        cls.def(py::init<xt::pyarray<domain_t>, xt::pyarray<codomain_t>>());

        cls.def("apply",
                &LabelMapper_t::template apply<xt::pyarray<domain_t>>,
                "src"_a, "allow_unmapped"_a=false,
                py::call_guard<py::gil_scoped_release>());

        cls.def("apply_inplace",
                &LabelMapper_t::template apply_inplace<xt::pyarray<domain_t>>,
                "src"_a, "allow_unmapped"_a=false,
                py::call_guard<py::gil_scoped_release>());

        // Add an overload for LabelMapper(), which is actually a function that returns
        // the appropriate LabelMapper type (e.g. LabelMapper_u64u32)
        m.def("LabelMapper", make_label_mapper<domain_t, codomain_t>, "domain"_a, "codomain"_a);
    }

    template <typename T>
    xt::pyarray<T> py_downsample_labels(xt::pyarray<T> const & labels, int factor, bool suppress_zero )
    {
        // FIXME: There's GOT to be a more elegant way to auto-select the right call based on dimansionality
        if (labels.shape().size() == 3)
        {
            return downsample_labels<xt::pyarray<T>, 3>(labels, factor, suppress_zero);
        }
        if (labels.shape().size() == 2)
        {
            return downsample_labels<xt::pyarray<T>, 2>(labels, factor, suppress_zero);
        }
        std::ostringstream ss;
        ss << "Unsupported number of dimensions: " << labels.shape().size();
        throw std::runtime_error(ss.str());
    }

    PYBIND11_MODULE(dvidutils, m) // note: PYBIND11_MODULE requires pybind11 >= 2.2.0
    {
        xt::import_numpy();

        m.doc() = R"docu(
            A collection of utility functions for dealing with dvid data

            .. currentmodule:: dvidutils

            .. autosummary::
               :toctree: _generate
        
               LabelMapper

        )docu";

        // FIXME: uint64_t cases won't work properly until xtensor-python #116 (second part) is fixed.
        export_label_mapper<uint64_t, uint64_t>(m);
        export_label_mapper<uint64_t, uint32_t>(m);

        export_label_mapper<uint32_t, uint32_t>(m);
        export_label_mapper<uint16_t, uint16_t>(m);
        export_label_mapper<uint8_t,  uint8_t>(m);

        m.def("downsample_labels", &py_downsample_labels<uint64_t>, "labels"_a, "factor"_a, "suppress_zero"_a=false, py::call_guard<py::gil_scoped_release>());
        m.def("downsample_labels", &py_downsample_labels<uint32_t>, "labels"_a, "factor"_a, "suppress_zero"_a=false, py::call_guard<py::gil_scoped_release>());
        m.def("downsample_labels", &py_downsample_labels<uint16_t>, "labels"_a, "factor"_a, "suppress_zero"_a=false, py::call_guard<py::gil_scoped_release>());
        m.def("downsample_labels", &py_downsample_labels<uint8_t>,  "labels"_a, "factor"_a, "suppress_zero"_a=false, py::call_guard<py::gil_scoped_release>());

        m.def("remap_duplicates", &remap_duplicates<xt::pytensor<float, 2>, xt::pytensor<uint32_t, 2>>, "vertices"_a, py::call_guard<py::gil_scoped_release>());
        
        m.def("encode_faces_to_drc_bytes", encode_faces_to_drc_bytes, "vertices"_a, "normals"_a, "faces"_a);
    
        m.def("decode_drc_bytes_to_faces", decode_drc_bytes_to_faces, "drc_bytes"_a);

    }
}
