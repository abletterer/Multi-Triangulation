#ifndef __VDPMESH_H__
#define __VDPMESH_H__

#include "Topology/map/embeddedMap2.h"
#include "Algo/Decimation/selector.h"
#include "Algo/Decimation/edgeSelector.h"
#include "Algo/Decimation/geometryApproximator.h"

#include "VSplit.h"

namespace CGoGN {

namespace Algo {

namespace Surface {

template <typename PFP>
class VDProgressiveMesh
{
public:
    typedef typename PFP::MAP MAP;
    typedef typename PFP::VEC3 VEC3;
    typedef typename PFP::REAL REAL;

private:
    MAP& m_map;
    VertexAttribute<typename PFP::VEC3>& positionsTable;

    DartMarker& inactiveMarker;
    SelectorUnmarked dartSelect;

    Algo::Surface::Decimation::EdgeSelector<PFP>* m_selector;
    std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*> m_approximators;
    
    Algo::Surface::Decimation::Approximator<PFP, VEC3, EDGE>* m_positionsApproximator;

public:
    VDProgressiveMesh(
        MAP& map, DartMarker& inactive,
        VertexAttribute<typename PFP::VEC3>& position
    );
    ~VDProgressiveMesh();

    void createPM(unsigned int percentWantedVertices);

    Algo::Surface::Decimation::EdgeSelector<PFP>* selector() { return m_selector; }
    std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>& approximators() { return m_approximators; }

    void edgeCollapse(VSplit<PFP>* vs); 
    void vertexSplit(VSplit<PFP>* vs);

    void coarsen();
    void refine();
private:
};
}
}
}

#include "VDPMesh.hpp"

#endif
