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
 * LVRPointBufferBridge.cpp
 *
 *  @date Feb 6, 2014
 *  @author Thomas Wiemann
 */
#include "LVRPointBufferBridge.hpp"
#include "LVRModelBridge.hpp"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPointData.h>

namespace lvr
{

inline unsigned char floatToColor(float f)
{
    return f * 255;
}

LVRPointBufferBridge::LVRPointBufferBridge(PointBufferPtr pointCloud)
{
    m_useSpectralChannel.r = true;
    m_useSpectralChannel.g = true;
    m_useSpectralChannel.b = true;
    m_spectralChannels.r = 0;
    m_spectralChannels.g = 0;
    m_spectralChannels.b = 0;

    // default: solid color gradient
    m_useGradient = false;
    m_useNDVI = false;
    m_useNormalizedGradient = false;
    m_spectralGradientChannel = 0;
    m_spectralGradient = HOT;

    if(pointCloud)
    {
        // Save pc data
        m_pointBuffer = pointCloud;

        // default: visible light
        m_spectralChannels.r = pointCloud->getChannel(612, 0);
        m_spectralChannels.g = pointCloud->getChannel(552, 0);
        m_spectralChannels.b = pointCloud->getChannel(462, 0);

        // Generate vtk actor representation
        computePointCloudActor(pointCloud);

        // Save meta information
        size_t numColors(0), numNormals(0);
        m_numPoints = pointCloud->getNumPoints();
        pointCloud->getPointNormalArray(numNormals);
        pointCloud->getPointColorArray(numColors);

        if(numColors > 0) m_hasColors = true;
        if(numNormals > 0) m_hasNormals = true;
    }
    else
    {
        m_numPoints = 0;
        m_hasNormals = false;
        m_hasColors = false;
    }
}

void LVRPointBufferBridge::setSpectralChannels(color<size_t> channels, color<bool> use_channel)
{
    if (channels == m_spectralChannels && use_channel == m_useSpectralChannel)
    {
        return;
    }

    m_spectralChannels = channels;
    m_useSpectralChannel = use_channel;

    refreshSpectralChannel();
}


void LVRPointBufferBridge::refreshSpectralChannel()
{
    size_t n, n_channels;
    floatArr spec = m_pointBuffer->getPointSpectralChannelsArray(n, n_channels);

    if (!n)
    {
        return;
    }

    vtkSmartPointer<vtkUnsignedCharArray> scalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
    scalars->SetNumberOfComponents(3);
    scalars->SetName("Colors");
    scalars->SetNumberOfTuples(n);

    #pragma omp parallel for
    for (vtkIdType i = 0; i < n; i++)
    {
        int specIndex = n_channels * i;
        unsigned char speccolor[3];
        speccolor[0] = m_useSpectralChannel.r ? floatToColor(spec[specIndex + m_spectralChannels.r]) : 0;
        speccolor[1] = m_useSpectralChannel.g ? floatToColor(spec[specIndex + m_spectralChannels.g]) : 0;
        speccolor[2] = m_useSpectralChannel.b ? floatToColor(spec[specIndex + m_spectralChannels.b]) : 0;

#if VTK_MAJOR_VERSION < 7
        scalars->SetTupleValue(i, speccolor);
#else
        scalars->SetTypedTuple(i, speccolor); // no idea how the new method is called
#endif
    }

    m_pointCloudActor->GetMapper()->GetInput()->GetPointData()->SetScalars(scalars);
}

void LVRPointBufferBridge::getSpectralChannels(color<size_t> &channels, color<bool> &use_channel) const
{
    channels = m_spectralChannels;
    use_channel = m_useSpectralChannel;
}

void LVRPointBufferBridge::setSpectralColorGradient(GradientType gradient, size_t channel, bool normalized, bool useNDVI)
{
    if (m_spectralGradient == gradient && m_spectralGradientChannel == channel
        && m_useNormalizedGradient == normalized && m_useNDVI == useNDVI)
    {
        return;
    }

    m_spectralGradient = gradient;
    m_spectralGradientChannel = channel;
    m_useNormalizedGradient = normalized;
    m_useNDVI = useNDVI;

    refreshSpectralGradient();
}

void LVRPointBufferBridge::refreshSpectralGradient()
{
    size_t n, n_channels;
    floatArr spec = m_pointBuffer->getPointSpectralChannelsArray(n, n_channels);

    if (!n)
    {
        return;
    }

    float ndviMax = 0;
    float ndviMin = 1;

    floatArr ndvi;
    if (m_useNDVI)
    {
        ndvi = floatArr(new float[n]);

        size_t redStart = m_pointBuffer->getChannel(400, 0);
        size_t redEnd = m_pointBuffer->getChannel(700, 1);
        size_t nearRedStart = m_pointBuffer->getChannel(700, n_channels - 2);
        size_t nearRedEnd = m_pointBuffer->getChannel(1100, n_channels - 1);

        #pragma omp parallel for reduction(max : ndviMax), reduction(min : ndviMin)
        for (int i = 0; i < n; i++)
        {
            float redTotal = 0;
            float nearRedTotal = 0;
            float* specPixel = spec.get() + n_channels * i;

            for (int channel = redStart; channel < redEnd; channel++)
            {
                redTotal += specPixel[channel];
            }
            for (int channel = nearRedStart; channel < nearRedEnd; channel++)
            {
                nearRedTotal += specPixel[channel];
            }

            float red = redTotal / (redEnd - redStart);
            float nearRed = nearRedTotal / (nearRedEnd - nearRedStart);

            float val = (nearRed - red) / (nearRed + red);
            val = (val + 1) / 2; // NDVI is in range [-1, 1] => transform to [0, 1]
            ndvi[i] = val;

            if (val < ndviMin) ndviMin = val;
            if (val > ndviMax) ndviMax = val;
        }
    }

    vtkSmartPointer<vtkUnsignedCharArray> scalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
    scalars->SetNumberOfComponents(3);
    scalars->SetName("Colors");
    scalars->SetNumberOfTuples(n);

    unsigned char min = 0;
    unsigned char max = 255;
    if(m_useNormalizedGradient && !m_useNDVI)
    {
        float max_val = spec[m_spectralGradientChannel], min_val = spec[m_spectralGradientChannel];
        #pragma omp parallel for reduction(max : max_val), reduction(min : min_val)
        for (int i = 0; i < n; i++)
        {
            int specIndex = n_channels * i + m_spectralGradientChannel;
            if(spec[specIndex] > max_val)
            {
                max_val = spec[specIndex];
            }
            if(spec[specIndex] < min_val)
            {
                min_val = spec[specIndex];
            }
        }
        min = floatToColor(min_val);
        max = floatToColor(max_val);
    }

    if(m_useNormalizedGradient && m_useNDVI)
    {
        min = floatToColor(ndviMin);
        max = floatToColor(ndviMax);
    }

    ColorMap colorMap(max - min);

	#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        int specIndex = n_channels * i;
        float color[3];

        if (m_useNDVI)
        {
            colorMap.getColor(color, floatToColor(ndvi[i]) - min, m_spectralGradient);
        }
        else
        {
            colorMap.getColor(color, floatToColor(spec[specIndex + m_spectralGradientChannel]) - min, m_spectralGradient);
        }

        unsigned char speccolor[3];
        speccolor[0] = color[0] * 255;
        speccolor[1] = color[1] * 255;
        speccolor[2] = color[2] * 255;

#if VTK_MAJOR_VERSION < 7
        scalars->SetTupleValue(i, speccolor);
#else
        scalars->SetTypedTuple(i, speccolor); // no idea how the new method is called
#endif
    }

    m_pointCloudActor->GetMapper()->GetInput()->GetPointData()->SetScalars(scalars);
}

void LVRPointBufferBridge::getSpectralColorGradient(GradientType &gradient, size_t &channel, bool &normalized, bool &useNDVI) const
{
    gradient = m_spectralGradient;
    channel = m_spectralGradientChannel;
    normalized = m_useNormalizedGradient;
    useNDVI = m_useNDVI;
}

void LVRPointBufferBridge::useGradient(bool useGradient)
{
    m_useGradient = useGradient;

    // update
    if(useGradient)
    {
        refreshSpectralGradient();
    }
    else
    {
        refreshSpectralChannel();
    }
}

PointBufferPtr LVRPointBufferBridge::getPointBuffer()
{
    return m_pointBuffer;
}

size_t  LVRPointBufferBridge::getNumPoints()
{
    return m_numPoints;
}

bool LVRPointBufferBridge::hasNormals()
{
    return m_hasNormals;
}

bool LVRPointBufferBridge::hasColors()
{
    return m_hasColors;
}

LVRPointBufferBridge::~LVRPointBufferBridge()
{
}

void LVRPointBufferBridge::computePointCloudActor(PointBufferPtr pc)
{
    if(pc)
    {
        m_pointCloudActor = vtkSmartPointer<vtkActor>::New();

        // Setup a poly data object
        vtkSmartPointer<vtkPolyData>    vtk_polyData = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints>      vtk_points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray>   vtk_cells = vtkSmartPointer<vtkCellArray>::New();

        vtkSmartPointer<vtkUnsignedCharArray> scalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
        scalars->SetNumberOfComponents(3);
        scalars->SetName("Colors");

        double point[3];
        size_t n, n_c, n_s_p, n_s_channels;
        floatArr points = pc->getPointArray(n);
        ucharArr colors = pc->getPointColorArray(n_c);
        floatArr spec = pc->getPointSpectralChannelsArray(n_s_p, n_s_channels);

        scalars->SetNumberOfTuples(n_s_p ? n_s_p : n);
        vtk_points->SetNumberOfPoints(n_s_p ? n_s_p : n);

        for(vtkIdType i = 0; i < n; i++)
        {
            int index = 3 * i;
            point[0] = points[index    ];
            point[1] = points[index + 1];
            point[2] = points[index + 2];

            if(n_s_p)
            {
                if (i >= n_s_p) // only take points with spectral information
                {
                    break;
                }
                int specIndex = n_s_channels * i;
                unsigned char speccolor[3];
                speccolor[0] = floatToColor(spec[specIndex + m_spectralChannels.r]);
                speccolor[1] = floatToColor(spec[specIndex + m_spectralChannels.g]);
                speccolor[2] = floatToColor(spec[specIndex + m_spectralChannels.b]);

#if VTK_MAJOR_VERSION < 7
                scalars->SetTupleValue(i, speccolor);
#else
                scalars->SetTypedTuple(i, speccolor); // no idea how the new method is called
#endif
            }
            else if(n_c)
            {
                unsigned char color[3];
                color[0] = colors[index];
                color[1] = colors[index + 1];
                color[2] = colors[index + 2];

#if VTK_MAJOR_VERSION < 7
                scalars->SetTupleValue(i, color);
#else
                scalars->SetTypedTuple(i, color); // no idea how the new method is called
#endif
            }

            vtk_points->SetPoint(i, point);
            vtk_cells->InsertNextCell(1, &i);
        }

        vtk_polyData->SetPoints(vtk_points);
        vtk_polyData->SetVerts(vtk_cells);

        if(n_c || n_s_p)
        {
            vtk_polyData->GetPointData()->SetScalars(scalars);
        }

        // Create poly data mapper and generate actor
        //vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
#ifdef LVR_USE_VTK5
        mapper->SetInput(vtk_polyData);
#else
        mapper->SetInputData(vtk_polyData);
#endif
        m_pointCloudActor->SetMapper(mapper);
    }
}

LVRPointBufferBridge::LVRPointBufferBridge(const LVRPointBufferBridge& b)
{
    m_pointCloudActor   = b.m_pointCloudActor;
    m_hasColors         = b.m_hasColors;
    m_hasNormals        = b.m_hasNormals;
    m_numPoints         = b.m_numPoints;
    m_spectralChannels  = b.m_spectralChannels;
    m_useSpectralChannel= b.m_useSpectralChannel;
    m_useGradient       = b.m_useGradient;
    m_useNDVI           = b.m_useNDVI;
    m_spectralGradient  = b.m_spectralGradient;
    m_useNormalizedGradient = b.m_useNormalizedGradient;
    m_spectralGradientChannel = b.m_spectralGradientChannel;
}

void LVRPointBufferBridge::setBaseColor(float r, float g, float b)
{
    vtkSmartPointer<vtkProperty> p = m_pointCloudActor->GetProperty();
    p->SetColor(r, g, b);
    m_pointCloudActor->SetProperty(p);
}

void LVRPointBufferBridge::setPointSize(int pointSize)
{
    vtkSmartPointer<vtkProperty> p = m_pointCloudActor->GetProperty();
    p->SetPointSize(pointSize);
    m_pointCloudActor->SetProperty(p);
}

void LVRPointBufferBridge::setOpacity(float opacityValue)
{
    vtkSmartPointer<vtkProperty> p = m_pointCloudActor->GetProperty();
    p->SetOpacity(opacityValue);
    m_pointCloudActor->SetProperty(p);
}

void LVRPointBufferBridge::setVisibility(bool visible)
{
    if(visible) m_pointCloudActor->VisibilityOn();
    else m_pointCloudActor->VisibilityOff();
}

vtkSmartPointer<vtkActor> LVRPointBufferBridge::getPointCloudActor()
{
    return m_pointCloudActor;
}


} /* namespace lvr */
