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

namespace VDPMesh
{
    
template <typename PFP>
VDProgressiveMesh<PFP>::VDProgressiveMesh(
		MAP& map, DartMarker& inactive,
		VertexAttribute<typename PFP::VEC3>& position
	) :
	m_map(map), positionsTable(position), inactiveMarker(inactive), dartSelect(inactiveMarker)
{
	CGoGNout << "  creating approximator .." << CGoGNflush ;

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

    noeud = m_map.template getAttribute<EmbNode, VERTEX>("noeud");

	CGoGNout << "..done" << CGoGNendl ;
}

template <typename PFP>
VDProgressiveMesh<PFP>::~VDProgressiveMesh()
{
	for(unsigned int i = 0; i < m_splits.size(); ++i)
		delete m_splits[i] ;
	if(m_selector)
		delete m_selector ;
	for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		delete (*it) ;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::addNodes() {
    AttributeContainer container = m_map.template getAttributeContainer<VERTEX>();
    for(unsigned int i = container.begin(); i!=container.end(); container.next(i)) {
        noeud[i].node = new Node();
        noeud[i].node->setActive(false);
    }
}

template <typename PFP>
void VDProgressiveMesh<PFP>::createPM(unsigned int percentWantedVertices)
{
	unsigned int nbVertices = m_map.template getNbOrbits<VERTEX>() ;
	unsigned int nbWantedVertices = nbVertices * percentWantedVertices / 100 ;
	CGoGNout << "  creating PM (" << nbVertices << " vertices).." << /* flush */ CGoGNendl ;
	
    
    CGoGNout << "  addingNodes.." << /* flush */ CGoGNflush ;
    addNodes();
	CGoGNout << "  ..done" << /* flush */ CGoGNendl ;

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

        Node* n = new Node();
        n->setVSplit(vs);

        
		m_splits.push_back(vs);

		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		{
			(*it)->approximate(d) ;					// compute approximated attributes with its associated detail
			(*it)->saveApprox(d) ;
		}

		m_selector->updateBeforeCollapse(d) ;		// update selector

		edgeCollapse(vs) ;							// collapse edge

		unsigned int newV = m_map.template setOrbitEmbeddingOnNewCell<VERTEX>(d2) ;
		unsigned int newE1 = m_map.template setOrbitEmbeddingOnNewCell<EDGE>(d2) ;
		unsigned int newE2 = m_map.template setOrbitEmbeddingOnNewCell<EDGE>(dd2) ;
		vs->setApproxV(newV) ;
		vs->setApproxE1(newE1) ;
		vs->setApproxE2(newE2) ;

		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
			(*it)->affectApprox(d2);				// affect data to the resulting vertex

		m_selector->updateAfterCollapse(d2, dd2) ;	// update selector
        
        //Mise en place de la hiérarchie de la forêt
        /*Traversor2EV<MAP> trav(m_map, d);
        int i=0;
        for(Dart it = trav.begin(); it!=trav.end(); it = trav.next()) {
            if(i==0) {
                n->setLeftChild(noeud[it].node);
            }
            else if(i==1) {
                n->setRightChild(noeud[it].node);
            }
            noeud[it].node->setParent(n);
            ++i;
        }*/

		if(nbVertices <= nbWantedVertices)
			finished = true ;
	}
	delete m_selector ;
	m_selector = NULL ;

	CGoGNout << "..done (" << nbVertices << " vertices)" << CGoGNendl ;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::edgeCollapse(VSplit<PFP>* vs)
{
	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ;

	inactiveMarker.markOrbit<FACE>(d) ;
	inactiveMarker.markOrbit<FACE>(dd) ;

	m_map.extractTrianglePair(d) ;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::vertexSplit(VSplit<PFP>* vs)
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
void VDProgressiveMesh<PFP>::coarsen()
{
	/*
    if(m_cur == m_splits.size())
		return ;

	VSplit<PFP>* vs = m_splits[m_cur] ; // get the split node
	++m_cur ;
    

	// Dart d = vs->getEdge() ;
	// Dart dd = m_map.phi2(d) ;		// get some darts
	Dart d2 = vs->getLeftEdge() ;
	Dart dd2 = vs->getLeftEdge() ;

	edgeCollapse(vs) ;	// collapse edge

	m_map.template setOrbitEmbedding<VERTEX>(d2, vs->getApproxV()) ;
	m_map.template setOrbitEmbedding<EDGE>(d2, vs->getApproxE1()) ;
	m_map.template setOrbitEmbedding<EDGE>(dd2, vs->getApproxE2()) ;
    */
}

template <typename PFP>
void VDProgressiveMesh<PFP>::refine()
{
	/*
    if(m_cur == 0)
		return ;

	--m_cur ;
	VSplit<PFP>* vs = m_splits[m_cur] ; // get the split node

	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ; 		// get some darts
	Dart dd2 = vs->getRightEdge() ;
	Dart d2 = vs->getLeftEdge() ;
	Dart d1 = m_map.phi2(d2) ;
	Dart dd1 = m_map.phi2(dd2) ;

	unsigned int v1 = m_map.template getEmbedding<VERTEX>(d) ;				// get the embedding
	unsigned int v2 = m_map.template getEmbedding<VERTEX>(dd) ;			// of the new vertices
	unsigned int e1 = m_map.template getEmbedding<EDGE>(m_map.phi1(d)) ;
	unsigned int e2 = m_map.template getEmbedding<EDGE>(m_map.phi_1(d)) ;	// and new edges
	unsigned int e3 = m_map.template getEmbedding<EDGE>(m_map.phi1(dd)) ;
	unsigned int e4 = m_map.template getEmbedding<EDGE>(m_map.phi_1(dd)) ;

	vertexSplit(vs) ; // split vertex

	m_map.template setOrbitEmbedding<VERTEX>(d, v1) ;		// embed the
	m_map.template setOrbitEmbedding<VERTEX>(dd, v2) ;	// new vertices
	m_map.template setOrbitEmbedding<EDGE>(d1, e1) ;
	m_map.template setOrbitEmbedding<EDGE>(d2, e2) ;		// and new edges
	m_map.template setOrbitEmbedding<EDGE>(dd1, e3) ;
	m_map.template setOrbitEmbedding<EDGE>(dd2, e4) ;
    */
}

} // namespace VDPMesh
} // namespace Surface
} // namespace Algo
} // namespace CGoGN
