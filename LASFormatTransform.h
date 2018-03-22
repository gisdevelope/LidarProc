//
// Created by wuwei on 18-3-13.
//

#ifndef LIDARPROC_LASFORMATTRANSFORM_H
#define LIDARPROC_LASFORMATTRANSFORM_H

#include <string>
#include "Geometry.h"
#include "LASPoint.h"

class LASFormatTransform3DTiles {
public:
    /**
     * 3D tiles pre json
     * @param center xyz coordinate
     * @param pathOut
     */
    void LASFormatTransform_3DTilesJson(Point3D *center,std::string pathOut);

    /**
     * 3D tiles points
     * @param pnts
     * @param pntNumber
     * @param pathOut
     */
    void LASFormatTransform_3DTilesPnts(LASPoint* pnts,int pntNumber,std::string pathOut);
};


#endif //LIDARPROC_LASFORMATTRANSFORM_H
