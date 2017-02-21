//
//  nn_utils.hpp
//  Dig
//
//  Created by DB on 2016-9-15
//  Copyright (c) 2016 DB. All rights reserved.
//

#ifndef __NN_UTILS_HPP
#define __NN_UTILS_HPP

#include <vector>

// #include "Dense"
#ifdef BLAZE
    #include "src/external/eigen/Dense"
    #include "src/include/public.hpp"  // for Neighbor
#else
    #include "Dense"
    #include "public.hpp"
#endif

using std::vector;

using Eigen::Matrix;
using Eigen::MatrixXd;
using Eigen::ArrayXd;
using Eigen::VectorXd;
using Eigen::RowVectorXd;
using Eigen::MatrixXi;
using Eigen::ArrayXi;

using Eigen::Dynamic;
using Eigen::RowMajor;
using Eigen::ColMajor;

typedef Eigen::Matrix<double, Dynamic, Dynamic, RowMajor> RowMatrixXd;

static const int16_t kInvalidIdx = -1;

namespace nn {

// ------------------------------------------------ neighbor munging

template<template<class...> class Container>
inline void sort_neighbors_ascending_distance(Container<Neighbor>& neighbors) {
    std::sort(std::begin(neighbors), std::end(neighbors),
        [](const Neighbor& a, const Neighbor& b) -> bool
        {
            return a.dist < b.dist;
        }
    );
}

template<template<class...> class Container>
inline void sort_neighbors_ascending_idx(Container<Neighbor>& neighbors) {
    std::sort(std::begin(neighbors), std::end(neighbors),
        [](const Neighbor& a, const Neighbor& b) -> bool
        {
            return a.idx < b.idx;
        }
    );
}

/** given a sorted collection of the best neighbors found so far, (potentially)
 * inserts a new neighbor into the list such that the sorting is preserved;
 * assumes the neighbors container contains only valid neighbors and is sorted
 *
 * Returns the distance to the last (farthest) neighbor after possible insertion
 */
template<template<class...> class Container>
inline dist_t maybe_insert_neighbor(
	Container<Neighbor>& neighbors_bsf, Neighbor newNeighbor)
{
    assert(neighbors_bsf.size() > 0);
	size_t len = neighbors_bsf.size();
    size_t i = len - 1;
    auto dist = newNeighbor.dist;

    if (dist < neighbors_bsf[i].dist) {
        neighbors_bsf[i] = newNeighbor;
    }

    while (i > 0 && neighbors_bsf[i-1].dist > dist) {
        // swap new and previous neighbor
        Neighbor tmp = neighbors_bsf[i-1];

        neighbors_bsf[i-1] = neighbors_bsf[i];
        neighbors_bsf[i] = tmp;
        i--;
    }
    return neighbors_bsf[len - 1].dist;
}
template<template<class...> class Container>
inline dist_t maybe_insert_neighbor(Container<Neighbor>& neighbors_bsf,
    double dist, typename Neighbor::idx_t idx)
{
	return maybe_insert_neighbor(neighbors_bsf, Neighbor{idx, dist});
}

template<template<class...> class Container,
    template<class...> class Container2>
inline dist_t maybe_insert_neighbors(Container<Neighbor>& neighbors_bsf,
    const Container2<Neighbor>& potential_neighbors)
{
    dist_t d_bsf = kMaxDist;
    for (auto& n : potential_neighbors) {
        d_bsf = maybe_insert_neighbor(neighbors_bsf, n);
    }
    return d_bsf;
}


template<class T>
inline vector<Neighbor> knn_from_dists(const T* dists, size_t len, size_t k) {
    assert(k > 0);
    assert(len > 0);
    k = std::min(k, len);
    vector<Neighbor> ret(k); // warning: populates it with k 0s
    for (idx_t i = 0; i < k; i++) {
		ret[i] = Neighbor{i, dists[i]};
    }
    sort_neighbors_ascending_distance(ret);
    for (idx_t i = k; i < len; i++) {
        maybe_insert_neighbor(ret, dists[i], i);
    }
    return ret;
}

template<class T, class R>
inline vector<Neighbor> neighbors_in_radius(const T* dists, size_t len, R radius_sq) {
    vector<Neighbor> neighbors;
    for (idx_t i = 0; i < len; i++) {
        auto dist = dists[i];
        if (dists[i] < radius_sq) {
			neighbors.emplace_back(i, dist);
        }
    }
    return neighbors;
}

// inline vector<vector<idx_t> > idxs_from_nested_neighbors(
//     const vector<vector<Neighbor> >& neighbors)
// {
//     return map([](const vector<Neighbor>& inner_neighbors) -> std::vector<idx_t> {
//         return map([](const Neighbor& n) -> idx_t {
//             return n.idx;
//         }, inner_neighbors);
//     }, neighbors);
// }

} // namespace nn
#endif // __NN_UTILS_HPP