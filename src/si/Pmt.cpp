/******************************************************************************
Este arquivo eh parte da implementacao das ferramentas DTV do TeleMidia

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licenca Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribuido na expectativa de que seja util, porem, SEM
NENHUMA GARANTIA; nem mesmo a garantia implicita de COMERCIABILIDADE OU
ADEQUACAO A UMA FINALIDADE ESPECIFICA. Consulte a Licenca Publica Geral do
GNU versao 2 para mais detalhes.

Voce deve ter recebido uma copia da Licenca Publica Geral do GNU versao 2 junto
com este programa; se nao, escreva para a Free Software Foundation, Inc., no
endereco 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.

Para maiores informacoes:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
******************************************************************************
This file is part of the TeleMidia DTV Tools

Copyright: 1989-2007 PUC-RIO/LABORATORIO TELEMIDIA, All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License version 2 for more
details.

You should have received a copy of the GNU General Public License version 2
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

For further information contact:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
*******************************************************************************/

#include "si/Pmt.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace mpeg2 {

string Pmt::getStreamTypeName(unsigned char streamType) {
	static map<short, string> streamTypeNames;

	(streamTypeNames)[0] = "ISO/IEC Reserved";
	(streamTypeNames)[1] = "ISO/IEC 11172-2 Video";
	(streamTypeNames)[2] = "ISO/IEC 13818-2 Video";
	(streamTypeNames)[3] = "ISO/IEC 11172-3 Audio";
	(streamTypeNames)[4] = "ISO/IEC 13818-3 Audio";
	(streamTypeNames)[5] = "ISO/IEC 13818 Private Sections";
	(streamTypeNames)[6] =	"ISO/IEC 13818 Private Data";
	(streamTypeNames)[7] = "ISO/IEC 13522 MHEG";
	(streamTypeNames)[8] = "ISO/IEC 13818-1 Annex A DSM-CC";
	(streamTypeNames)[9] = "ITU-T Rec. H.222.1";
	(streamTypeNames)[10] = "ISO/IEC 13818-6 type A";
	(streamTypeNames)[11] = "ISO/IEC 13818-6 type B";
	(streamTypeNames)[12] = "ISO/IEC 13818-6 type C";
	(streamTypeNames)[13] = "ISO/IEC 13818-6 type D";
	(streamTypeNames)[14] = "ISO/IEC 13818-1 auxiliary";
	(streamTypeNames)[15] =	"ISO/IEC 13818-7 Audio (ADTS transport syntax)";
	(streamTypeNames)[16] = "ISO/IEC 14496-2 Visual";
	(streamTypeNames)[17] = "ISO/IEC 14496-2 Audio (LATM transport syntax)";
	(streamTypeNames)[18] =	"SL-packetized or FlexMux stream (PES packets)";
	(streamTypeNames)[19] =	"SL-packetized or FlexMux stream (Sections)";
	(streamTypeNames)[20] = "ISO/IEC 13818-6 Synchronized Download Protocol";
	(streamTypeNames)[21] = "Metadata carried in PES packets";
	(streamTypeNames)[22] = "Metadata carried in metadata_sections";
	(streamTypeNames)[23] =	"Metadata (ISO/IEC 13818-6 Data Carousel)";
	(streamTypeNames)[24] =	"Metadata (ISO/IEC 13818-6 Object Carousel)";
	(streamTypeNames)[25] =	"Metadata (ISO/IEC 13818-6 SDP)";
	(streamTypeNames)[26] = "IPMP stream";
	(streamTypeNames)[27] =	"AVC video stream";
	(streamTypeNames)[127] = "IPMP stream";
	(streamTypeNames)[129] = "audio AC3";
	(streamTypeNames)[138] = "audio DTS";

	if (streamTypeNames.count(streamType) == 0) {
		return "Unknown";
	}
	return (streamTypeNames)[streamType];
}

Pmt::Pmt() : PrivateSection() {
	programInfoList = new map<char*, unsigned short>;
	esList = new map<unsigned short, unsigned char>;
	esDescriptorList = new map<unsigned short, map<char*, unsigned short>*>;
	pcrPid = 0;
	programInfoLength = 0;
	tableId = 0x02;
}

Pmt::~Pmt() {
	if (programInfoList != NULL) {
		delete (programInfoList);
	}
	if (esList != NULL) {
		delete (esList);
	}
	if (esDescriptorList != NULL) {
		clearEsDescriptorList();
		delete (esDescriptorList);
	}
}

int Pmt::processSectionPayload() {
	int pos = PrivateSection::processSectionPayload();
	int sectionSize = privateSectionLength + 3;
	int endOfPi;
	unsigned char descriptorSize;
	unsigned char streamType;
	unsigned short esPid, esInfoLength;

	programInfoList->clear();
	esList->clear();
	clearEsDescriptorList();
	esDescriptorList->clear();

	pcrPid = (((stream[pos] & 0x1F) << 8) | (stream[pos+1] & 0xFF));
	programInfoLength =
		(((stream[pos+2] & 0x0F) << 8) | (stream[pos+3] & 0xFF));

	pos = pos + 4;

	endOfPi = pos + programInfoLength;

	while (pos < endOfPi) {
		descriptorSize = (stream[pos + 1] & 0xFF) + 2;
		addInfo(stream + pos, descriptorSize);
		pos = pos + descriptorSize;
	}

	while ((pos + 4) < sectionSize) {
		streamType = stream[pos] & 0xFF;
		esPid = (((stream[pos+1] & 0x1F) << 8) | (stream[pos+2] & 0xFF));
		esInfoLength = (((stream[pos+3] & 0x0F) << 8) | (stream[pos+4] & 0xFF));
		addEs(streamType, esPid);
		pos = pos + 5;

		endOfPi = pos + esInfoLength;
		while (pos < endOfPi) {
			descriptorSize = (stream[pos + 1] & 0xFF) + 2;
			addEsDescriptor(esPid, stream + pos, descriptorSize);
			pos = pos + descriptorSize;
		}
	}

	if (sectionSyntaxIndicator) {
		crc32 = ((stream[pos] & 0xFF) << 24) |
					((stream[pos + 1] & 0xFF) << 16) |
					((stream[pos + 2] & 0xFF) << 8) |
					(stream[pos + 3] & 0xFF);
		pos += 4;
	}

	return pos;
}

int Pmt::updateStream() {
	map<char*, unsigned short>::iterator i;
	map<unsigned short, unsigned char>::iterator j;
	map<unsigned short, map<char*, unsigned short>*>::iterator k;
	map<char*, unsigned short>::iterator h;
	map<char*, unsigned short>* descList = NULL;
	unsigned short value, esInfoLength;
	int pos = PrivateSection::updateStream();
	int posEsInfoLength;

	stream[pos] = 0xE0;
	stream[pos] = stream[pos] | ((pcrPid >> 8) & 0xFF);
	pos++;
	stream[pos++] = pcrPid & 0xFF;

	stream[pos] = 0xF0;
	stream[pos] = stream[pos] | ((programInfoLength >> 8) & 0x0F);
	pos++;
	stream[pos++] = programInfoLength & 0xFF;

	i = programInfoList->begin();
	while (i != programInfoList->end()) {
		if ((i->second > 0) && (i->first != NULL)) {
			memcpy(stream + pos, i->first, i->second);
			pos = pos + i->second;
		}
		++i;
	}

	j = esList->begin();
	while (j != esList->end()) {
		value = j->second;
		stream[pos++] = value & 0xFF;
		value = j->first;
		stream[pos] = 0xE0;
		stream[pos] = stream[pos] | ((value >> 8) & 0x1F);
		pos++;
		stream[pos++] = value & 0xFF;

		posEsInfoLength = pos;
		pos = pos + 2;

		esInfoLength = 0;
		k = esDescriptorList->find(j->first);
		if (k != esDescriptorList->end()) {
			descList = k->second;
			if (descList != NULL) {
				h = descList->begin();
				while (h != descList->end()) {
					if ((h->second > 0) && (h->first != NULL)) {
						memcpy(stream + pos, h->first, h->second);
						pos = pos + h->second;
						esInfoLength = esInfoLength + h->second;
					}
					++h;
				}
			}
		}
		stream[posEsInfoLength] = 0xF0;
		stream[posEsInfoLength] =
				stream[posEsInfoLength] | ((esInfoLength >> 8) & 0x0F);
		posEsInfoLength++;
		stream[posEsInfoLength] = esInfoLength & 0xFF;
		++j;
	}

	if (sectionSyntaxIndicator) {
		Crc32 crc;
		unsigned int value = crc.crc(stream, pos);
		crc32 = value;
		stream[pos++] = (crc32 >> 24) & 0xFF;
		stream[pos++] = (crc32 >> 16) & 0xFF;
		stream[pos++] = (crc32 >> 8) & 0xFF;
		stream[pos++] = crc32 & 0xFF;
	}
	return pos;
}

int Pmt::calculateSectionSize() {
	map<char*, unsigned short>::iterator i;
	map<unsigned short, unsigned char>::iterator j;
	map<unsigned short, map<char*, unsigned short>*>::iterator k;
	map<char*, unsigned short>::iterator h;
	map<char*, unsigned short>* descList = NULL;
	unsigned short esInfoLength;
	unsigned int pos = PrivateSection::calculateSectionSize();

	programInfoLength = 0;
	i = programInfoList->begin();
	while (i != programInfoList->end()) {
		programInfoLength = programInfoLength + i->second;
		++i;
	}

	pos = pos + programInfoLength + 4;
	pos = pos + (esList->size() * 5);

	j = esList->begin();
	while (j != esList->end()) {
		esInfoLength = 0;
		k = esDescriptorList->find(j->first);
		if (k != esDescriptorList->end()) {
			descList = k->second;
			if (descList != NULL) {
				h = descList->begin();
				while (h != descList->end()) {
					if ((h->second > 0) && (h->first != NULL)) {
						esInfoLength = esInfoLength + h->second;
					}
					++h;
				}
			}
		}
		pos = pos + esInfoLength;
		++j;
	}

	return pos + 4;
}

unsigned short Pmt::getProgramNumber() {
	return tableIdExtension;
}

void Pmt::setProgramNumber(unsigned short pnum) {
	tableIdExtension = pnum;
}

unsigned short Pmt::getPCRPid() {
	return pcrPid;
}

void Pmt::setPCRPid(unsigned short pid) {
	pcrPid = pid;
}

map<unsigned short, unsigned char>* Pmt::getEsList() {
	return esList;
}

void Pmt::addEs(unsigned char streamType, unsigned short pid) {
	(*esList)[pid] = streamType;
	(*esDescriptorList)[pid] = NULL;
}

void Pmt::removeEs(unsigned short pid) {
	map<unsigned short, unsigned char>::iterator i;
	map<unsigned short, map<char*, unsigned short>*>::iterator j;

	if ((esList != NULL) && (!esList->empty())) {
		i = esList->find(pid);
		if (i != esList->end()) {
			esList->erase(i);
			removeEsDescriptorList(pid);
			j = esDescriptorList->find(pid);
			if (j != esDescriptorList->end()) {
				esDescriptorList->erase(j);
			}
		}
	}
}

map<char*, unsigned short>* Pmt::getEsDescriptorList(unsigned short pid) {
	map<unsigned short, map<char*, unsigned short>*>::iterator i;

	if ((esDescriptorList != NULL) && (!esDescriptorList->empty())) {
		i = esDescriptorList->find(pid);
		if (i != esDescriptorList->end()) {
			return i->second;
		}
	}
	return NULL;
}

int Pmt::addEsDescriptor(
		unsigned short pid, char* data, unsigned char length) {

	map<unsigned short, map<char*, unsigned short>*>::iterator i;
	map<char*, unsigned short>* descList = NULL;
	char* tempData;

	if ((esDescriptorList != NULL) && (!esDescriptorList->empty())) {
		i = esDescriptorList->find(pid);
		if (i != esDescriptorList->end()) {
			descList = i->second;
			if (descList == NULL) {
				descList = new map<char*, unsigned short>;
				i->second = descList;
			}
			tempData = new char[length];
			memcpy(tempData, data, length);
			(*descList)[tempData] = length;
			return pid;
		}
	}
	return -1;
}

void Pmt::removeEsDescriptorList(unsigned short pid) {
	map<unsigned short, map<char*, unsigned short>*>::iterator i;
	map<char*, unsigned short>* descList = NULL;
	map<char*, unsigned short>::iterator j;

	if ((esDescriptorList != NULL) && (!esDescriptorList->empty())) {
		i = esDescriptorList->find(pid);
		if (i != esDescriptorList->end()) {
			descList = i->second;
			if (descList != NULL) {
				i->second = NULL;
				j = descList->begin();
				while (j != descList->end()) {
					delete (j->first);
					++j;
				}
				delete descList;
			}
		}
	}
}

map<char*, unsigned short>* Pmt::getProgramInfoList() {
	return programInfoList;
}

void Pmt::addInfo(char* data, unsigned short length) {
	char* tempData;

	tempData = new char[length];
	memcpy(tempData, data, length);
	(*programInfoList)[tempData] = length;
}

void Pmt::removeInfoList() {
	map<char*, unsigned short>::iterator i;
	i = programInfoList->begin();
	while (i != programInfoList->end()) {
		delete (i->first);
		++i;
	}
	programInfoList->clear();
}

void Pmt::clearEsDescriptorList() {
	map<unsigned short, map<char*, unsigned short>*>::iterator i;
	i = esDescriptorList->begin();
	while (i != esDescriptorList->end()) {
		removeEsDescriptorList(i->first);
		++i;
	}
	esDescriptorList->clear();
}

}
}
}
}