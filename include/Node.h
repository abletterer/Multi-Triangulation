 #ifndef __NODE_H__
 #define __NODE_H__

#include "VSplit.h"

namespace CGoGN {

namespace Algo {

namespace Surface {

namespace VDPMesh {

struct PFP: public PFP_STANDARD
{
    typedef EmbeddedMap2 MAP;
};

typedef PFP::MAP MAP;
typedef PFP::VEC3 VEC3;

/*
 * Classe définissant les différents noeuds de l'arbre des maillages progressifs
 */
struct Node {
    public:
        Node(VSplit<PFP>* vs = NULL);
        ~Node();

        Node* getParent() { return m_parent; }
        void setParent(Node* parent) { m_parent = parent; }

        Node* getLeftChild() { return m_child_left; }
        void setLeftChild(Node* child_left) { m_child_left = child_left; }

        Node* getRightChild() { return m_child_right; }
        void setRightChild(Node* child_right) { m_child_right = child_right; }
        
        VSplit<PFP>* getVSplit() { return m_vsplit; }
        void setVSplit(VSplit<PFP>* vsplit) { m_vsplit = vsplit; }

        bool isActive() { return m_active; } 
        void setActive(bool active) { m_active = active; }
        
    private:
        /*Liens dans l'arborescence*/
        Node* m_parent;
        Node* m_child_left;
        Node* m_child_right;
        
        /*Informations pour la transformation*/
        VSplit<PFP>* m_vsplit;
        bool m_active;
};

typedef struct
{
    Node* node;
    static std::string CGoGNnameOfType() { return "NodeInfo" ; }
} NodeInfo;

} //namespace VDPMesh
} //namespace Surface
} //namespace Algo
} //namespace CGoGN

#endif
