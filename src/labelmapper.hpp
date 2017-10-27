#ifndef DVIDUTILS_LABELMAPPER_HPP
#define DVIDUTILS_LABELMAPPER_HPP

#include <utility>
#include <unordered_map>

#include "xtensor/xarray.hpp"
#include "xtensor/xtensor.hpp"
#include "xtensor/xmath.hpp"

namespace dvidutils
{

    // Stores a mapping from an original set of labels (the domain)
    // to a new set of labels (the codomain), and exposes a function "apply()"
    // to convert arrays of domain label voxels into arrays of codomain label voxels.
    template<typename domain_t, typename codomain_t>
    class LabelMapper
    {
    public:
        typedef std::unordered_map<domain_t, codomain_t> mapping_t;

        typedef xt::xtensor<domain_t, 1> domain_list_t;
        typedef xt::xtensor<codomain_t, 1> codomain_list_t;

        typedef xt::xarray<domain_t> domain_array_t;
        typedef xt::xarray<codomain_t> codomain_array_t;

        class KeyError : public std::runtime_error
        {
            using std::runtime_error::runtime_error;
        };
        
        // Construct directly from a pre-existing mapping
        LabelMapper(mapping_t mapping)
        : _mapping(std::move(mapping))
        {
        }

        // Construct from domain and codomain lists
        template <typename domain_list_t, typename codomain_list_t>
        LabelMapper(domain_list_t const & domain, codomain_list_t const & codomain)
        {
            if (domain.shape().size() != 1 || codomain.shape().size() != 1)
            {
                throw std::runtime_error("Can't initialize LabelMapper: domain and codomain should be 1D arrays");
            }
            if (domain.size() != codomain.size())
            {
                throw std::runtime_error("Can't initialize LabelMapper: "
                                         "domain and codomain arrays don't have matching sizes.");
            }

            // Load up the mapping
            for (size_t i = 0; i < domain.size(); ++i)
            {
                _mapping[domain(i)] = codomain(i);
            }
        }
        
        template <typename domain_array_t>
        codomain_array_t apply( domain_array_t const & src, bool allow_unmapped = false )
        {
            auto res = codomain_array_t::from_shape(src.shape());
            {
                auto lookup_voxel = [&](domain_t px) -> codomain_t {
                    auto iter = _mapping.find(px);
                    if (iter != _mapping.end())
                    {
                        return iter->second;
                    }
                    
                    if (allow_unmapped)
                    {
                        // Key is missing. Return the original value.
                        return static_cast<typename codomain_array_t::value_type>(px);
                    }
                    
                    throw KeyError("Label not found in mapping: " + std::to_string(+px));
                    return 0; // unreachable line
                };
                
                xt::noalias(res) = xt::vectorize(lookup_voxel)(src);
            }
            
            return res;
        }
        
    private:
        mapping_t _mapping;
    };
}

#endif
