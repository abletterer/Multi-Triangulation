#include "Node.h"

namespace CGoGN {

namespace Algo {

namespace Surface {

Node::Node(VSplit<PFP>* vsplit) 
    : m_vsplit(vsplit)
{
}

Node::~Node() 
{
    delete m_vsplit;
}

}//namespace Surface
}//namespace Algo
}//namespace CGoGN
