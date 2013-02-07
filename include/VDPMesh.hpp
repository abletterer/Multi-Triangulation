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
        VertexAttribute<typename PFP::VEC3>& positions
    ) :
    m_map(map), positionsTable(positions), inactiveMarker(inactive), dartSelect(inactiveMarker)
{
    CGoGNout << "  creating approximator.." << CGoGNflush;

    std::vector<VertexAttribute<typename PFP::VEC3>*> pos_v;
    pos_v.push_back(&positionsTable);

    m_approximators.push_back(new Algo::Surface::Decimation::Approximator_MidEdge<PFP>(m_map, pos_v));

    CGoGNout << "..done" << CGoGNendl;

    CGoGNout << "  creating selector.." << CGoGNflush;
    
    m_selector = new Algo::Surface::Decimation::EdgeSelector_Length<PFP>(m_map, positionsTable, m_approximators, dartSelect);
    
    CGoGNout << "..done" << CGoGNendl;

    CGoGNout << "  initializing selector.." << CGoGNflush;

    m_selector->init();

    CGoGNout << "..done" << CGoGNendl;
}

template <typename PFP>
VDProgressiveMesh<PFP>::~VDProgressiveMesh() 
{
    if(m_selector)
        delete m_selector;
    for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
        delete (*it);
}

template <typename PFP>
void VDProgressiveMesh<PFP>::initialiseTree()
{
    //Ajout d'un attribut de sommet de type Node
    EdgeAttribute<Node*> noeud = m_map.addAttribute<Node*, EGDE>("noeud");

    AttributeContainer& container = m_map.getAttributeContainer<EDGE>();
    for(unsigned int i = container.begin(); i != container.end(); container.next(i)) 
    {
        noeud[i] = new Node();
    }
}

template <typename PFP>
void VDProgressiveMesh<PFP>::createPM(unsigned int percentWantedVertices)
{
    unsigned int nbVertices = m_map.template getNbOrbits<VERTEX>();
    unsigned int nbWantedVertices = nbVertices * percentWantedVertices;
	
    CGoGNout << "  creating PM (" << nbVertices << " vertices).." << /* flush */ CGoGNendl ;

    bool finished = false;
    Dart d;
    while(!finished) 
    {
        if(!m_selector->nextEdge(d))
            break;

        --nbVertices;
        Dart d2 = m_map.phi2(m_map.phi_1(d));
        Dart dd2 = m_map.phi2(m_map.phi_1(m_map.phi2(d)));

        VSplit<PFP>* vs = new VSplit<PFP>(m_map, d, d2, dd2);

        EdgeAttribute<Node*> noeud = m_map.getAttribute<Node*, EDGE>("noeud");
        Node* node = noeud[d];  //On récupère le noeud associé à l'arête courante

        node->setVSplit(vs);    //On associe le vsplit construit au noeud de l'arête courante

        for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
        {
            (*it)->approximate(d);
            (*it)->saveApprox(d);
        }

        m_selector->updateBeforeCollapse(d);
        
        edgeCollapse(VSplit<PFP>(m_map, d, d2, dd2));

        unsigned int newV = m_map.template setOrbitEmbeddingOnNewCell<VERTEX>(d2);
        vs->setApproxV(newV);
		
        for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
			(*it)->affectApprox(d2);

        m_selector->updateAfterCollapse(d2, dd2);

        if(nbVertices <= nbWantedVertices)
            finished = true;
    }
    delete m_selector;
    m_selector = NULL;
	
    CGoGNout << "..done (" << nbVertices << " vertices)" << CGoGNendl ;

}

template <typename PFP>
void VDProgressiveMesh<PFP>::edgeCollapse(VSplit<PFP>* vs)
{
    Dart d = vs->getEdge();
	Dart dd = m_map.phi2(d);
	
    inactiveMarker.markOrbit<FACE>(d);
	inactiveMarker.markOrbit<FACE>(dd);

	m_map.extractTrianglePair(d);
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

}

template <typename PFP>
void VDProgressiveMesh<PFP>::refine()
{
    
}

}
}
}
}
