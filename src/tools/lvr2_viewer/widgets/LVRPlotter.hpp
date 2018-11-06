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

#ifndef LVRPLOTTER_HPP_
#define LVRPLOTTER_HPP_

#include <QWidget>

#include "lvr2/io/DataStruct.hpp"

namespace lvr2
{

enum class PlotMode
{
	LINE,
	BAR
};

class LVRPlotter : public QWidget
{
	Q_OBJECT

Q_SIGNALS:
    void mouseRelease();

public:

    LVRPlotter(QWidget* parent = (QWidget*)nullptr);
    virtual ~LVRPlotter();

	void setPlotMode(PlotMode mode);
	void setXRange(int min, int max);
	void setPoints(floatArr points, size_t numPoints);
	void setPoints(floatArr points, size_t numPoints, float min, float max);
	void removePoints();

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event); 
    ///Create axes, labeling and draw graph or bar chart
	void paintEvent(QPaintEvent *event);

private:
	floatArr m_points;
	size_t   m_numPoints;
	float    m_min;
	float    m_max;
	PlotMode m_mode;
	int		 m_minX;
	int		 m_maxX;
};

} /* namespace lvr2 */

#endif /* LVRPLOTTER_HPP_ */