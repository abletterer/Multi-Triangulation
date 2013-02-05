template <typename PFP>
VDProgressiveMesh<PFP>::ProgressiveMesh(
        MAP& map, DartMarker& inactive,
        VertexAttribute<typename PFP::VEC3>& positions
    ) :
    m_map(map), positionsTable(positions), inactiveMarker(inactive), dart
