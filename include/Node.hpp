#include "Node.h"

namespace CGoGN {

namespace Algo {

namespace Surface {

Node::Node(VSplit<PFP>* vsplit) 
    : m_vsplit(vsplit),
      m_parent(NULL),
      m_child_left(NULL),
      m_child_right(NULL),
      m_active(false)
{
}

Node::~Node() 
{
    delete m_vsplit;
}

}//namespace Surface
}//namespace Algo
}//namespace CGoGN
