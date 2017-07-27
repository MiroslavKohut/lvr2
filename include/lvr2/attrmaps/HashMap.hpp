/* Copyright (C) 2011 Uni Osnabrück
 * This file is part of the LAS VEGAS Reconstruction Toolkit,
 *
 * LAS VEGAS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LAS VEGAS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */


/*
 * HashMap.hpp
 *
 *  @date 27.07.2017
 */

#ifndef LVR2_ATTRMAPS_HASHMAP_H_
#define LVR2_ATTRMAPS_HASHMAP_H_

#include <unordered_map>

#include <lvr2/attrmaps/AttributeMap.hpp>

using std::unordered_map;

namespace lvr2
{

template<typename HandleT, typename ValueT>
class HashMap : public AttributeMap<HandleT, ValueT>
{
public:
    bool containsKey(HandleT key) const final;
    optional<ValueT> insert(HandleT key, const ValueT& value) final;
    optional<ValueT> remove(HandleT key) final;
    void clear() final;
    optional<ValueT&> get(HandleT key) final;
    optional<const ValueT&> get(HandleT key) const final;
    size_t numValues() const final;

    AttributeMapHandleIteratorPtr<HandleT> begin() const final;
    AttributeMapHandleIteratorPtr<HandleT> end() const final;

private:
    unordered_map<HandleT, ValueT> m_map;
};

template<typename HandleT, typename ValueT>
class HashMapIterator : public AttributeMapHandleIterator<HandleT>
{
public:
    HashMapIterator(typename unordered_map<HandleT, ValueT>::const_iterator iter);

    AttributeMapHandleIterator<HandleT>& operator++() final;
    bool operator==(const AttributeMapHandleIterator<HandleT>& other) const final;
    bool operator!=(const AttributeMapHandleIterator<HandleT>& other) const final;
    HandleT operator*() const final;

private:
    typename unordered_map<HandleT, ValueT>::const_iterator m_iter;
};

} // namespace lvr2

#include <lvr2/attrmaps/HashMap.tcc>

#endif /* LVR2_ATTRMAPS_HASHMAP_H_ */
