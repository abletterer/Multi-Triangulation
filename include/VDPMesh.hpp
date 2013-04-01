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
	m_map(map), positionsTable(position), inactiveMarker(inactive), dartSelect(inactiveMarker), m_height(0)
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
    
    noeud = m_map.template getAttribute<EmbNode, VERTEX>("noeud") ;
	if(!noeud.isValid())
		noeud = m_map.template addAttribute<EmbNode, VERTEX>("noeud") ;
	
    CGoGNout << "  initializing interest box.." << CGoGNflush;
    m_bb = new Box();
    CGoGNout << "..done" << CGoGNendl;

    CGoGNout << "  initializing drawer.." << CGoGNflush;
    m_drawer = new Utils::Drawer();
	updateDrawer();
    CGoGNout << "..done" << CGoGNendl;
}

template <typename PFP>
VDProgressiveMesh<PFP>::~VDProgressiveMesh()
{
	m_active_nodes.clear();
    if(m_selector)
		delete m_selector ;
	for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		delete (*it) ;
	delete m_bb;
	delete m_drawer;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::addNodes() {
    TraversorV<MAP> trav(m_map);
    for(Dart d = trav.begin(); d!=trav.end(); d = trav.next()) {
        noeud[d].node = new Node(NULL, true, m_map.template getEmbedding<VERTEX>(d), 0);
        m_active_nodes.push_front(noeud[d].node);
        noeud[d].node->setCurrentPosition(m_active_nodes.begin());
    }
    m_height = 0;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::createPM(unsigned int percentWantedVertices)
{
	unsigned int nbVertices = m_map.template getNbOrbits<VERTEX>() ;
	unsigned int nbWantedVertices = nbVertices * percentWantedVertices / 100 ;
    
    CGoGNout << "  initializing nodes.." << CGoGNflush ;
    addNodes();
	CGoGNout << "..done" << CGoGNendl ;
	
    CGoGNout << "  creating PM (" << nbVertices << " vertices).." << /* flush */ CGoGNflush ;
    
	bool finished = false ;
	Dart d ;
	while(!finished)
	{
		if(!m_selector->nextEdge(d))
			break ;

		--nbVertices ;
		Dart d2 = m_map.phi2(m_map.phi_1(d)) ;
		Dart dd2 = m_map.phi2(m_map.phi_1(m_map.phi2(d))) ;
		Dart d1 = m_map.phi2(m_map.phi1(d)) ;
	    Dart dd1 = m_map.phi2(m_map.phi1(m_map.phi2(d))) ;

		VSplit<PFP>* vs = new VSplit<PFP>(m_map, d, dd2, d2, dd1, d1) ;	// create new VSplit node 
        
        Node* n = new Node(vs);   //Création du nouveau noeud de l'arbre

        Node* n_d2 = noeud[d2].node;
        Node* n_dd2 = noeud[dd2].node;

        /*Mise en place de la hiérarchie par rapport au nouveau noeud*/
        n->setLeftChild(n_d2);
        n->setRightChild(n_dd2);
        n_d2->setParent(n);
        n_dd2->setParent(n);

        /*Calcul de la hauteur du plus grand arbre de la forêt*/
        int height;
        if(noeud[d2].node->getHeight()>noeud[dd2].node->getHeight()) {
            height = noeud[d2].node->getHeight();
        }
        else {
            height = noeud[dd2].node->getHeight();
        }

        /*Suppression des deux anciens noeuds du front courant*/
        m_active_nodes.erase(n_d2->getCurrentPosition());
        m_active_nodes.erase(n_dd2->getCurrentPosition());
        n_d2->setActive(false);
        n_dd2->setActive(false);
        
        /*Ajout du nouveau noeud au front courant*/
        n->setActive(true);
        m_active_nodes.push_front(n);
        n->setCurrentPosition(m_active_nodes.begin());
        
        n->setHeight(height+1);
        
        if(m_height<=height) {
            /*Si la hauteur du plus grand arbre est inférieure ou égale à celle de l'arbre actuel*/
            /*NB: La hauteur du plus grand arbre ne sera jamais inférieure à celle de l'arbre courant*/
            ++m_height;
        }

		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		{
			(*it)->approximate(d) ;					// compute approximated attributes with its associated detail
			(*it)->saveApprox(d) ;
		}

		m_selector->updateBeforeCollapse(d) ;		// update selector

		edgeCollapse(n->getVSplit()) ;							// collapse edge

		unsigned int newV = m_map.template setOrbitEmbeddingOnNewCell<VERTEX>(d2) ;
		unsigned int newE1 = m_map.template setOrbitEmbeddingOnNewCell<EDGE>(d2) ;
		unsigned int newE2 = m_map.template setOrbitEmbeddingOnNewCell<EDGE>(dd2) ;
		n->getVSplit()->setApproxV(newV) ;
		n->getVSplit()->setApproxE1(newE1) ;
		n->getVSplit()->setApproxE2(newE2) ;
        
        noeud[d2].node = n; //Affectation du nouveau noeud a l'attribut de sommet
        n->setVertex(m_map.template getEmbedding<VERTEX>(d2));  //Indique le numéro de sommet pointant sur ce noeud
        
		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
			(*it)->affectApprox(d2);				// affect data to the resulting vertex

		m_selector->updateAfterCollapse(d2, dd2) ;	// update selector

		if(nbVertices <= nbWantedVertices)
			finished = true ;

	}
	delete m_selector ;
	m_selector = NULL ;

	CGoGNout << "..done (" << nbVertices << " vertices)" << CGoGNendl ;
    CGoGNout << m_active_nodes.size() << " active nodes" << CGoGNendl;
    CGoGNout << "Hauteur du plus grand arbre de la forêt : " << m_height << CGoGNendl;
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
    CGoGNout << "COARSEN" << CGoGNendl;
    std::list<Node*>::iterator it=m_active_nodes.begin();
    std::list<Node*>::iterator it_back;
    while(it != m_active_nodes.end()) {
    	it_back = it;
    	std::advance(it_back, 2);
        if(coarsen(*it)==1 && it!=--m_active_nodes.end()) {
        	it = it_back;
        }
        else {
        	++it;
        }
    }
    drawFront();
}

template <typename PFP>
int VDProgressiveMesh<PFP>::coarsen(Node* n)
{
    int res = 0;
    if(n && n->isActive() && !m_bb->contains(positionsTable[n->getVertex()])) {
        //Si n fait partie du front et qu'il n'est pas contenu dans la boîte d'intérêt
        Node* parent = n->getParent();
        if(parent && !parent->isActive()) {
            /*Si le noeud parent existe et qu'il ne fait pas partie du front*/
            Node* child_left = parent->getLeftChild();
            Node* child_right = parent->getRightChild();
            if(child_left && child_left->isActive()
            && child_right && child_right->isActive()) {
                //Si n a un noeud parent et que celui-ci ne fait pas partie du front
                VSplit<PFP>* vs = parent->getVSplit(); 
                Dart d = vs->getEdge();
                Dart d2 = vs->getLeftEdge();
                Dart dd2 = vs->getRightEdge();
                Dart d1 = vs->getOppositeLeftEdge();
                Dart dd1 = vs->getOppositeRightEdge();

                if( inactiveMarker.isMarked(d1)
                ||  inactiveMarker.isMarked(d2)
                ||  inactiveMarker.isMarked(dd1)
                ||  inactiveMarker.isMarked(dd2))
                    return res;

//                CGoGNout << "  Dart d : " << m_map.template getEmbedding<VERTEX>(d) << CGoGNendl;
//                CGoGNout << "  Dart phi1(d) : " << m_map.template getEmbedding<VERTEX>(m_map.phi1(d)) << CGoGNendl;
//                CGoGNout << "  Dart d1 : " << m_map.template getEmbedding<VERTEX>(d1) << CGoGNendl;
//                CGoGNout << "  Dart d2 : " << m_map.template getEmbedding<VERTEX>(d2) << CGoGNendl;
//				CGoGNout << "  Dart dd1 : " << m_map.template getEmbedding<VERTEX>(dd1) << CGoGNendl;
//				CGoGNout << "  Dart dd2 : " << m_map.template getEmbedding<VERTEX>(dd2) << CGoGNendl;
//				CGoGNout << "----------------------" << CGoGNendl;

                edgeCollapse(vs);

                m_map.template setOrbitEmbedding<VERTEX>(d2, vs->getApproxV());
	            m_map.template setOrbitEmbedding<EDGE>(d2, vs->getApproxE1());
                m_map.template setOrbitEmbedding<EDGE>(dd2, vs->getApproxE2());

//                CGoGNout << "  Dart d : " << m_map.template getEmbedding<VERTEX>(d) << CGoGNendl;
//                CGoGNout << "  Dart phi1(d) : " << m_map.template getEmbedding<VERTEX>(m_map.phi1(d)) << CGoGNendl;
//                CGoGNout << "  Dart d1 : " << m_map.template getEmbedding<VERTEX>(d1) << CGoGNendl;
//                CGoGNout << "  Dart d2 : " << m_map.template getEmbedding<VERTEX>(d2) << CGoGNendl;
//				CGoGNout << "  Dart dd1 : " << m_map.template getEmbedding<VERTEX>(dd1) << CGoGNendl;
//				CGoGNout << "  Dart dd2 : " << m_map.template getEmbedding<VERTEX>(dd2) << CGoGNendl;
//				CGoGNout << "----------------------" << CGoGNendl;
        
                //Mise a jour des informations de l'arbre
                m_active_nodes.erase(child_left->getCurrentPosition());
                m_active_nodes.erase(child_right->getCurrentPosition());
                child_left->setActive(false);
                child_right->setActive(false);
                parent->setActive(true);
                m_active_nodes.push_front(parent);
                parent->setCurrentPosition(m_active_nodes.begin());
                ++res;
            }
        }
    }
    return res;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::refine() {
    CGoGNout << "REFINE" << CGoGNendl;
    std::list<Node*>::iterator it_back;
    std::list<Node*>::iterator it=m_active_nodes.begin();
    while(it!=m_active_nodes.end()) {
    	it_back = it;
    	++it_back;
        refine(*it);
		it = it_back;
    }
    drawFront();
}

template <typename PFP>
int VDProgressiveMesh<PFP>::refine(Node* n)
{
    int res = 0;
    if(n && n->isActive() && m_bb->contains(positionsTable[n->getVertex()])) {
        //Si n fait partie du front et qu'il est contenu dans la boîte d'intérê
        Node* child_left = n->getLeftChild();
        Node* child_right = n->getRightChild();
        if( child_left && !child_left->isActive() 
        &&  child_right && !child_right->isActive()) {
            //Si n a deux fils et que ceux-ci ne font pas partie du front
            VSplit<PFP>* vs = n->getVSplit();

	        Dart d = vs->getEdge();
	        Dart dd = m_map.phi2(d);
	        Dart dd2 = vs->getRightEdge();
	        Dart d2 = vs->getLeftEdge();
	        Dart d1 = vs->getOppositeLeftEdge();
	        Dart dd1 = vs->getOppositeRightEdge();

            if( inactiveMarker.isMarked(d1)
            ||  inactiveMarker.isMarked(d2)
            ||  inactiveMarker.isMarked(dd1)
            ||  inactiveMarker.isMarked(dd2))
                return res;
        
	        unsigned int v1 = m_map.template getEmbedding<VERTEX>(d);				// get the embedding
	        unsigned int v2 = m_map.template getEmbedding<VERTEX>(dd);			// of the new vertices
	        unsigned int e1 = m_map.template getEmbedding<EDGE>(m_map.phi1(d));
	        unsigned int e2 = m_map.template getEmbedding<EDGE>(m_map.phi_1(d));	// and new edges
	        unsigned int e3 = m_map.template getEmbedding<EDGE>(m_map.phi1(dd));
	        unsigned int e4 = m_map.template getEmbedding<EDGE>(m_map.phi_1(dd));
	
            vertexSplit(vs);
            
	        m_map.template setOrbitEmbedding<VERTEX>(d, v1);		// embed the
	        m_map.template setOrbitEmbedding<VERTEX>(dd, v2);	// new vertices
	        m_map.template setOrbitEmbedding<EDGE>(d1, e1);
	        m_map.template setOrbitEmbedding<EDGE>(d2, e2);		// and new edges
	        m_map.template setOrbitEmbedding<EDGE>(dd1, e3);
	        m_map.template setOrbitEmbedding<EDGE>(dd2, e4);

            m_map.template copyDartEmbedding<VERTEX>(m_map.phi_1(d), d1);
            m_map.template copyDartEmbedding<VERTEX>(m_map.phi_1(dd), dd1);

            //Mise a jour des informations de l'arbre
            m_active_nodes.erase(n->getCurrentPosition());
            n->setActive(false);
            child_left->setActive(true);
            child_right->setActive(true);
            m_active_nodes.push_front(child_left);
            child_left->setCurrentPosition(m_active_nodes.begin());
            m_active_nodes.push_front(child_right);
            child_right->setCurrentPosition(m_active_nodes.begin());
            res++;
        }
    }
    return res;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::updateDrawer() {
	VEC3 a = m_bb->getPosMin();
	VEC3 b = VEC3(m_bb->getPosMax()[0], m_bb->getPosMin()[1], m_bb->getPosMin()[2]);
	VEC3 c = VEC3(m_bb->getPosMax()[0], m_bb->getPosMax()[1], m_bb->getPosMin()[2]);
	VEC3 d = VEC3(m_bb->getPosMin()[0], m_bb->getPosMax()[1], m_bb->getPosMin()[2]);
	VEC3 e = VEC3(m_bb->getPosMin()[0], m_bb->getPosMax()[1], m_bb->getPosMax()[2]);
	VEC3 f = VEC3(m_bb->getPosMin()[0], m_bb->getPosMin()[1], m_bb->getPosMax()[2]);
	VEC3 g = VEC3(m_bb->getPosMax()[0], m_bb->getPosMin()[1], m_bb->getPosMax()[2]);
	VEC3 h = m_bb->getPosMax();

	m_drawer->newList(GL_COMPILE_AND_EXECUTE);
	m_drawer->begin(GL_POINTS);
		m_drawer->lineWidth(7.0f);
		m_drawer->pointSize(9.0f);
		m_drawer->color3f(0.0f,0.7f,0.0f);
		/*glVertex3f(a[0], a[1], a[2]); glVertex3f(b[0], b[1], b[2]);
		glVertex3f(a[0], a[1], a[2]); glVertex3f(d[0], d[1], d[2]);
		glVertex3f(a[0], a[1], a[2]); glVertex3f(f[0], f[1], f[2]);

		glVertex3f(c[0], c[1], c[2]); glVertex3f(d[0], d[1], d[2]);
		glVertex3f(c[0], c[1], c[2]); glVertex3f(b[0], b[1], b[2]);
		glVertex3f(c[0], c[1], c[2]); glVertex3f(h[0], h[1], h[2]);

		glVertex3f(g[0], g[1], g[2]); glVertex3f(b[0], b[1], b[2]);
		glVertex3f(g[0], g[1], g[2]); glVertex3f(f[0], f[1], f[2]);
		glVertex3f(g[0], g[1], g[2]); glVertex3f(h[0], h[1], h[2]);

		glVertex3f(e[0], e[1], e[2]); glVertex3f(d[0], d[1], d[2]);
		glVertex3f(e[0], e[1], e[2]); glVertex3f(f[0], f[1], f[2]);
		glVertex3f(e[0], e[1], e[2]); glVertex3f(h[0], h[1], h[2]);*/

		glVertex3f(a[0], a[1], a[2]);
		glVertex3f(b[0], b[1], b[2]);
		glVertex3f(c[0], c[1], c[2]);
		glVertex3f(d[0], d[1], d[2]);
		glVertex3f(e[0], e[1], e[2]);
		glVertex3f(f[0], f[1], f[2]);
		glVertex3f(g[0], g[1], g[2]);
		glVertex3f(h[0], h[1], h[2]);
	m_drawer->end();
}

/*FONCTIONS DE DEBOGAGE*/
template <typename PFP>
void VDProgressiveMesh<PFP>::drawForest() {
    for(std::list<Node*>::iterator it = m_active_nodes.begin(); it != m_active_nodes.end(); ++it) {
        /*On parcourt l'ensemble des racines de la foret*/
        drawTree(*it);
        CGoGNout << CGoGNendl;
    } 
}

template <typename PFP>
void VDProgressiveMesh<PFP>::drawTree(Node* node) {
    CGoGNout << node->getVertex() << CGoGNflush;
    if(node) {
        if(node->getLeftChild()) {
            CGoGNout << " G( " << CGoGNflush;
            drawTree(node->getLeftChild());
            CGoGNout << " ) " << CGoGNflush;
        }
        if(node->getRightChild()) {
            CGoGNout << " D( " << CGoGNflush;
            drawTree(node->getRightChild());
            CGoGNout << " ) " << CGoGNflush;
        }
    }
}

template <typename PFP>
void VDProgressiveMesh<PFP>::drawFront() {
    CGoGNout << "Front courant : " << CGoGNendl;
    CGoGNout << "  " << m_active_nodes.size() << " noeuds actifs" << CGoGNendl;
    for(std::list<Node*>::iterator it = m_active_nodes.begin(); it != m_active_nodes.end(); ++it) {
        if(it!=--m_active_nodes.end())
            CGoGNout << (*it)->getVertex() << " | " << CGoGNflush;
        else
            CGoGNout << (*it)->getVertex() <<  CGoGNendl;
    }
}

} // namespace VDPMesh
} // namespace Surface
} // namespace Algo
} // namespace CGoGN
