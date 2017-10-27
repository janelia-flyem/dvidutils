#include <numeric>
#include <cmath>
#include <unordered_map>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "xtensor/xarray.hpp"
#include "xtensor/xnoalias.hpp"

#define FORCE_IMPORT_ARRAY
#include "xtensor-python/pyarray.hpp"
#include "xtensor-python/pyvectorize.hpp"

#include "utils.hpp"
#include "labelmapper.hpp"

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
    template<typename domain_t, typename codomain_t>
    auto export_label_mapper(py::module m)
    {
        typedef LabelMapper<domain_t, codomain_t> LabelMapper_t;
        std::string name = "LabelMapper_" + dtype_pair_name<domain_t, codomain_t>();

        auto cls = py::class_<LabelMapper_t>(m, name.c_str());
        cls.def(py::init<xt::pyarray<domain_t>, xt::pyarray<codomain_t>>());
        cls.def("apply", &LabelMapper_t::template apply<xt::pyarray<domain_t>>,
                "src"_a, "allow_unmapped"_a=false,
                py::call_guard<py::gil_scoped_release>());

        // Add an overload for LabelMapper(), which is actually a function that returns
        // the appropriate LabelMapper type (e.g. LabelMapper_u64u32)
        m.def("LabelMapper", make_label_mapper<domain_t, codomain_t>, "domain"_a, "codomain"_a);
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
        export_label_mapper<uint8_t, uint8_t>(m);
    }
}
