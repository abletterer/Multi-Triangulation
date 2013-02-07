#include "Node.h"

namespace CGoGN {

namespace Algo {

namespace Surface {

Node::Node(VSplit<PFP>* vs) 
    : m_vsplit(vsplit)
{
}

Node::~Node() 
{
    delete m_vsplit;
}

}
}
}
