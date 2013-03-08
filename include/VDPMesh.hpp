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
	
    CGoGNout << "..done" << CGoGNendl ;

    noeud = m_map.template getAttribute<EmbNode, VERTEX>("noeud");
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
    TraversorCell<MAP, VERTEX> trav(m_map);
    for(Dart d = trav.begin(); d!=trav.end(); d = trav.next()) {
        noeud[d].node = new Node();
        noeud[d].node->setActive(true);
        m_active_nodes.push_back(noeud[d].node);
        if(m_active_nodes.size()==1) {
            //A l'insertion du premier élément
            noeud[d].node->setCurrentPosition(m_active_nodes.begin());
        }
        else {
            noeud[d].node->setCurrentPosition(--m_active_nodes.end());
        }
    }
}

template <typename PFP>
void VDProgressiveMesh<PFP>::createPM(unsigned int percentWantedVertices)
{
	unsigned int nbVertices = m_map.template getNbOrbits<VERTEX>() ;
	unsigned int nbWantedVertices = nbVertices * percentWantedVertices / 100 ;
    
    CGoGNout << "  addingNodes.." << CGoGNflush ;
    addNodes();
	CGoGNout << "..done" << CGoGNendl ;
	
    CGoGNout << "  creating PM (" << nbVertices << " vertices).." << /* flush */ CGoGNflush ;	

	bool finished = false ;
	Dart d ;
    std::list<Dart>* ids = new std::list<Dart>();
	while(!finished)
	{
		if(!m_selector->nextEdge(d))
			break ;

		--nbVertices ;
		Dart d2 = m_map.phi2(m_map.phi_1(d)) ;
		Dart dd2 = m_map.phi2(m_map.phi_1(m_map.phi2(d))) ;

		VSplit<PFP>* vs = new VSplit<PFP>(m_map, d, dd2, d2) ;	// create new VSplit node 
        
        Node* n = new Node(vs);   //Création du nouveau noeud de l'arbre
        Node* n_d2 = noeud[d2].node;
        Node* n_dd2 = noeud[dd2].node;

        /*Mise en place de la hiérarchie par rapport au nouveau noeud*/
        n->setLeftChild(n_d2);
        n->setRightChild(n_dd2);
        n_d2->setParent(n);
        n_dd2->setParent(n);

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
        
        /*Suppression des deux anciens noeuds du front courant*/
        m_active_nodes.erase(n_d2->getCurrentPosition());
        m_active_nodes.erase(n_dd2->getCurrentPosition());
        n_d2->setActive(false);
        n_dd2->setActive(false);
        
        /*Ajout du nouveau noeud au front courant*/
        n->setActive(true);
        m_active_nodes.push_back(n);
        n->setCurrentPosition(--m_active_nodes.end());
        
        noeud[d2].node = n; //Affectation du nouveau noeud a l'attribut de sommet

        ids->push_back(vs->getEdge());

        if(std::find(ids->begin(), ids->end(), vs->getRightEdge())!=ids->end()) {
            CGoGNout << "Fils droit trouvé" << CGoGNendl;
            if(std::find(ids->begin(), ids->end(), vs->getLeftEdge())!=ids->end()) {
                CGoGNout << "Fils gauche trouvé" << CGoGNendl;
            }
            break;
        }
        
        /*CGoGNout << "Noeud :" << CGoGNendl;
        CGoGNout << "  Sommet :" << vs->getEdge() << CGoGNendl;
        CGoGNout << "  Fils droit :" << vs->getRightEdge() << CGoGNendl;
        CGoGNout << "  Fils gauche :" << vs->getLeftEdge() << CGoGNendl;*/
        
		if(nbVertices <= nbWantedVertices)
			finished = true ;

	}
	delete m_selector ;
	m_selector = NULL ;

	CGoGNout << "..done (" << nbVertices << " vertices)" << CGoGNendl ;
    CGoGNout << m_active_nodes.size() << " active nodes" << CGoGNendl;
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
void VDProgressiveMesh<PFP>::coarsen() {
    int i=0;
    for(std::list<Node*>::iterator it=m_active_nodes.begin(); it!=m_active_nodes.end(); ++it) {
        i += coarsen(*it);
    }
    CGoGNout << i << CGoGNflush;
}

template <typename PFP>
int VDProgressiveMesh<PFP>::coarsen(Node* n)
{
	/*
    if(m_cur == m_splits.size())
		return ;

	VSplit<PFP>* vs = m_splits[m_cur] ; // get the split node
	++m_cur ;
    

	// Dart d = vs->getEdge() ;
	// Dart dd = m_map.phi2(d) ;		// get some darts
	Dart d2 = vs->getLeftEdge() ;
	Dart dd2 = vs->getRightEdge() ;

	edgeCollapse(vs) ;	// collapse edge

	m_map.template setOrbitEmbedding<VERTEX>(d2, vs->getApproxV()) ;
	m_map.template setOrbitEmbedding<EDGE>(d2, vs->getApproxE1()) ;
	m_map.template setOrbitEmbedding<EDGE>(dd2, vs->getApproxE2()) ;
    */

    int res = 0;
    if(n->isActive()) {
        //Si n fait partie du front
        Node* parent = n->getParent();
        if(parent!=NULL && !parent->isActive()) {
            //Si n a un noeud parent et que celui-ci ne fait pas partie du front
            VSplit<PFP>* vs = n->getVSplit(); 
            Dart d2 = vs->getLeftEdge();
            Dart dd2 = vs->getRightEdge();

            edgeCollapse(vs);

            m_map.template setOrbitEmbedding<VERTEX>(d2, vs->getApproxV());
            m_map.template setOrbitEmbedding<EDGE>(d2, vs->getApproxE1());
            m_map.template setOrbitEmbedding<EDGE>(dd2, vs->getApproxE2());

            //Mise a jour des informations de l'arbre
            m_active_nodes.erase(parent->getLeftChild()->getCurrentPosition());
            m_active_nodes.erase(parent->getRightChild()->getCurrentPosition());
            parent->getLeftChild()->setActive(false);
            parent->getRightChild()->setActive(false);
            parent->setActive(true);
            m_active_nodes.push_back(parent);
            parent->setCurrentPosition(--m_active_nodes.end());
            ++res;
        }
    }
    return res;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::refine() {
    for(std::list<Node*>::iterator it=m_active_nodes.begin(); it!=m_active_nodes.end(); ++it) {
        if(refine(*it)==1)
            break;        
    }
}

template <typename PFP>
int VDProgressiveMesh<PFP>::refine(Node* n)
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

    int res = 0;
    if(n->isActive()) {
        //Si n fait partie du front
        Node* child_left = n->getLeftChild();
        Node* child_right = n->getRightChild();
        if(child_left!=NULL && child_right!=NULL 
        && !child_left->isActive() && !child_right->isActive()) {
            //Si n a deux fils et que ceux-ci ne font pas partie du front
            VSplit<PFP>* vs = n->getVSplit();

	        Dart d = vs->getEdge();
	        Dart dd = m_map.phi2(d);
	        Dart dd2 = vs->getRightEdge();
	        Dart d2 = vs->getLeftEdge();
	        Dart d1 = m_map.phi2(d2);
	        Dart dd1 = m_map.phi2(dd2);

	        unsigned int v1 = m_map.template getEmbedding<VERTEX>(d);				// get the embedding
	        unsigned int v2 = m_map.template getEmbedding<VERTEX>(dd);			// of the new vertices
	        unsigned int e1 = m_map.template getEmbedding<EDGE>(m_map.phi1(d));
	        unsigned int e2 = m_map.template getEmbedding<EDGE>(m_map.phi_1(d));	// and new edges
	        unsigned int e3 = m_map.template getEmbedding<EDGE>(m_map.phi1(dd));
	        unsigned int e4 = m_map.template getEmbedding<EDGE>(m_map.phi_1(dd));
	
            //VERIFIER SI NOEUD RECUPERE DU SOMMET CORRESPOND AUX FILS ESTIMES

            CGoGNout << "  Arete d2 : " << m_map.template getEmbedding<EDGE>(d2) << CGoGNendl;
            CGoGNout << "  Arete dd2 : " << m_map.template getEmbedding<EDGE>(dd2) << CGoGNendl;
            
            vertexSplit(vs);
            
	        m_map.template setOrbitEmbedding<VERTEX>(d, v1);		// embed the
	        m_map.template setOrbitEmbedding<VERTEX>(dd, v2);	// new vertices
	        m_map.template setOrbitEmbedding<EDGE>(d1, e1);
	        m_map.template setOrbitEmbedding<EDGE>(d2, e2);		// and new edges
	        m_map.template setOrbitEmbedding<EDGE>(dd1, e3);
	        m_map.template setOrbitEmbedding<EDGE>(dd2, e4);
            
            //Mise a jour des informations de l'arbre
            m_active_nodes.erase(n->getCurrentPosition());
            n->setActive(false);
            child_left->setActive(true);
            child_right->setActive(true);
            m_active_nodes.push_back(child_left);
            child_left->setCurrentPosition(--m_active_nodes.end());
            m_active_nodes.push_back(child_right);
            child_right->setCurrentPosition(--m_active_nodes.end());
            res++;
        }
    }
    return res;
}

} // namespace VDPMesh
} // namespace Surface
} // namespace Algo
} // namespace CGoGN
