/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009-2011, IGG Team, LSIIT, University of Strasbourg           *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/
#ifndef _VDPMesh_App_
#define _VDPMesh_App_

#include "Utils/Qt/qtSimple.h"
#include "ui_VDPMesh_App.h"
#include "Utils/Qt/qtui.h"

#include "VDPMesh.h"

using namespace CGoGN;

class VDPMesh_App: public Utils::QT::SimpleQT
{
	Q_OBJECT
public:

	VDPMesh_App() {}

	~VDPMesh_App() {}

	void cb_redraw();

	void cb_initGL();

	//void cb_mousePress(int button, int x, int y);

	//void cb_mouseRelease(int button, int x, int y);

	//void cb_mouseClick(int button, int x, int y);

	//void cb_mouseMove(int buttons, int x, int y);

	//void cb_wheelEvent(int delta, int x, int y);

	//void cb_keyPress(int code);

	//void cb_keyRelease(int code);

public slots:
};

#endif
