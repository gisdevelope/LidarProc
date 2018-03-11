
#include "LASVectorClassify.h"

#include "LASPoint.h"
#include "LASReader.h"

OGREnvelope LASVectorClassify::LASRectToEnvelope(Rect2D rect)
{
	OGREnvelope envelope;
	envelope.MinX = rect.minx;
	envelope.MinY = rect.miny;
	envelope.MaxX = rect.maxx;
	envelope.MaxY = rect.maxy;
	return envelope;
}

Rect2D LASVectorClassify::LASEnvelopToRect(OGREnvelope *m_Envelope)
{
	Rect2D rect;
	rect.minx = m_Envelope->MinX;
	rect.miny = m_Envelope->MinY;
	rect.maxx = m_Envelope->MaxX;
	rect.maxy = m_Envelope->MaxY;
	
	return rect;
}

bool  LASVectorClassify::LASSetMaps(vector<string> nameKey, vector<eLASClassification>typeKey)
{
	if (nameKey.size() != typeKey.size())
		return false;

	for (int i = 0; i < nameKey.size(); ++i)
		name_type_maps.insert(pair<string, int>(nameKey[i], typeKey[i]));
}

void LASVectorClassify::VectorPointsToPolygon(Point2Ds points, OGRPolygon* poPolygon)
{
	OGRLinearRing ring;
	for(auto pnt:points){
		ring.addPoint(pnt.x,pnt.y);
	}
	ring.closeRings();
	poPolygon->addRing(&ring);
}

void LASVectorClassify::VectorPolygonToPoints(OGRPolygon *poPolygon, Point2Ds &points)
{
	OGRLinearRing* ring=poPolygon->getExteriorRing();
	for(int i=0;i<ring->getNumPoints();++i)
	{
		OGRPoint opt;
		ring->getPoint(i,&opt);
		Point2D vpt;
		vpt.x = opt.getX();
		vpt.y = opt.getY();
		points.push_back(vpt);
	}
}

bool LASVectorClassify::TestGeos() {
	OGRPolygon  poPolygon ;
	OGRLinearRing ring;
	ring.addPoint(0, 10);
	ring.addPoint(-10, -10);
	ring.addPoint(10, -10);
	ring.closeRings();

	poPolygon.addRing(&ring);
	OGRPoint *pnt = new OGRPoint(0, 0);
	bool test1 = ring.isPointInRing(pnt, 0);
	bool test2 = poPolygon.IsPointOnSurface(pnt);
	return (test1&&test2);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void LASShpClassify::LASClassifyByVector(const char* pathLas, vector<string> pShpPath, const char* pathOutLas)
{
	//???LAS
	//ILASDataset *datasetLas = new ILASDataset();
	FILE* fLasIn = nullptr, *fLasOut = nullptr;
	fLasIn = fopen(pathLas, "rb");
	fLasOut = fopen(pathOutLas, "wb");

	if (fLasIn == nullptr || fLasOut == nullptr)
		return;

	LASHeader    headerLas;
	LidarPatchReader patchReaderLine;
	patchReaderLine.LidarReader_ReadHeader(fLasIn, headerLas);
	patchReaderLine.LidarReader_WriteHeader(fLasOut, headerLas);

	const int readPatch = 100000;
	LASPoint* lasPnt = nullptr;
	try
	{
		lasPnt = new LASPoint[readPatch];
	}
	catch (bad_alloc e) {
		printf("%s\n", e.what());
		return;
	}
	
	GDALAllRegister();
	CPLSetConfigOption("SHAPE_ENCODING", "");

	int realReadPoints = 0;
	while (!feof(fLasIn)) {
		Rect2D rect=patchReaderLine.LidarReader_ReadPatch(fLasIn, headerLas, lasPnt, realReadPoints);
		for (size_t i = 0; i < pShpPath.size(); ++i)
		{
			GDALDataset *pShpData = (GDALDataset*)GDALOpenEx(pShpPath[i].c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);

			if (pShpData == NULL)
			{
				printf("SHP File Open Failed!\n");
				return;
			}
			for (size_t j = 0; j < pShpData->GetLayerCount(); ++j) {
				OGRLayer *pLayer = pShpData->GetLayer(j);
				pLayer->ResetReading();
				pLayer->SetSpatialFilterRect(rect.minx, rect.miny, rect.maxx, rect.maxy);
				int count = pLayer->GetFeatureCount();

				if (count <= 0)
					continue;

				OGRFeature *pFeature;
				while ((pFeature = pLayer->GetNextFeature()) != NULL)
				{
					OGRFeatureDefn *poFDefn = pLayer->GetLayerDefn();
					string typeName;
					int iField;
					for (iField = 0; iField < poFDefn->GetFieldCount(); iField++)
					{
						if (poFDefn->GetFieldDefn(iField)->GetNameRef() == "????????") {
							typeName=pFeature->GetFieldAsString(iField);
							break;
						}
					}

					OGRGeometry *poGeometry;
					poGeometry = pFeature->GetGeometryRef();
					OGRwkbGeometryType geoWkbType = poGeometry->getGeometryType();
					if (geoWkbType == wkbPolygon)
					{
						OGRPolygon *polygon = (OGRPolygon*)poGeometry;
						for (int k = 0; k < realReadPoints; ++k) {
							OGRPoint *pnt = new OGRPoint(lasPnt[k].m_vec3d.x, lasPnt[k].m_vec3d.y);
							if (polygon->IsPointOnSurface(pnt))
							{
								std::map<std::string, int>::iterator it = name_type_maps.find(typeName);
								if (it != name_type_maps.end())
									lasPnt[k].m_classify = it->second;
								else
									lasPnt[k].m_classify = elcUnclassified;
							}
						}
					}
					else if (geoWkbType == wkbMultiPolygon) {
						OGRMultiPolygon *polygonM = (OGRMultiPolygon*)poGeometry;
						for (int n = 0; n < polygonM->getNumGeometries(); n++)
						{
							OGRGeometry * FirstGeometry = polygonM->getGeometryRef(n);
							if (FirstGeometry == NULL)
							{
								continue;
							}
							OGRPolygon * poMultiPolygon = (OGRPolygon *)FirstGeometry;
							if (poMultiPolygon->getExteriorRing() == NULL)
							{
								continue;
							}
							OGRLinearRing * poMultiLinearRing = poMultiPolygon->getExteriorRing();

							for (int k = 0; k < realReadPoints; ++k) {
								OGRPoint *pnt = new OGRPoint(lasPnt[k].m_vec3d.x, lasPnt[k].m_vec3d.y);
								if (poMultiLinearRing->isPointInRing(pnt))
								{
									std::map<std::string, int>::iterator it = name_type_maps.find(typeName);
									if (it != name_type_maps.end())
										lasPnt[k].m_classify = it->second;
									else
										lasPnt[k].m_classify = elcUnclassified;
								}
							}
						}
					}
					else 
					{
						printf("not supportted polygon type\n");
					}
				}
			}
		}
		patchReaderLine.LidarReader_WritePatch(fLasOut, headerLas, lasPnt, realReadPoints);
	};//while

	delete[]lasPnt; lasPnt = nullptr;
	fclose(fLasIn); fLasIn = nullptr;
	fclose(fLasOut); fLasOut = nullptr;
}