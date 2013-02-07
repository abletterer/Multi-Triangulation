/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009-2011, IGG Team, LSIIT, University of Strasbourg           *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.u-strasbg.fr/                                         *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#include "VDPMesh_App.h"
#include "Algo/Geometry/boundingbox.h"

using namespace CGoGN;

int main(int argc, char **argv)
{

	QApplication app(argc, argv);
	VDPMesh_App sqt;
	sqt.setWindowTitle("View-Dependent Progressive Meshes");

	sqt.createMap();

	sqt.show();

	return app.exec();
}

void VDPMesh_App::createMap() 
{
    position = m_map.addAttribute<VEC3, VERTEX>("position");

    //Création de deux nouvelles faces : 1 triangle et 1 carré
    Dart d1 = m_map.newFace(3);
    Dart d2 = m_map.newFace(4);
    
    //On "coud" les deux faces ensembles
    m_map.sewFaces(d1, d2);

    position[d1] = VEC3(0, 0, 0);
    position[m_map.phi1(d1)] = VEC3(2, 0, 0);
    position[m_map.phi_1(d1)] = VEC3(1, 2, 0);
    position[m_map.phi<11>(d2)] = VEC3(0, -2, 0);
    position[m_map.phi_1(d2)] = VEC3(2, -2, 0);
    
    Geom::BoundingBox<PFP::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP>(m_map, position);
    float lWidthObj = std::max<PFP::REAL>(std::max<PFP::REAL>(bb.size(0), bb.size(1)), bb.size(2));
    Geom::Vec3f lPosObj = (bb.min() +  bb.max()) / PFP::REAL(2);


    // send BB info to interface for centering on GL screen
	setParamObject(lWidthObj, lPosObj.data());
    
    show();

    // render the topo of the map without boundary darts
    SelectorDartNoBoundary<PFP::MAP> nb(m_map);
	m_render_topo->updateData<PFP>(m_map, position, 0.9f, 0.9f, nb);
}

void VDPMesh_App::cb_initGL() 
{
    m_render_topo = new Algo::Render::GL2::TopoRender();
}

void VDPMesh_App::cb_redraw()
{
    m_render_topo->drawTopo();
} 
