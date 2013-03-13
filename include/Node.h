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
        Node(VSplit<PFP>* vsplit = NULL)
        : m_parent(NULL), m_child_left(NULL), m_child_right(NULL), m_vsplit(vsplit), m_active(false), m_position(NULL)
        {
        }

        ~Node() {
            delete m_vsplit;
        }

        Node* getParent() { return m_parent; }
        void setParent(Node* parent) { m_parent = parent; }

        Node* getLeftChild() { return m_child_left; }
        void setLeftChild(Node* child_left) { m_child_left = child_left; }

        Node* getRightChild() { return m_child_right; }
        void setRightChild(Node* child_right) { m_child_right = child_right; }
        
        VSplit<PFP>* getVSplit() { return m_vsplit; }
        void setVSplit(VSplit<PFP>* vsplit) { m_vsplit = vsplit; }

        unsigned int getVertex() { return m_vertex; }
        void setVertex(unsigned int vertex) { m_vertex = vertex;  }

        bool isActive() { return m_active; } 
        void setActive(bool active) { 
            m_active = active;
        }

        std::list<Node*>::iterator getCurrentPosition() { return m_position; }
        void setCurrentPosition(std::list<Node*>::iterator position) { m_position = position; }

        int getHeight() { return m_height;  }
        void setHeight(int height) { m_height = height;  }

        bool operator==(const Node& n) {
            return  m_parent == n.m_parent
                &&  m_child_left == n.m_child_left
                &&  m_child_right == n.m_child_right;
        }
        
    private:
        /*Liens dans l'arborescence*/
        Node* m_parent;
        Node* m_child_left;
        Node* m_child_right;
        
        /*Informations pour la transformation*/
        VSplit<PFP>* m_vsplit;
        unsigned int m_vertex;
        bool m_active;

        /*Informations pour l'acces dans le front courant*/
        std::list<Node*>::iterator m_position;

        /*Informations sur la position dans l'arbre (hauteur du noeud)*/
        int  m_height;
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
