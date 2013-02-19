 #ifndef __NODE_H__
 #define __NODE_H__

#include "VSplit.h"

namespace CGoGN {

namespace Algo {

namespace Surface {

struct PFP: public PFP_STANDARD
{
    typedef EmbeddedMap2 MAP;
};

typedef PFP::MAP MAP;
typedef PFP::VEC3 VEC3;

/*
 * Classe définissant les différents noeuds de l'arbre des maillages progressifs
 */
class Node {
    public:
        Node(VSplit<PFP>* vs = NULL);
        ~Node();

        Node* getParent() { return m_parent; }
        void setParent(Node* parent) { m_parent = parent; }

        Node* getChild_1() { return m_child_1; }
        void setChild_1(Node* child_1) { m_child_1 = child_1; }

        Node* getChild_2() { return m_child_2; }
        void setChild_2(Node* child_2) { m_child_2 = child_2; }
        
        Node* getNeighbor() { return m_neighbor; }
        void setNeighbor(Node* neighbor) { m_neighbor = neighbor; }

        VSplit<PFP>* getVSplit() { return m_vsplit; }
        void setVSplit(VSplit<PFP>* vsplit) { m_vsplit = vsplit; }

        bool isActive() { return m_active; } 
        void setActive(bool active) { m_active = active; }
        
	    static std::string CGoGNnameOfType() { return "Noeud"; } 

    private:
        /*Liens dans l'arborescence*/
        Node* m_parent;
        Node* m_child_1;
        Node* m_child_2;
        Node* m_neighbor;
        
        /*Informations pour la transformation*/
        VSplit<PFP>* m_vsplit;
        bool m_active;
};
}//namespace Surface
}//namespace Algo
}//namespace CGoGN

#endif
