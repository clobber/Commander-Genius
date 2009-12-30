/*
 * CEGAGraphics.h
 *
 *  Created on: 27.12.2009
 *      Author: gerstrong
 */

#ifndef CGALAXY_EGAGRAPHICS_H_
#define CGALAXY_EGAGRAPHICS_H_

#include <string>
#include <vector>
#include "../fileio/CExeFile.h"

#include "EGAStructs.h"
//
class CEGAGraphicsGalaxy
{
public:
	CEGAGraphicsGalaxy(short episode, const std::string& path, CExeFile &ExeFile);

	bool loadData();
	bool begin();
	bool exportBMP();

	virtual ~CEGAGraphicsGalaxy();

private:

	std::string m_path;
	short m_episode;
	std::vector<unsigned long> m_egahead;
	std::vector<ChunkStruct> m_egagraph;
	CExeFile &m_Exefile;
};

#endif /* CGALAXY_EGAGRAPHICS_H_ */
