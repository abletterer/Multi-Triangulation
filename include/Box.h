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
        Box(PFP::VEC3 pos_min = PFP::VEC3(-2., -2., -2.), PFP::VEC3 pos_max = PFP::VEC3(2., 2., 2.))
        	: m_pos_min(pos_min), m_pos_max(pos_max)
        {
        	m_drawer = new Utils::Drawer();
        }
        
        Box(Geom::BoundingBox<PFP::VEC3> bb)
        	: m_pos_min(bb.min()), m_pos_max(bb.max())
        {
        	m_drawer = new Utils::Drawer();
        }

        ~Box()
        {
        	delete m_drawer;
        }

        PFP::VEC3 getPosMin() { return m_pos_min; }
        void setPosMin(PFP::VEC3 pos_min) { m_pos_min = pos_min; }
        void incPosMin(float inc, unsigned int dir) { m_pos_min[dir] += inc; }
        void decPosMin(float inc, unsigned int dir) { m_pos_min[dir] -= inc; }
        
        PFP::VEC3 getPosMax() { return m_pos_max; }
        void setPosMax(PFP::VEC3 pos_max) { m_pos_max = pos_max; }
        void incPosMax(float inc, unsigned int dir) { m_pos_max[dir] += inc; }
        void decPosMax(float inc, unsigned int dir) { m_pos_max[dir] -= inc; }

        Utils::Drawer* getDrawer() { return m_drawer; }

        void updateDrawer() {
        	VEC3 a = m_pos_min;
        	VEC3 b = VEC3(m_pos_max[0], m_pos_min[1], m_pos_min[2]);
        	VEC3 c = VEC3(m_pos_max[0], m_pos_max[1], m_pos_min[2]);
        	VEC3 d = VEC3(m_pos_min[0], m_pos_max[1], m_pos_min[2]);
        	VEC3 e = VEC3(m_pos_min[0], m_pos_max[1], m_pos_max[2]);
        	VEC3 f = VEC3(m_pos_min[0], m_pos_min[1], m_pos_max[2]);
        	VEC3 g = VEC3(m_pos_max[0], m_pos_min[1], m_pos_max[2]);
        	VEC3 h = m_pos_max;

        	m_drawer->newList(GL_COMPILE);
        	m_drawer->begin(GL_LINES);
        		m_drawer->lineWidth(2.0f);
        		m_drawer->color3f(0.0f, 1.f, 0.0f);
        		m_drawer->vertex3f(a[0], a[1], a[2]); m_drawer->vertex3f(b[0], b[1], b[2]);
        		m_drawer->vertex3f(a[0], a[1], a[2]); m_drawer->vertex3f(d[0], d[1], d[2]);
        		m_drawer->vertex3f(a[0], a[1], a[2]); m_drawer->vertex3f(f[0], f[1], f[2]);

        		m_drawer->vertex3f(c[0], c[1], c[2]); m_drawer->vertex3f(d[0], d[1], d[2]);
        		m_drawer->vertex3f(c[0], c[1], c[2]); m_drawer->vertex3f(b[0], b[1], b[2]);
        		m_drawer->vertex3f(c[0], c[1], c[2]); m_drawer->vertex3f(h[0], h[1], h[2]);

        		m_drawer->vertex3f(g[0], g[1], g[2]); m_drawer->vertex3f(b[0], b[1], b[2]);
        		m_drawer->vertex3f(g[0], g[1], g[2]); m_drawer->vertex3f(f[0], f[1], f[2]);
        		m_drawer->vertex3f(g[0], g[1], g[2]); m_drawer->vertex3f(h[0], h[1], h[2]);

        		m_drawer->vertex3f(e[0], e[1], e[2]); m_drawer->vertex3f(d[0], d[1], d[2]);
        		m_drawer->vertex3f(e[0], e[1], e[2]); m_drawer->vertex3f(f[0], f[1], f[2]);
        		m_drawer->vertex3f(e[0], e[1], e[2]); m_drawer->vertex3f(h[0], h[1], h[2]);
        	m_drawer->end();
        	m_drawer->endList();
        }

        bool contains(PFP::VEC3 pos) {
        	return 	m_pos_min[0] <= pos[0] && pos[0] <= m_pos_max[0]
			 &&		m_pos_min[1] <= pos[1] && pos[1] <= m_pos_max[1]
			 &&		m_pos_min[2] <= pos[2] && pos[2] <= m_pos_max[2];
        }

        void print() {
        	CGoGNout << "Boite d'intéret : " << CGoGNendl;
        	CGoGNout << "  Min : X = " << m_pos_min[0] << " | Y = " << m_pos_min[1] << " | Z = " << m_pos_min[2] << CGoGNendl;
        	CGoGNout << "  Max : X = " << m_pos_max[0] << " | Y = " << m_pos_max[1] << " | Z = " << m_pos_max[2] << CGoGNendl;
        }
    private:
        PFP::VEC3 m_pos_min;    //Position [xmin;ymin;zmin]
        PFP::VEC3 m_pos_max;    //Position [xmax;ymax;zmax]
        Utils::Drawer* m_drawer;
};
} //namespace VDPMesh
} //namespace Surface
} //namespace Algo
} //namespace CGoGN

#endif
