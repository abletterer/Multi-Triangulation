#ifndef __BOX_H__
#define __BOX_H__

namespace CGoGN {

namespace Algo {

namespace Surface {

namespace VDPMesh {

/*
 * Classe définissant une boîte d'intérêt
 */
struct Box {
    public:
        Box(PFP::VEC3 pos_min = PFP::VEC3(0., 0., 0.), PFP::VEC3 pos_max = PFP::VEC3(0., 0., 0.))
           : m_pos_min(pos_min), m_pos_max(pos_max) 
        {} 
        
        ~Box()
        {}

        PFP::VEC3 getPosMin() { return m_pos_min; }
        void setPosMin(PFP::VEC3 pos_min) { m_pos_min = pos_min; }
        
        PFP::VEC3 getPosMax() { return m_pos_max; }
        void setPosMax(PFP::VEC3 pos_max) { m_pos_max = pos_max; }

        bool contains(PFP::VEC3 pos) {
            return  m_pos_min <= pos && pos <= m_pos_max;
        }
    private:
        PFP::VEC3 m_pos_min;    //Position [xmin;ymin;zmin]
        PFP::VEC3 m_pos_max;    //Position [xmax;ymax;zmax]
};
} //namespace VDPMesh
} //namespace Surface
} //namespace Algo
} //namespace CGoGN

#endif
