/*
 * Carousel.h
 *
 *  Created on: Nov 3, 2009
 *      Author: Felippe Nagato
 */

#ifndef CAROUSEL_H_
#define CAROUSEL_H_

#include "util/functions.h"
#include "TSPacket.h"
#include "dsmcc/DSMCCSection.h"
#include "dsmcc/carousel/Module.h"
#include "dsmcc/carousel/ModuleManager.h"
#include "dsmcc/carousel/DownloadMessage/DownloadServerInitiate.h"
#include "dsmcc/carousel/DownloadMessage/DownloadInfoIndication.h"
#include "dsmcc/carousel/DownloadMessage/DownloadDataBlock.h"
#include "dsmcc/carousel/DownloadMessage/ModuleInfo.h"

#include <iostream>
#include <vector>
#include <map>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2::util;


namespace br {
namespace pucrio {
namespace telemidia {
namespace mpeg2 {
namespace dsmcc {

	class Carousel {
		private:
			unsigned char continuityCounter;

		protected:
			unsigned short pid;
			int bitrate;
			unsigned short blockSize;
			unsigned int serviceDomain;
			unsigned int transactionId;
			bool sectionEncapsulationMode;
			string serviceGatewayFolder;
			DownloadServerInitiate* dsi;
			DownloadInfoIndication* dii;

			ModuleManager* moduleManager;

			int makeDSI();
			int makeDII();
			int encapsulateCarousel(string outputFile, string tempFolder);
			int writeSections(int fd, vector<DSMCCSection*>* section);
			void clearSectionList(vector<DSMCCSection*>* sectionList);
			void checkMinBlockSize();

		public:
			Carousel();
			~Carousel();

			void setPid(unsigned short pid);
			unsigned short getPid();
			void setBitrate(int bitrate);
			int getBitrate();
			unsigned short getBlockSize();
			unsigned int getServiceDomain();
			unsigned int getDownloadId();
			unsigned int getTransactionId();
			string getServiceGatewayFolder();
			void setBlockSize(unsigned short size);
			void setServiceDomain(unsigned int sd);
			void setDownloadId(unsigned int id);
			void setTransactionId(unsigned int id);
			void setServiceGatewayFolder(string path);

			bool getSectionEncapsulationMode();
			void setSectionEncapsulationMode(bool active);

			int createCarousel(string outputFile, string tempFolder);
			map<string, InternalIor*>* getFilesIor();
	 };
}
}
}
}
}

#endif /* CAROUSEL_H_ */