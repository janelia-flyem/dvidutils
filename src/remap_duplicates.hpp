#ifndef DVIDUTILS_MESHSTITCHER_HPP
#define DVIDUTILS_MESHSTITCHER_HPP

#include <cstdint>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "xtensor/xtensor.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xmath.hpp"
#include "xtensor/xoperation.hpp"
#include "xtensor/xadapt.hpp"

using std::size_t;
using std::uint32_t;

namespace dvidutils
{
    // Given an array of vertices (N,3), find those vertices which are
    // duplicates and return an index mapping that points only to the
    // first occurrence of each duplicate vertex found.
    // Non-duplicate vertexes are not included in the result,
    // i.e. anything missing from the results is implicitly identity-mapped.
    template <typename vertices_array_t, typename index_map_array_t>
    index_map_array_t remap_duplicates(vertices_array_t const & vertices)
    {
        // We would like to build a map of vertex -> first index where it appears,
        // but that would require copying nearly all vertices, which is RAM-expensive.
        // Instead, we use a map of index -> first equivalent index,
        // via a custom hash function, saving on the RAM overhead of storing
        // vertices as keys (we just store the indexes instead).
        auto hasher = [&](uint32_t index) {
            size_t hash = 0;
            boost::hash_combine(hash, vertices(index, 0));
            boost::hash_combine(hash, vertices(index, 1));
            boost::hash_combine(hash, vertices(index, 2));
            return hash;
            
        };

        // Vertex index comparison.
        // Two indexes are considered equal if they point to equivalent vertices.
        auto comparer = [&](uint32_t const & left, uint32_t const & right) -> bool {
            return xt::view(vertices, left) == xt::view(vertices, right);
        };

        typedef std::unordered_map<uint32_t, uint32_t,
                                  decltype(hasher),
                                  decltype(comparer)> index_map_t;

        // Maps from an arbitrary vertex index to the index in which
        // that vertex first appears in the array, as described above.
        index_map_t to_first_mapper(10, hasher, comparer );

        
        // Simultaneously build the map and record the non-identity mappings.
        std::vector<uint32_t> changes;
        for (size_t i = 0; i < vertices.shape()[0]; ++i)
        {
            auto iter = to_first_mapper.find(i);
            if (iter == to_first_mapper.end())
            {
                to_first_mapper[i] = i;
            }
            else
            {
                changes.push_back(i);
                changes.push_back(iter->second);
            }
        }

        // Copy to an xtensor array.
        // This will need to change slightly when I upgrade our xtensor dependency.
        // They renamed xadapt() to adapt() (and maybe other changes?)
        std::vector<size_t> shape = { changes.size()/2, 2};
        index_map_array_t results = xt::xadapt(changes, shape);
        return results;
    }
}

#endif
