//
// Created by wuwei on 18-3-13.
//

#include "LASFormatTransform.h"
void LASFormatTransform3DTiles::LASFormatTransform_3DTilesJson(Point3D *center,std::string pathOut){
    string jsTemplate="{\n\
    \"asset\" : {\n  \
        \"version:\" \"1.0\"\n\
    },\n\
    \"geometricError\":10000,\n\
    \"refine\":\"add\",\n\
    \"root\":\n\
    { \n\
        \"boundingVolume\":{\n  \
        \"sphere\":[%lf,%lf,%lf,100]\n\
        },\n\
        \"geometricError\": 0,\n\
        \"content\":{\n  \
        \"url\":\"%s.pnts\n\"  \
         },\n\
         \"children\":[]\n\
   }\n\
}";
    int pos=pathOut.find_last_of('/');
    string s(pathOut.substr(pos+1));
    char buffer[2048];
    sprintf(buffer,jsTemplate.c_str(),center->x,center->y,center->z,s.c_str());
    string writeStr = pathOut+".json";

    FILE* fjson = fopen(writeStr.c_str(),"w+");
    fprintf(fjson,"%s",buffer);
    fclose(fjson);
}

void LASFormatTransform3DTiles::LASFormatTransform_3DTilesPnts(LASPoint *pnts, int pntNumber, std::string pathOut) {
    //process
    //write binnary data
    //gzip binnary data
    //remove binnary pnt;
    //rename gzip data as .pnt
}