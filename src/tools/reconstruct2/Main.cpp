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


 /**
 * @mainpage LSSR Toolkit Documentation
 *
 * @section Intro Introduction
 *
 * This software delivers tools to build surface reconstructions from
 * point cloud data and a simple viewer to display the results.
 * Additionally, the found surfaces can be classified into
 * categories in terms of floor, ceiling walls etc.. The main aim of this
 * project is to deliver fast and accurate surface extraction algorithms
 * for robotic applications such as teleoperation in unknown environments
 * and localization.
 *
 * LSSR is under permanent development and runs under Linux and MacOS.
 * A Windows version will be made available soon. The software is currently
 * under heavy reorganization, so it may happen that some interfaces change.
 * Additionally, not all features have been ported to the new structure,
 * so some functionalities may not be available at all times.
 *
 * In the current version the previously available plane clustering and
 * classification algorithms are not available. Please use the previous
 * release (0.1) if your are interested in these functionalities. The
 * missing features will be re-integrated in the next release.
 *
 * @section Compiling Compiling the software
 *
 * This software uses cmake \ref [http://www.cmake.org]. The easiest way
 * to build LSSR is to perform an out of source build:
 * \verbatim
 * mkdir build
 * cd build
 * cmake .. && make
 * cd ../bin
 * \endverbatim
 *
 * External library dependencies:
 *
 * <ul>
 * <li>OpenGL</li>
 * <li>OpenGL Utility Toolkit (glut)</li>
 * <li>OpenGL Utility Library (glu)</li>
 * <li>OpenMP</li>
 * <li>Boost
 *   <ul>
 *     <li>Thread</li>
 *     <li>Filesystem</li>
 *     <li>Program Options</li>
 *     <li>System</li>
 *   </ul>
 * </li>
 * <li>Qt 4.7 or above (for viewer and qviewer)</li>
 * <li>libQGLViewer 2.3.9 or newer (for qviewer)</li>
 * <li>X.Org X11 libXi runtime library</li>
 * <li>X.Org X11 libXmu/libXmuu runtime libraries</li>
 * </ul>
 *
 *
 * @section Usage Software Usage
 *
 * LSSR comes with a tool to reconstruct surfaces from unorganized
 * points and two viewer applications. To build a surface from the
 * provided example data set, just call from the program directory
 *
 * \verbatim
 * bin/reconstruct dat/points.pts -v5
 * \endverbatim
 *
 * This command will produce a triangle mesh of the data points stored in
 * a file called "triangle_mesh.ply". The data set and the reconstruction
 * can be displayed with one of the provided viewers. Important
 * parameters for "reconstruct" are
 *
 * <table border="0">
 * <tr>
 * <td width = 10%>
 * --help
 * </td>
 * <td>
 * Prints a short description of all relevant parameters.
 * </td>
 * </tr>
 * <tr>
 * <td>-v or -i</td>
* <td>
* <p>These parameters affect the accuracy of the reconstruction.
* <i>-i</i> defines the number of intersections on the longest side
* of the scanned scene and determines the corresponding voxelsize.
* Using this parameter is useful if the scaling of a scene is
* unknown. A value of about 100 will usually generate coarse surface.
* Experiment with this value to get a tradeoff between accuracy and
* mesh size. If you know the scaling of the objects, you can set a
* fixed voxelsize by using the <i>-v</i> parameter.
* </p>
* </td>
* </tr>
* <tr>
* <td>--ki, --kn, --kd</td>
* <td>These parameters determine the number of nearest neighbors used
* for initial normal estimation (<i>--kn</i>), normal interpolation
* (<i>--ki</i>) and distance value evaluation (<i>--kd</i>). In data
* sets with a lot of noise, increasing these values can lead to better
* approximations at the cost of running time. Increasing <i>--kd</i>
* usually helps to generate more continuous surfaces in sparse
* scans, but yields in a lot of smoothing, i.e. in the
* reconstuctions, sharp features will be smoothed out.</td>
* </tr>
* </table>
*
* @section API API Description
*
* A detailed API documentation will be made available soon.
*
* @section Tutorials Tutorials
*
* A set of tutorials how to use LSSR will be made available soon.
*/

#include <iostream>
#include <memory>
#include <tuple>
#include <stdlib.h>

#include <boost/optional.hpp>


// Program options for this tool
#ifndef DEBUG
  #include "Options.hpp"
#endif


// Local includes
// #include <lvr/reconstruction/FastReconstruction.hpp>
// #include <lvr/reconstruction/PointsetGrid.hpp>
// #include <lvr/reconstruction/FastBox.hpp>

// #include <lvr/io/PLYIO.hpp>
// #include <lvr/geometry/Matrix4.hpp>
// #include <lvr/geometry/HalfEdgeMesh.hpp>
// #include <lvr/texture/Texture.hpp>
// #include <lvr/texture/Transform.hpp>
// #include <lvr/texture/Texturizer.hpp>
// #include <lvr/texture/Statistics.hpp>
// #include <lvr/geometry/QuadricVertexCosts.hpp>
// #include <lvr/reconstruction/SharpBox.hpp>

#include <lvr/config/lvropenmp.hpp>
#include <lvr/io/Timestamp.hpp>
#include <lvr/io/Model.hpp>
#include <lvr/io/ModelFactory.hpp>
#include <lvr/io/PointBuffer.hpp>
#include <lvr/reconstruction/PointsetSurface.hpp>

#include <lvr2/geometry/HalfEdgeMesh.hpp>
#include <lvr2/geometry/BaseVector.hpp>
#include <lvr2/geometry/Vector.hpp>
#include <lvr2/geometry/Point.hpp>
#include <lvr2/geometry/Normal.hpp>
#include <lvr2/attrmaps/StableVector.hpp>
#include <lvr2/attrmaps/VectorMap.hpp>
#include <lvr2/algorithm/FinalizeAlgorithm.hpp>
#include <lvr2/algorithm/NormalAlgorithms.hpp>
#include <lvr2/algorithm/ColorAlgorithms.hpp>
#include <lvr2/geometry/BoundingBox.hpp>
#include <lvr2/algorithm/Planar.hpp>
#include <lvr2/algorithm/ClusterPainter.hpp>
#include <lvr2/algorithm/ClusterAlgorithms.hpp>
#include <lvr2/algorithm/CleanupAlgorithms.hpp>
#include <lvr2/algorithm/ReductionAlgorithms.hpp>
#include <lvr2/algorithm/Materializer.hpp>
#include <lvr2/algorithm/Texturizer.hpp>

#include <lvr2/reconstruction/AdaptiveKSearchSurface.hpp>
#include <lvr2/reconstruction/BilinearFastBox.hpp>
#include <lvr2/reconstruction/FastReconstruction.hpp>
#include <lvr2/reconstruction/PointsetSurface.hpp>
#include <lvr2/reconstruction/SearchTree.hpp>
#include <lvr2/reconstruction/SearchTreeFlann.hpp>
#include <lvr2/reconstruction/HashGrid.hpp>
#include <lvr2/reconstruction/PointsetGrid.hpp>
#include <lvr2/io/PointBuffer.hpp>
#include <lvr2/io/MeshBuffer.hpp>
#include <lvr2/util/Factories.hpp>
#include <lvr2/algorithm/MeshNavAlgorithms.hpp>
#include <lvr2/algorithm/UtilAlgorithms.hpp>

// PCL related includes
#ifdef LVR_USE_PCL
#include <lvr/reconstruction/PCLKSurface.hpp>
#endif

#if defined CUDA_FOUND
    #define GPU_FOUND

    #include <lvr/reconstruction/cuda/CudaSurface.hpp>

    typedef lvr::CudaSurface GpuSurface;
#elif defined OPENCL_FOUND
    #define GPU_FOUND

    #include <lvr/reconstruction/opencl/ClSurface.hpp>
    typedef lvr::ClSurface GpuSurface;
#endif



using boost::optional;
using std::unique_ptr;
using std::make_unique;

using lvr::timestamp;

using namespace lvr2;

using BaseVecT = BaseVector<float>;
using PsSurface = lvr::PointsetSurface<BaseVecT>;
// using AkSurface = lvr::AdaptiveKSearchSurface<BaseVecT, Normal<float>>;

// #ifdef LVR_USE_PCL
// using PclSurface = lvr::PCLKSurface<BaseVecT, Normal<float>>;
// #endif

/*
 * DUMMY TEST CODE STARTS HERE!!!
 */
using Vec = BaseVector<float>;


void lvr2Playground()
{
    auto buf = make_shared<PointBuffer<Vec>>();
    SearchTreeFlann<Vec> flann(buf);

    using Vec = lvr2::Vector<lvr2::BaseVector<float>>;
    using Poi = lvr2::Point<lvr2::BaseVector<float>>;

    Vec v1, v2;
    Poi p1, p2;

    v1 + p1;
    v1 + v2;
    // p1 + p2;

    v1.length();
    // v1.distance(v2);

    // p1.length();
    p1.distance(p2);

    lvr2::HalfEdgeMesh<lvr2::BaseVector<float>> mesh;
    auto v0H = mesh.addVertex(BaseVector<float>(0, 0, 0));
    auto v1H = mesh.addVertex(BaseVector<float>(1, 0, 0));
    auto v2H = mesh.addVertex(BaseVector<float>(1, 0, 1));
    auto bottomFace1 = mesh.addFace(v0H, v1H, v2H);
    mesh.getVertexPositionsOfFace(bottomFace1);

    using StableVector = lvr2::StableVector<VertexHandle, Vec>;

    // StableVector stuff
    cout << "========= StableVector =========" << endl;
    StableVector vec;
    VertexHandle handle1(1);
    VertexHandle handle2(0);
    cout << vec.numUsed() << std::endl;
    vec.push(v1);
    cout << vec.numUsed() << std::endl;
    vec.push(v2);
    cout << vec.numUsed() << std::endl;
    vec.erase(handle1);
    cout << vec.numUsed() << std::endl;
    auto vec1 = vec[handle2];

    cout << vec.size() << std::endl;
    cout << vec1.x << std::endl;

    vec = StableVector();

    for (int i = 0; i < 10; i++)
    {
        vec.push(Vec(i, 0, 0));
    }

    for (uint32_t i = 2; i < 12; i += 2)
    {
        vec.erase(VertexHandle(i));
    }

    for (auto handle : vec)
    {
        cout << handle << ": " << vec[handle] << endl;
    }

    // VectorMap stuff 2
    cout << "========= VectorMap =========" << endl;
    lvr2::VectorMap<VertexHandle, std::string> map;
    cout << map.numValues() << endl;
    map.insert(handle1, "test1");
    cout << map[handle1] << std::endl;
    cout << map.numValues() << endl;

    lvr2::VectorMap<VertexHandle, std::string> map2(10, "test");
    for (auto i = 0; i < 10; i++) {
        VertexHandle handleLoop(i);
        cout << map2[handleLoop] << endl;
    }
    cout << map2.numValues() << endl;

    VertexHandle handleLoop(5);
    map2[handleLoop] = "lalala";
    for (auto i = 0; i < 10; i++) {
        VertexHandle handleLoop(i);
        cout << map2[handleLoop] << endl;
    }
    cout << map2.numValues() << endl;

    handle1 = VertexHandle(42);
    map2.insert(handle1, "42 !!");
    cout << map2.numValues() << endl;
    auto opt = map2.get(handle1);
    if (opt) {
        cout << "found value! " << *opt << endl;
    }

    handle1 = VertexHandle(39);
    opt = map2.get(handle1);
    if (!opt) {
        cout << "found no value!" << endl;
    }
//    map2[handle1];

    handle1 = VertexHandle(42);
    map2.erase(handle1);
    cout << map2.numValues() << endl;
//    cout << map2[handle1] << endl;
}

/// Dummy type that prints stuff when important methods are called.
struct Verbosi
{
    std::string s;
    Verbosi() : s("default!") {}
    Verbosi(std::string s) : s(s) {}
    Verbosi(const Verbosi& other) : s(other.s) { cout << "Verbosi: Copy-Ctor - " << s << " from " << other.s << endl; }
    Verbosi(Verbosi&& other) noexcept : s(move(other.s)) { cout << "Verbosi: Move-Ctor - " << s << endl; }

    ~Verbosi() { cout << "Verbosi: Dtor - " << s << endl; }

    Verbosi& operator=(const Verbosi& other)
    {
        this->s = other.s;
        cout << "Verbosi: copy-assignment - " << s << "=" << other.s << endl;
    }
    Verbosi& operator=(Verbosi&& other)
    {
        this->s = move(other.s);
        cout << "Verbosi: move-assignment - " << s << "=" << other.s << endl;
    }
};

void testStableVector()
{
    {
        StableVector<VertexHandle, Verbosi> sv1;
        {
            cout << "### pushing 'a'" << endl;
            auto va = Verbosi("a");
            sv1.push(va);

            cout << "### reserve(2)" << endl;
            sv1.reserve(2);

            cout << "### pushing 'b' with copy" << endl;
            auto vb = Verbosi("b");
            sv1.push(vb);

            cout << "### reserve(3)" << endl;
            sv1.reserve(3);

            cout << "### pushing 'c' with move" << endl;
            sv1.push(Verbosi("c"));
            cout << "### end of inner scope" << endl;
        }
        cout << "### end of outer scope" << endl;
    }
}

void testTinyMap()
{
    TinyFaceMap<string> tm;
    tm.insert(FaceHandle(1), "hi");
    cout << tm[FaceHandle(1)] << endl;
    tm.insert(FaceHandle(37), "huhu");
    for (auto fH: tm)
    {
        cout << fH << endl;
    }
}

void createHouseFromNikolaus(lvr2::HalfEdgeMesh<lvr2::BaseVector<float>>& mesh)
{
    // scale
    float s = 5;

    // create house from nikolaus
    auto p0 = mesh.addVertex(BaseVector<float>(0, 0, 0));
    auto p1 = mesh.addVertex(BaseVector<float>(s, 0, 0));
    auto p2 = mesh.addVertex(BaseVector<float>(s, 0, s));
    auto p3 = mesh.addVertex(BaseVector<float>(0, 0, s));
    auto p4 = mesh.addVertex(BaseVector<float>(0, s, 0));
    auto p5 = mesh.addVertex(BaseVector<float>(s, s, 0));
    auto p6 = mesh.addVertex(BaseVector<float>(s, s, s));
    auto p7 = mesh.addVertex(BaseVector<float>(0, s, s));
    auto p8 = mesh.addVertex(BaseVector<float>(s/2, s+(s/2), s/2));

    auto bottomFace1 = mesh.addFace(p0, p1, p2);
    auto bottomFace2 = mesh.addFace(p0, p2, p3);

    auto rightFace1 = mesh.addFace(p1, p5, p6);
    auto rightFace2 = mesh.addFace(p1, p6, p2);

    auto leftFace1 = mesh.addFace(p3, p7, p4);
    auto leftFace2 = mesh.addFace(p4, p0, p3);

    auto frontFace1 = mesh.addFace(p7, p3, p2);
    auto frontFace2 = mesh.addFace(p2, p6, p7);

    auto backFace1 = mesh.addFace(p0, p4, p5);
    auto backFace2 = mesh.addFace(p5, p1, p0);

    auto roofFaceFront  = mesh.addFace(p7, p6, p8);
    auto roofFaceLeft   = mesh.addFace(p4, p7, p8);
    auto roofFaceBack   = mesh.addFace(p5, p4, p8);
    auto roofFaceRight  = mesh.addFace(p6, p5, p8);
}

void testFinalize(lvr2::HalfEdgeMesh<lvr2::BaseVector<float>>& mesh)
{
    createHouseFromNikolaus(mesh);
    mesh.debugCheckMeshIntegrity();

    // Check all for loops
    for (auto h : mesh.faces()) {}
    for (auto h : mesh.edges()) {}
    for (auto h : mesh.vertices()) {}

    FinalizeAlgorithm<BaseVector<float>> finalize;
    auto buffer = finalize.apply(mesh);

    // Create output model and save to file
    auto model = new lvr::Model(buffer->toOldBuffer());
    lvr::ModelPtr m(model);
    cout << timestamp << "Saving mesh." << endl;
    lvr::ModelFactory::saveModel( m, "triangle_mesh.ply");
}

void testClusterGrowing()
{
    lvr2::HalfEdgeMesh<lvr2::BaseVector<float>> mesh;
    createHouseFromNikolaus(mesh);
    auto normals = calcFaceNormals(mesh);
    auto clusterSet = planarClusterGrowing(mesh, normals, 0.999);
    cout << "Generated " << clusterSet.numCluster() << " clusters." << endl;
}

void testCollapseEdge()
{
    lvr2::HalfEdgeMesh<lvr2::BaseVector<float>> mesh;
    createHouseFromNikolaus(mesh);
    cout << "CheckMeshIntegrity: " << mesh.debugCheckMeshIntegrity() << endl;
    for (auto edgeH: mesh.edges())
    {
        cout << "ITERATION " << edgeH << endl;
        if (!mesh.isCollapsable(edgeH))
        {
            continue;
        }
        auto vertex = mesh.collapseEdge(edgeH);

        FinalizeAlgorithm<BaseVector<float>> finalize;
        auto buffer = finalize.apply(mesh);

        // Create output model and save to file
        auto model = new lvr::Model(buffer->toOldBuffer());
        lvr::ModelPtr m(model);
        cout << timestamp << "Saving mesh." << endl;
        std::stringstream ss;
        ss << "collapsed_nikolaus_" << edgeH.idx() << ".ply";
        lvr::ModelFactory::saveModel(m, ss.str());

        mesh.debugCheckMeshIntegrity();
        // cout << "CheckMeshIntegrity: " << << endl;
        // break;
    }

    cout << "done" << endl;

    FinalizeAlgorithm<BaseVector<float>> finalize;
    auto buffer = finalize.apply(mesh);

    // Create output model and save to file
    auto model = new lvr::Model(buffer->toOldBuffer());
    lvr::ModelPtr m(model);
    cout << timestamp << "Saving mesh." << endl;
    lvr::ModelFactory::saveModel( m, "triangle_mesh.ply");
}

void testEdgeFlip()
{
    lvr2::HalfEdgeMesh<lvr2::BaseVector<float>> mesh;
    createHouseFromNikolaus(mesh);

    FinalizeAlgorithm<BaseVector<float>> finalize;
    auto buffer = finalize.apply(mesh);
    auto model = new lvr::Model(buffer->toOldBuffer());
    lvr::ModelPtr m(model);
    lvr::ModelFactory::saveModel(m, "flipped_nikolaus_original.ply");

    for (auto edgeH: mesh.edges())
    {
        if (!mesh.isFlippable(edgeH))
        {
            continue;
        }
        cout << "ITERATION " << edgeH << endl;

        mesh.flipEdge(edgeH);

        // mesh.debugCheckMeshIntegrity();

        FinalizeAlgorithm<BaseVector<float>> finalize;
        auto buffer = finalize.apply(mesh);
        auto model = new lvr::Model(buffer->toOldBuffer());
        lvr::ModelPtr m(model);
        std::stringstream ss;
        ss << "flipped_nikolaus_" << edgeH.idx() << ".ply";
        lvr::ModelFactory::saveModel(m, ss.str());
    }
}

void testContourMethods()
{
    HalfEdgeMesh<Vec> mesh;
    createHouseFromNikolaus(mesh);

    // We want the contour of the "cluster" made up by face 0 and 1
    walkContour(mesh, EdgeHandle(0), [](auto vH, auto eH)
    {
        cout << vH << " " << eH << endl;
    }, [](auto faceH)
    {
        return faceH.idx() <= 1;
    });

    // Remove both bottom faces
    mesh.removeFace(FaceHandle(0));
    mesh.removeFace(FaceHandle(1));

    // Remove one roof face
    mesh.removeFace(FaceHandle(10));

    mesh.debugCheckMeshIntegrity();
    for (auto eH: mesh.edges())
    {
        if (mesh.numAdjacentFaces(eH) == 1)
        {
            vector<EdgeHandle> contourEdges;
            calcContourEdges(mesh, eH, contourEdges);
            for (auto contourEdgeH: contourEdges)
            {
                cout << contourEdgeH << " -> ";
            }
            cout << endl;

            vector<VertexHandle> contourVertices;
            calcContourVertices(mesh, eH, contourVertices);
            for (auto contourEdgeH: contourVertices)
            {
                cout << contourEdgeH << " -> ";
            }
            cout << endl << "---------------" << endl;
        }
    }
}

/*
 * DUMMY TEST CODE ENDS HERE!!!
 */

template <typename BaseVecT>
PointsetSurfacePtr<BaseVecT> loadPointCloud(const reconstruct::Options& options)
{
    // Create a point loader object
    lvr::ModelPtr model = lvr::ModelFactory::readModel(options.getInputFileName());

    // Parse loaded data
    if (!model)
    {
        cout << timestamp << "IO Error: Unable to parse " << options.getInputFileName() << endl;
        return nullptr;
    }
    auto buffer = make_shared<PointBuffer<Vec>>(*model->m_pointCloud);

    // Create a point cloud manager
    string pcm_name = options.getPCM();
    PointsetSurfacePtr<Vec> surface;

    // Create point set surface object
    if(pcm_name == "PCL")
    {
        cout << timestamp << "Using PCL as point cloud manager is not implemented yet!" << endl;
        panic_unimplemented("PCL as point cloud manager");
    }
    else if(pcm_name == "STANN" || pcm_name == "FLANN" || pcm_name == "NABO" || pcm_name == "NANOFLANN")
    {
        surface = make_shared<AdaptiveKSearchSurface<BaseVecT>>(
            buffer,
            pcm_name,
            options.getKn(),
            options.getKi(),
            options.getKd(),
            options.useRansac(),
            options.getScanPoseFile()
        );
    }
    else
    {
        cout << timestamp << "Unable to create PointCloudManager." << endl;
        cout << timestamp << "Unknown option '" << pcm_name << "'." << endl;
        return nullptr;
    }

    // Set search options for normal estimation and distance evaluation
    surface->setKd(options.getKd());
    surface->setKi(options.getKi());
    surface->setKn(options.getKn());

    // Calculate normals if necessary
    if(!buffer->hasNormals() || options.recalcNormals())
    {
        
        if(options.useGPU())
        {
            #ifdef GPU_FOUND
                std::vector<float> flipPoint = options.getFlippoint();
                size_t num_points;
                lvr::floatArr points;
                lvr::PointBuffer old_buffer = buffer->toOldBuffer();
                points = old_buffer.getPointArray(num_points);
                lvr::floatArr normals = lvr::floatArr(new float[ num_points * 3 ]);
                std::cout << "Generate GPU kd-tree..." << std::endl;
                GpuSurface gpu_surface(points, num_points);
                std::cout << "finished." << std::endl;

                gpu_surface.setKn(options.getKn());
                gpu_surface.setKi(options.getKi());
                gpu_surface.setFlippoint(flipPoint[0], flipPoint[1], flipPoint[2]);
                std::cout << "Start Normal Calculation..." << std::endl;
                gpu_surface.calculateNormals();
                gpu_surface.getNormals(normals);
                std::cout << "finished." << std::endl;
                old_buffer.setPointNormalArray(normals, num_points);
                buffer->copyNormalsFrom(old_buffer);
                gpu_surface.freeGPU();
            #else
                std::cout << "ERROR: GPU Driver not installed" << std::endl;
                surface->calculateSurfaceNormals();
            #endif
        }
        else
        {
            surface->calculateSurfaceNormals();
        }
    }
    else
    {
        cout << timestamp << "Using given normals." << endl;
    }

    return surface;
}

std::pair<shared_ptr<GridBase>, unique_ptr<FastReconstructionBase<Vec>>>
    createGridAndReconstruction(
        const reconstruct::Options& options,
        PointsetSurfacePtr<BaseVecT> surface
    )
{
    // Determine whether to use intersections or voxelsize
    bool useVoxelsize = options.getIntersections() <= 0;
    float resolution = useVoxelsize ? options.getVoxelsize() : options.getIntersections();

    // Create a point set grid for reconstruction
    string decompositionType = options.getDecomposition();

    // Fail safe check
    if(decompositionType != "MC" && decompositionType != "PMC" && decompositionType != "SF" )
    {
        cout << "Unsupported decomposition type " << decompositionType << ". Defaulting to PMC." << endl;
        decompositionType = "PMC";
    }

    if(decompositionType == "MC")
    {
        cout << "Decomposition type 'MC' is not implemented yet!" << endl;
        panic_unimplemented("decomposition type 'MC'");
    }
    else if(decompositionType == "PMC")
    {
        BilinearFastBox<Vec>::m_surface = surface;
        auto grid = std::make_shared<PointsetGrid<Vec, BilinearFastBox<Vec>>>(
            resolution,
            surface,
            surface->getBoundingBox(),
            useVoxelsize,
            options.extrude()
        );
        grid->calcDistanceValues();
        auto reconstruction = make_unique<FastReconstruction<Vec, BilinearFastBox<Vec>>>(grid);
        return make_pair(grid, std::move(reconstruction));
    }
    else if(decompositionType == "SF")
    {
        cout << "Decomposition type 'SF' is not implemented yet!" << endl;
        panic_unimplemented("decomposition type 'SF'");
    }

    return make_pair(nullptr, nullptr);
}


void testMeshnav(
    const BaseMesh<BaseVecT>& mesh,
    DenseVertexMap<Rgb8Color>& colorVertices,
    const VertexMap<Normal<BaseVecT>>& vertexNormals
)
{
    // calculate height differences
    DenseVertexMap<float> heightDifferences;
    heightDifferences = calcVertexHeightDiff(mesh, 31);
    float maxVal = -1;
    float minVal = -1;

    // search the max value of all height differences
    for (auto f: heightDifferences)
    {
        if(heightDifferences[f] > maxVal) maxVal = heightDifferences[f];
    }

    // fix visual color scheme by norming the height difference values
    for (auto f: heightDifferences)
    {
        heightDifferences[f] = heightDifferences[f] / maxVal;
    }

    auto roughness = calcVertexRoughness(mesh, 31, vertexNormals);

    maxVal = -1;

    for (auto f: roughness)
    {
        //cout << "Current height difference:" << height_differences[f] << endl;
        if(roughness[f] > maxVal) maxVal = roughness[f];
    }

    for (auto f: roughness)
    {
        roughness[f] = roughness[f] / maxVal;
    }

    DenseVertexMap<float> combCost;
    for(auto vH: mesh.vertices())
    {
        combCost.insert(vH, heightDifferences[vH] + roughness[vH]);
    }

    // create function pointer to the color conversion function
    Rgb8Color (*colorFunctionPointer)(float);
    colorFunctionPointer = &floatToGrayScaleColor;

    // create map of color vertices according to the calculated height differences
    colorVertices = lvr2::map<DenseAttrMap>(combCost, colorFunctionPointer);
}

int main(int argc, char** argv)
{
    // =======================================================================
    // Parse and print command line parameters
    // =======================================================================
    // Parse command line arguments
    reconstruct::Options options(argc, argv);

    // Exit if options had to generate a usage message
    // (this means required parameters are missing)
    if (options.printUsage())
    {
        return EXIT_SUCCESS;
    }

    std::cout << options << std::endl;


    // =======================================================================
    // Load (and potentially store) point cloud
    // =======================================================================
    lvr::OpenMPConfig::setNumThreads(options.getNumThreads());

    auto surface = loadPointCloud<Vec>(options);
    if (!surface)
    {
        cout << "Failed to create pointcloud. Exiting." << endl;
        return EXIT_FAILURE;
    }

    // Save points and normals only
    if(options.savePointNormals())
    {
        lvr::ModelPtr pn(new lvr::Model);
        auto oldBuffer = boost::make_shared<lvr::PointBuffer>(
            surface->pointBuffer()->toOldBuffer()
        );
        pn->m_pointCloud = oldBuffer;
        lvr::ModelFactory::saveModel(pn, "pointnormals.ply");
    }


    // =======================================================================
    // Reconstruct mesh from point cloud data
    // =======================================================================
    // Create an empty mesh
    lvr2::HalfEdgeMesh<Vec> mesh;

    shared_ptr<GridBase> grid;
    unique_ptr<FastReconstructionBase<Vec>> reconstruction;
    std::tie(grid, reconstruction) = createGridAndReconstruction(options, surface);

    // Reconstruct mesh
    reconstruction->getMesh(mesh);

    // Save grid to file
    if(options.saveGrid())
    {
        grid->saveGrid("fastgrid.grid");
    }

    // =======================================================================
    // Optimize and finalize mesh
    // =======================================================================
    // if(options.getDanglingArtifacts())
    // {
    //     mesh.removeDanglingArtifacts(options.getDanglingArtifacts());
    // }

    // // Optimize mesh
    // mesh.cleanContours(options.getCleanContourIterations());
    // mesh.setClassifier(options.getClassifier());
    // mesh.getClassifier().setMinRegionSize(options.getSmallRegionThreshold());

    // if(options.optimizePlanes())
    // {
    //     mesh.optimizePlanes(options.getPlaneIterations(),
    //             options.getNormalThreshold(),
    //             options.getMinPlaneSize(),
    //             options.getSmallRegionThreshold(),
    //             true);

    //     mesh.fillHoles(options.getFillHoles());
    //     mesh.optimizePlaneIntersections();
    //     mesh.restorePlanes(options.getMinPlaneSize());

    //     if(options.getNumEdgeCollapses())
    //     {
    //         QuadricVertexCosts<ColorVertex<float, unsigned char> , Normal<float> > c
    //             = QuadricVertexCosts<ColorVertex<float, unsigned char> , Normal<float> >(true);
    //         mesh.reduceMeshByCollapse(options.getNumEdgeCollapses(), c);
    //     }
    // }
    // else if(options.clusterPlanes())
    // {
    //     mesh.clusterRegions(options.getNormalThreshold(), options.getMinPlaneSize());
    //     mesh.fillHoles(options.getFillHoles());
    // }

    if(options.getDanglingArtifacts())
    {
        cout << timestamp << "Removing dangling artifacts" << endl;
        removeDanglingCluster(mesh, static_cast<size_t>(options.getDanglingArtifacts()));
    }

    // Magic number from lvr1 `cleanContours`...
    cleanContours(mesh, options.getCleanContourIterations(), 0.0001);

    naiveFillSmallHoles(mesh, options.getFillHoles(), false);

    auto faceNormals = calcFaceNormals(mesh);

    auto costLambda = [&](auto edgeH)
    {
        return collapseCostSimpleNormalDiff(mesh, faceNormals, edgeH);
    };

    // This is for debugging purposes! You can save a mesh whose colors can
    // represent float values ... or sth like that. Coolio!
    // {
    //     // Create vertex colors from other attributes
    //     auto edgeCosts = attrMapFromFunc<DenseAttrMap>(mesh.edges(), [&](auto edgeH){
    //         auto maybeCost = costLambda(edgeH);
    //         return maybeCost ? *maybeCost : 100;
    //     });
    //     float min, max;
    //     std::tie(min, max) = minMaxOfMap(edgeCosts);
    //     auto vertexCosts = attrMapFromFunc<DenseAttrMap>(mesh.vertices(), [&](VertexHandle vertexH)
    //     {
    //         float sum = 0.0;
    //         size_t count = 0;
    //         for (auto edgeH: mesh.getEdgesOfVertex(vertexH))
    //         {
    //             sum += edgeCosts[edgeH];
    //             count += 1;
    //         }
    //         const auto value = sum / count;
    //         return (value + min) / (max - min);
    //     });
    //     auto vertexColors = lvr2::map<DenseAttrMap>(vertexCosts, floatToGrayScaleColor);

    //     // Save mesh
    //     FinalizeAlgorithm<Vec> finalize;
    //     finalize.setColorData(vertexColors);
    //     auto buffer = finalize.apply(mesh);
    //     auto m = boost::make_shared<lvr::Model>(buffer);
    //     lvr::ModelFactory::saveModel(m, "debug_attribute.ply");
    // }

    // Reduce mesh complexity
    const auto reductionRatio = options.getEdgeCollapseReductionRatio();
    if (reductionRatio > 0.0)
    {
        if (reductionRatio > 1.0)
        {
            throw "The reduction ratio needs to be between 0 and 1!";
        }

        // Each edge collapse removes two faces in the general case.
        // TODO: maybe we should calculate this differently...
        const auto count = static_cast<size_t>((mesh.numFaces() / 2) * reductionRatio);
        cout << timestamp << "Reducing mesh by collapsing up to " << count << " edges" << endl;
        iterativeEdgeCollapse(mesh, count, costLambda);
        faceNormals = calcFaceNormals(mesh);
    }

    ClusterBiMap<FaceHandle> clusterBiMap;
    if(options.optimizePlanes())
    {
        clusterBiMap = iterativePlanarClusterGrowing(
            mesh,
            faceNormals,
            options.getNormalThreshold(),
            options.getPlaneIterations(),
            options.getMinPlaneSize()
        );

        if (options.getSmallRegionThreshold() > 0)
        {
            deleteSmallPlanarCluster(mesh, clusterBiMap, static_cast<size_t>(options.getSmallRegionThreshold()));
        }
    }
    else
    {
        clusterBiMap = planarClusterGrowing(mesh, faceNormals, options.getNormalThreshold());
    }

    // Prepare color data for finalizing
    ClusterPainter painter(clusterBiMap);
    auto clusterColors = optional<DenseClusterMap<Rgb8Color>>(painter.simpsons(mesh));
    auto vertexColors = calcColorFromPointCloud(mesh, surface);

    // Calc normals for vertices
    auto vertexNormals = calcVertexNormals(mesh, faceNormals, *surface);

    // Debug mesh
    //auto duplicateVertices = getDuplicateVertices(mesh);
    //cout << "duplicate vertices: " << duplicateVertices.size() << endl;

    // Finalize mesh (convert it to simple `MeshBuffer`)
    // FinalizeAlgorithm<Vec> finalize;
    // finalize.setNormalData(vertexNormals);
    // if (color_vertices)
    // {
    //    finalize.setColorData(colorVertices);
    // }
    // auto buffer = finalize.apply(mesh);


    // Prepare finalize algorithm
    ClusterFlatteningFinalizer<Vec> finalize(clusterBiMap);
    finalize.setVertexNormals(vertexNormals);

    // TODO:
    // Vielleicht sollten indv. vertex und cluster colors mit in den Materializer aufgenommen werden
    // Dafür spricht: alles mit Farben findet dann an derselben Stelle statt
    // dagegen spricht: Materializer macht aktuell nur face colors und keine vertex colors


    // Vertex colors:
    // If vertex colors should be generated from pointcloud:
    if (options.vertexColorsFromPointcloud())
    {
        // set vertex color data from pointcloud
        finalize.setVertexColors(*vertexColors);
    }
    else if (clusterColors)
    {
        // else: use simpsons painter for vertex coloring
        finalize.setClusterColors(*clusterColors);
    }

    // Materializer for face materials (colors and/or textures)
    Materializer<Vec> materializer(
        mesh,
        clusterBiMap,
        faceNormals,
        *surface
    );
    // When using textures ...
    if (options.generateTextures())
    {
        Texturizer<Vec> texturizer(
            options.getTexelSize(),
            options.getTexMinClusterSize(),
            options.getTexMaxClusterSize()
        );
        materializer.setTexturizer(texturizer);
    }
    // Generate materials
    MaterializerResult<Vec> matResult = materializer.generateMaterials();
    // When using textures ...
    if (options.generateTextures())
    {
        // Save them to disk
        materializer.saveTextures();
    }

    // Add material data to finalize algorithm
    finalize.setMaterializerResult(matResult);
    // Run finalize algorithm
    auto buffer = finalize.apply(mesh);

    // =======================================================================
    // Write all results (including the mesh) to file
    // =======================================================================
    // TODO2
    // // Write classification to file
    // if ( options.writeClassificationResult() )
    // {
    //     mesh.writeClassificationResult();
    // }

    // Create output model and save to file
    auto m = boost::make_shared<lvr::Model>(buffer->toOldBuffer(matResult));

    if(options.saveOriginalData())
    {
        m->m_pointCloud = boost::make_shared<lvr::PointBuffer>(
            surface->pointBuffer()->toOldBuffer()
        );
    }
    cout << timestamp << "Saving mesh." << endl;
    lvr::ModelFactory::saveModel(m, "triangle_mesh.ply");
    lvr::ModelFactory::saveModel(m, "triangle_mesh.obj");
    cout << timestamp << "Program end." << endl;

    return 0;
}
