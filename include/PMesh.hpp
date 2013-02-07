/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009-2012, IGG Team, LSIIT, University of Strasbourg           *
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
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#include "Algo/Geometry/localFrame.h"

namespace CGoGN
{

namespace Algo
{

namespace Surface
{

namespace PMesh
{

template <typename PFP>
ProgressiveMesh<PFP>::ProgressiveMesh(
		MAP& map, DartMarker& inactive,
		VertexAttribute<typename PFP::VEC3>& position
	) :
	m_map(map), positionsTable(position), inactiveMarker(inactive), dartSelect(inactiveMarker)
{
	CGoGNout << "  creating approximator and predictor.." << CGoGNflush ;

	std::vector<VertexAttribute< typename PFP::VEC3>* > pos_v ;
	pos_v.push_back(&positionsTable) ;
	m_approximators.push_back(new Algo::Surface::Decimation::Approximator_MidEdge<PFP>(m_map, pos_v)) ;

	CGoGNout << "..done" << CGoGNendl ;

	CGoGNout << "  creating selector.." << CGoGNflush ;
			
    m_selector = new Algo::Surface::Decimation::EdgeSelector_Length<PFP>(m_map, positionsTable, m_approximators, dartSelect) ;
	
    CGoGNout << "..done" << CGoGNendl ;

	m_initOk = true ;

	CGoGNout << "  initializing approximators.." << CGoGNflush ;

	for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
	{
		if(! (*it)->init())
			m_initOk = false ;
		if((*it)->getApproximatedAttributeName() == "position")
			m_positionApproximator = reinterpret_cast<Algo::Surface::Decimation::Approximator<PFP, VEC3, EDGE>*>(*it) ;
	}

	CGoGNout << "..done" << CGoGNendl ;

	CGoGNout << "  initializing selector.." << CGoGNflush ;

	m_initOk = m_selector->init() ;

	CGoGNout << "..done" << CGoGNendl ;
}

template <typename PFP>
ProgressiveMesh<PFP>::~ProgressiveMesh()
{
	for(unsigned int i = 0; i < m_splits.size(); ++i)
		delete m_splits[i] ;
	if(m_selector)
		delete m_selector ;
	for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		delete (*it) ;
}

template <typename PFP>
void ProgressiveMesh<PFP>::createPM(unsigned int percentWantedVertices)
{
	unsigned int nbVertices = m_map.template getNbOrbits<VERTEX>() ;
	unsigned int nbWantedVertices = nbVertices * percentWantedVertices / 100 ;
	CGoGNout << "  creating PM (" << nbVertices << " vertices).." << /* flush */ CGoGNendl ;

	bool finished = false ;
	Dart d ;
	while(!finished)
	{
		if(!m_selector->nextEdge(d))
			break ;

		--nbVertices ;
		Dart d2 = m_map.phi2(m_map.phi_1(d)) ;
		Dart dd2 = m_map.phi2(m_map.phi_1(m_map.phi2(d))) ;

		VSplit<PFP>* vs = new VSplit<PFP>(m_map, d, dd2, d2) ;	// create new VSplit node
		m_splits.push_back(vs) ;								// and store it

		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		{
			(*it)->approximate(d) ;					// compute approximated attributes with its associated detail
			(*it)->saveApprox(d) ;
		}

		m_selector->updateBeforeCollapse(d) ;		// update selector

		edgeCollapse(vs) ;							// collapse edge

		unsigned int newV = m_map.template setOrbitEmbeddingOnNewCell<VERTEX>(d2) ;
		vs->setApproxV(newV) ;

		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
			(*it)->affectApprox(d2);				// affect data to the resulting vertex

		m_selector->updateAfterCollapse(d2, dd2) ;	// update selector

		if(nbVertices <= nbWantedVertices)
			finished = true ;
	}
	delete m_selector ;
	m_selector = NULL ;

	m_cur = m_splits.size() ;
	CGoGNout << "..done (" << nbVertices << " vertices)" << CGoGNendl ;
}

template <typename PFP>
void ProgressiveMesh<PFP>::edgeCollapse(VSplit<PFP>* vs)
{
	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ;

	inactiveMarker.markOrbit<FACE>(d) ;
	inactiveMarker.markOrbit<FACE>(dd) ;

	m_map.extractTrianglePair(d) ;
}

template <typename PFP>
void ProgressiveMesh<PFP>::vertexSplit(VSplit<PFP>* vs)
{
	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ;
	Dart d2 = vs->getLeftEdge() ;
	Dart dd2 = vs->getRightEdge() ;

	m_map.insertTrianglePair(d, d2, dd2) ;

	inactiveMarker.unmarkOrbit<FACE>(d) ;
	inactiveMarker.unmarkOrbit<FACE>(dd) ;
}

template <typename PFP>
void ProgressiveMesh<PFP>::coarsen()
{
	if(m_cur == m_splits.size())
		return ;

	VSplit<PFP>* vs = m_splits[m_cur] ; // get the split node
	++m_cur ;

	// Dart d = vs->getEdge() ;
	// Dart dd = m_map.phi2(d) ;		// get some darts
	Dart d2 = vs->getLeftEdge() ;

	edgeCollapse(vs) ;	// collapse edge

	m_map.template setOrbitEmbedding<VERTEX>(d2, vs->getApproxV()) ;
}

template <typename PFP>
void ProgressiveMesh<PFP>::refine()
{
	if(m_cur == 0)
		return ;

	--m_cur ;
	VSplit<PFP>* vs = m_splits[m_cur] ; // get the split node

	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ; 		// get some darts

	unsigned int v1 = m_map.template getEmbedding<VERTEX>(d) ;				// get the embedding
	unsigned int v2 = m_map.template getEmbedding<VERTEX>(dd) ;			// of the new vertices

	vertexSplit(vs) ; // split vertex

	m_map.template setOrbitEmbedding<VERTEX>(d, v1) ;		// embed the
	m_map.template setOrbitEmbedding<VERTEX>(dd, v2) ;	// new vertices
}

template <typename PFP>
void ProgressiveMesh<PFP>::gotoLevel(unsigned int l)
{
	if(l == m_cur || l > m_splits.size() || l < 0)
		return ;

	if(l > m_cur)
		while(m_cur != l)
			coarsen() ;
	else
		while(m_cur != l)
			refine() ;
}

} // namespace PMesh
} // namespace Surface
} // namespace Algo
} // namespace CGoGN
