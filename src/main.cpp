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

namespace dvidutils
{
    // From dict
    template<typename src_voxel_t>
    xt::pyarray<src_voxel_t> apply_mapping( xt::pyarray<src_voxel_t> const &src,
                                            std::unordered_map<src_voxel_t, src_voxel_t> labelmap,
                                            bool allow_incomplete_mapping = false )
    {
        LabelMapper<src_voxel_t, src_voxel_t> mapper(std::move(labelmap));
        auto remapped = mapper.apply_mapping(src, allow_incomplete_mapping);
        return remapped;
    }

    // From domain/codomain pair
    template<typename domain_t, typename codomain_t>
    xt::pyarray<codomain_t> apply_mapping( xt::pyarray<domain_t> const & src_array,
                                           xt::pyarray<domain_t> const & domain,
                                           xt::pyarray<codomain_t> const & codomain,
                                           bool allow_incomplete_mapping = false )
    {
        LabelMapper<domain_t, codomain_t> mapper(domain, codomain);
        auto remapped = mapper.apply_mapping(src_array, allow_incomplete_mapping);
        return remapped;
    }

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

        // FIXME: Maybe this overload will work once xtensor-python #116
        //m.def("apply_mapping", apply_mapping<uint64_t, uint32_t>, "src"_a, "domain"_a, "codomain"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");


        m.def("apply_mapping", apply_mapping<uint64_t, uint64_t>, "src"_a, "domain"_a, "codomain"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<uint32_t, uint32_t>, "src"_a, "domain"_a, "codomain"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<uint16_t, uint16_t>, "src"_a, "domain"_a, "codomain"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<uint8_t,  uint8_t>,  "src"_a, "domain"_a, "codomain"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");

        m.def("apply_mapping", apply_mapping<uint32_t, uint16_t>, "src"_a, "domain"_a, "codomain"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");

        
        m.def("apply_mapping", apply_mapping<uint64_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<uint8_t>,  "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<uint16_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<uint32_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");

        m.def("apply_mapping", apply_mapping<int64_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<int8_t>,  "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<int16_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");
        m.def("apply_mapping", apply_mapping<int32_t>, "src"_a, "labelmap"_a, "allow_incomplete_mapping"_a=false, "Remap a label array.");

        return m.ptr();
    }
}
