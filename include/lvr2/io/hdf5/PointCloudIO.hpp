#pragma once

#ifndef LVR2_IO_HDF5_POINTBUFFERIO_HPP
#define LVR2_IO_HDF5_POINTBUFFERIO_HPP

#include <boost/optional.hpp>

#include "lvr2/io/PointBuffer.hpp"

// Dependencies
#include "ChannelIO.hpp"
#include "VariantChannelIO.hpp"

namespace lvr2 {

namespace hdf5features {

/**
 * @class PointCloudIO 
 * @brief Hdf5IO Feature for handling PointBuffer related IO
 * 
 * This Feature of the Hdf5IO handles the IO of a PointBuffer object.
 * 
 * Example:
 * @code
 * MyHdf5IO io;
 * PointBufferPtr pointcloud, pointcloud_in;
 * 
 * // writing
 * io.open("test.h5");
 * io.save("apointcloud", pointcloud);
 * 
 * // reading
 * pointcloud_in = io.loadPointCloud("apointcloud");
 * 
 * @endcode
 * 
 * Generates attributes at hdf5 group:
 * - IO: PointCloudIO
 * - CLASS: PointBuffer
 * 
 * Dependencies:
 * - VariantChannelIO
 * 
 */
template<typename Derived>
class PointCloudIO {
public:
    void save(std::string name, const PointBufferPtr& buffer);
    void save(HighFive::Group& group, const PointBufferPtr& buffer);

    PointBufferPtr load(std::string name);
    PointBufferPtr load(HighFive::Group& group);
    PointBufferPtr loadPointCloud(std::string name);

protected:

    bool isPointCloud(HighFive::Group& group);

    Derived* m_file_access = static_cast<Derived*>(this);
    // dependencies
    VariantChannelIO<Derived>* m_vchannel_io = static_cast<VariantChannelIO<Derived>*>(m_file_access);

    static constexpr char* ID = "PointCloudIO";
    static constexpr char* OBJID = "PointBuffer";
};


} // hdf5features

} // namespace lvr2 

#include "PointCloudIO.tcc"

#endif // LVR2_IO_HDF5_POINTBUFFERIO_HPP