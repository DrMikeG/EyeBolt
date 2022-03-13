////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                          Copyright 2007 UGS Corp.                          //
//                            All rights reserved.                            //
//    This software and related documentation are proprietary to UGS Corp.    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "VTKUNIT/Precomp.hxx"
#include "TEST/All/TestUtil.hxx"
#include "TEST/Misc/TestPKUtils.hxx"
#include "Utils/Misc/PKUtils.hxx"
#include "Utils/Misc/DebugUtils.hxx"
#include "Utils/Misc/FeatureProperties.hxx"
#include "VTKUNIT/Framework/Core/Interaction.hxx"
#include "Utils/Misc/VTKBody.hxx"
#include "VTK/Api/VTKApi.hxx"
#include "VTKUNIT/Framework/Core/CompareObjects.hxx"
#include "VTK/Utils/APIUtils/VersionedPK.hxx"
#include "Utils/Misc/ParasolidJournalUtils.hxx"
#include <math.h>
#include "Utils/Maths/TolerantCompare.hxx"
#include "Utils/Maths/Xform.hxx"
#include "Utils/Maths/Dir.hxx"
#include "Utils/Maths/Constant.hxx"
#include "Utils/Maths/BcurveDef.hxx"
#include "VTKUNIT/Framework/Objects/Profile.hxx"
#include "VTKUNIT/Framework/Objects/Instance.hxx"
#include "Utils/Misc/ArrayUtils.hxx"
#include "Utils/Misc/PVMIdentifyBlendsOptions.hxx"
#include "Utils/Misc/PVMUtils.hxx"
#include "VTKUNIT/Framework/Objects/Reference.hxx"
#include "Utils/Maths/EllipseDef.hxx"
#include "VTKUNIT/Framework/Core/TagManager.hxx"

#include <time.h>

#include "VTKUNIT/Modelling/Parasolid/OffsetCurveApproximator.hxx"

//#include "VTKUNIT/Framework/Core/Interaction.hxx"
//#include "VTKUNIT/Framework/Core/TagManager.hxx"
//#include <algorithm>
//#include <map>
//#include <vector>

using namespace VTK;
using namespace VTKUNITNAME;

VTK_TEST_FN(TestPKUtils001)
{
    //test PKUtils().FacesGetCommonEdges

    ResultType result = ResultTypeOk;
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/New Stand_v20.x_t");
    CheckVTKBodyM(vbody);

    PK_FACE_t face1075Tag = FaceOfId(vbody, 1075);
    CheckM(0 != face1075Tag);
    PK_FACE_t face95Tag = FaceOfId(vbody, 95);
    CheckM(0 != face95Tag);

    PK_EDGE_t expectedEdge = EdgeOfId(vbody, 1039);
    CheckM(0 != expectedEdge);

    std::vector<PK_EDGE_t> commonEdges;
    int numCommonEdges = PKUtils().FacesGetCommonEdges(face1075Tag, face95Tag, commonEdges);
    CheckM(1 == numCommonEdges);
    CheckM(expectedEdge == commonEdges[0]);

    return true;
}


VTK_TEST_FN(TestPKUtils002)
{
    //test PKUtils().FaceGetNeighbourFaces
    ResultType result = ResultTypeOk;
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/New Stand_v20.x_t");
    CheckVTKBodyM(vbody);

    PK_FACE_t face2870Tag = FaceOfId(vbody, 2870);
    CheckM(0 != face2870Tag);

    PK_FACE_t face2872Tag = FaceOfId(vbody, 2872);
    CheckM(0 != face2872Tag);
    PK_FACE_t face1079Tag = FaceOfId(vbody, 1079);
    CheckM(0 != face1079Tag);
    PK_FACE_t face2868Tag = FaceOfId(vbody, 2868);
    CheckM(0 != face2868Tag);

    std::vector<PK_FACE_t> expectedResults;
    expectedResults.push_back(face2872Tag);
    expectedResults.push_back(face1079Tag);
    expectedResults.push_back(face2868Tag);
    std::sort(expectedResults.begin(), expectedResults.end(), std::less<int>());

    std::set<PK_FACE_t> results;
    size_t numNeighbours = PKUtils().FaceGetNeighbourFaces(face2870Tag, results);
    CheckM(3 == numNeighbours);
    std::vector<PK_FACE_t> resultsVec;
    std::set<PK_FACE_t>::iterator resultsIt = results.begin();
    while(resultsIt != results.end())
    {
        resultsVec.push_back(*resultsIt);
        ++resultsIt;
    }
    std::sort(resultsVec.begin(), resultsVec.end(), std::less<int>());

    int ctr = 0;
    for(ctr = 0; ctr < 3; ++ctr)
    {
        CheckM(resultsVec[ctr] == expectedResults[ctr]);
    }

    return true;
}

VTK_TEST_FN(TestPKUtils003_blending)
{
    // This code uses PKUtils().BlendEdges to put a 4mm blend (quite large) on the rims of the central hole in the stand model.
    // This shows that the temporary memory is freed properly by the util code.

    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/New Stand_v20.x_t");
    CheckVTKBodyM(vbody);

    IntSet edges;
    edges.insert(EdgeOfId(vbody,915));
    edges.insert(EdgeOfId(vbody,913));

    bool ok = PKUtils().BlendEdges(TestGetCurrentVersion(),edges,0.004);
    CheckM(ok);

    bool different = TestUtilAreBodiesDifferent(TestGetCurrentVersion(), vbody,"vtk_part_data:Misc/New Stand_v20_blended_hole.x_t");
    CheckM(!different);

    return true;
}

//YZ - 08/10/08: New tests added.
VTK_TEST_FN(TestPKUtils004_EntityGetBody)
{
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/New Stand_v20.x_t");
    CheckVTKBodyM(vbody);

    PK_BODY_t bodyFromEntity;

    //Check body from face
    PK_FACE_t faceTag = FaceOfId(vbody, 2870);
    CheckM(0 != faceTag);
    bodyFromEntity = VTKBody::EntityGetConstructionBody(faceTag).GetPKBodyTag();
    CheckM(vbody.GetPKBodyTag() == bodyFromEntity);

    //Check body from edge
    PK_EDGE_t edgeTag = EdgeOfId(vbody, 836);
    CheckM(0 != edgeTag);
    bodyFromEntity = VTKBody::EntityGetConstructionBody(edgeTag).GetPKBodyTag();
    CheckM(vbody.GetPKBodyTag() == bodyFromEntity);

    //Check body from vertex
    PK_VERTEX_t vertTag = VertexOfId(vbody, 2678);
    CheckM(0 != vertTag);
    bodyFromEntity = VTKBody::EntityGetConstructionBody(vertTag).GetPKBodyTag();
    CheckM(vbody.GetPKBodyTag() == bodyFromEntity);

    //Test an invalid entity tag.
    bodyFromEntity = VTKBody::EntityGetConstructionBody(0).GetPKBodyTag();
    CheckM(PK_ENTITY_null == bodyFromEntity);

    return true;
}

VTK_TEST_FN(TestPKUtils005_TopolGetGeomClass)
{
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/New Stand_v20.x_t");
    CheckVTKBodyM(vbody);

    PK_EDGE_t edgeTag;
    PK_FACE_t faceTag;
    EntityClass cls;

    //Test plane
    faceTag = FaceOfId(vbody, 101);
    CheckM(0 != faceTag);
    cls = PKUtils().TopolGetGeomClass(faceTag);
    CheckM(EntityClass::Plane == cls);

    //Test cylinder
    faceTag = FaceOfId(vbody, 340);
    CheckM(0 != faceTag);
    cls = PKUtils().TopolGetGeomClass(faceTag);
    CheckM(EntityClass::Cyl == cls);

    //Test sphere
    faceTag = FaceOfId(vbody, 2385);
    CheckM(0 != faceTag);
    cls = PKUtils().TopolGetGeomClass(faceTag);
    CheckM(EntityClass::Sphere == cls);

    //Test torus
    faceTag = FaceOfId(vbody, 1077);
    CheckM(0 != faceTag);
    cls = PKUtils().TopolGetGeomClass(faceTag);
    CheckM(EntityClass::Torus == cls);

    //Test blendsf?
    faceTag = FaceOfId(vbody, 2371);
    CheckM(0 != faceTag);
    cls = PKUtils().TopolGetGeomClass(faceTag);
    CheckM(EntityClass::Blendsf == cls);

    //Test circle
    edgeTag = EdgeOfId(vbody, 836);
    CheckM(0 != edgeTag);
    cls = PKUtils().TopolGetGeomClass(edgeTag);
    CheckM(EntityClass::Circle == cls);

    //Test ellipse
    edgeTag = EdgeOfId(vbody, 2127);
    CheckM(0 != edgeTag);
    cls = PKUtils().TopolGetGeomClass(edgeTag);
    CheckM(EntityClass::Ellipse == cls);

    //Test line
    edgeTag = EdgeOfId(vbody, 2825);
    CheckM(0 != edgeTag);
    cls = PKUtils().TopolGetGeomClass(edgeTag);
    CheckM(EntityClass::Line == cls);

    return true;
}

VTK_TEST_FN(TestPKUtils006)
{
    //Test for PKUtils().IsOpenSheet when there are multiple components to the body
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Feature/multiple_sheet_body_1.x_t");
    CheckM( vbody.IsOpenSheet(TestGetCurrentVersion()) );

    return true;
}

VTK_TEST_FN(TestPKUtils007_GeomCompare)
{
    IntIntPair geomPair;

    // cylinder compare
    PK_CYL_sf_t pkCylSF = {0};
    pkCylSF.basis_set.axis.coord[0] = 1; 
    pkCylSF.basis_set.axis.coord[1] = 0;
    pkCylSF.basis_set.axis.coord[2] = 0;
    pkCylSF.basis_set.location.coord[0] = 0;
    pkCylSF.basis_set.location.coord[1] = 0; 
    pkCylSF.basis_set.location.coord[2] = 0;
    pkCylSF.basis_set.ref_direction.coord[0] = 0; 
    pkCylSF.basis_set.ref_direction.coord[1] = 0; 
    pkCylSF.basis_set.ref_direction.coord[2] = -1;
    pkCylSF.radius = 0.1;

    PK_CYL_create( &pkCylSF, &geomPair.first );
    CheckTagM(geomPair.first);

    // should compare to self
    CheckM(PKUtils().DebugGeomCompare(geomPair.first, geomPair.first));

    pkCylSF.radius = .05;  // change radius
    geomPair.second = 0;
    CheckPKOkM(PK_CYL_create( &pkCylSF, &geomPair.second ));
    CheckTagM(geomPair.second);
    CheckM(false == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second));

    pkCylSF.radius = .1;  // restore radius
    pkCylSF.basis_set.location.coord[0] = 0.1;  // change location
    CheckPKOkM(PK_CYL_create( &pkCylSF, &geomPair.second ));
    CheckTagM(geomPair.second);
    CheckM(false == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second));
    
    pkCylSF.basis_set.location.coord[0] = 0;  // restore location
    pkCylSF.basis_set.axis.coord[0] = -1;  // change axis

    CheckPKOkM(PK_CYL_create( &pkCylSF, &geomPair.second ));
    CheckTagM(geomPair.second);
    CheckM(false == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second));

    PK_LINE_sf_t lineSf;
    lineSf.basis_set.location.coord[0] = 0.0;
    lineSf.basis_set.location.coord[1] = 0.0;
    lineSf.basis_set.location.coord[2] = 0.0;
    lineSf.basis_set.axis.coord[0] = 1.0;
    lineSf.basis_set.axis.coord[1] = 0.0;
    lineSf.basis_set.axis.coord[2] = 0.0;

    CheckPKOkM(PK_LINE_create(&lineSf, &geomPair.second));
    CheckTagM(geomPair.second);
    CheckM(PKUtils().DebugGeomCompare(geomPair.second, geomPair.second)); // compare self

    // cyl != line
    CheckM(false == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second));

    // change position
    lineSf.basis_set.location.coord[2] = 0.01;
    geomPair.first = 0;
    CheckPKOkM(PK_LINE_create(&lineSf, &geomPair.first));
    CheckTagM(geomPair.first);

    CheckM(false == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second));

    return true;
}

VTK_TEST_FN(TestPKUtils008_GeomIsCoincident)
{
    PK_PLANE_t plane1 = 0;
    PK_PLANE_t plane2 = 0;

    PK_PLANE_sf_t planeSf = {0} ;
    PK_PLANE_sf_t planeSf2 = {0} ;

    planeSf.basis_set.location.coord[0] = 0.0;
    planeSf.basis_set.location.coord[1] = 0.05;
    planeSf.basis_set.location.coord[2] = 0;
    planeSf.basis_set.axis.coord[0] = 0.0;
    planeSf.basis_set.axis.coord[1] = 1.0;
    planeSf.basis_set.axis.coord[2] = 0.0;
    planeSf.basis_set.ref_direction.coord[0] = 1.0;
    planeSf.basis_set.ref_direction.coord[1] = 0.0;
    planeSf.basis_set.ref_direction.coord[2] = 0.0;

    PK_PLANE_t plane = PK_ENTITY_null;
    PK_ERROR_code_t error = PK_PLANE_create(&planeSf, &plane1);
    CheckPKOkM(error);

    // verify that direction and axis does not affect coi test
    planeSf.basis_set.axis.coord[1] = -1.0;
    planeSf.basis_set.ref_direction.coord[0] = -1.0;

    error = PK_PLANE_create(&planeSf, &plane2);
    CheckPKOkM(error);

    CheckM( PKUtils().GeomsAreCoincident(plane1, plane2) );

    // plane data from SE issue
    planeSf.basis_set.location.coord[0] = 0.075828207671257100;
    planeSf.basis_set.location.coord[1] = 0.036584216985975054;
    planeSf.basis_set.location.coord[2] = -0.00033796580938760947;
    planeSf.basis_set.axis.coord[0] = -1.3579632210380921e-014;
    planeSf.basis_set.axis.coord[1] = -5.9157789185826907e-016;
    planeSf.basis_set.axis.coord[2] = 1.0000000000000000;
    planeSf.basis_set.ref_direction.coord[0] = 2.4077645034969498e-016;
    planeSf.basis_set.ref_direction.coord[1] = -1.0000000000000000;
    planeSf.basis_set.ref_direction.coord[2] = -5.9157789185826602e-016;

    error = PK_PLANE_create(&planeSf, &plane1);
    CheckPKOkM(error);

    planeSf2.basis_set.location.coord[0] = 0.073328207671257098;
    planeSf2.basis_set.location.coord[1] = 0.036584216985975054;
    planeSf2.basis_set.location.coord[2] = -0.00033796580938767697;
    planeSf2.basis_set.axis.coord[0] = 3.4441693984514552e-026;
    planeSf2.basis_set.axis.coord[1] = -1.4304428001365039e-010;
    planeSf2.basis_set.axis.coord[2] = -1.0000000000000000;
    planeSf2.basis_set.ref_direction.coord[0] = -5.4055402753414097e-009;
    planeSf2.basis_set.ref_direction.coord[1] = 1.0000000000000000;
    planeSf2.basis_set.ref_direction.coord[2] = -1.4304428001365039e-010;

    error = PK_PLANE_create(&planeSf2, &plane2);
    CheckPKOkM(error);

    // currently failing coi test
    CheckM( PKUtils().GeomsAreCoincident(plane1, plane2) );

    return true;
}

VTK_TEST_FN(TestPKUtils009)
{
    //Test for PKUtils().IdentifyBlends and PKUtils().IsBlendFace on a model containing
    //mutually dependent blends belonging to an RPF
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Feature/RPFAndNailBody.xml").c_str(),result);
    Instance* instance = interaction->GetInstance(0);
    VTKBody vbody = instance->TestGetFirstBody();
    IntArray rpfs;
    vbody.FindFeaturesSingleBodyDeprecated(0,rpfs);

    Version version = interaction->GetVersion();
    double tolerance = interaction->GetModellingTolerance();
    int attdef = interaction->GetFeaturePropertiesAttDefTag();
    char* integerFieldName = FeatureProperties::GetIntegerFieldName();
    int sheetMetalIntIndex = FeatureProperties::GetThicknessDependentIntIndex();

    int layerFace = FaceOfId(vbody,26);
    IntArray layer;
    layer.push_back(layerFace);

    int nonBoundaryBlendFace = FaceOfId(vbody,2049);
    IntArray nonBoundaryBlend;
    nonBoundaryBlend.push_back(nonBoundaryBlendFace);

    int boundaryBlendFace = FaceOfId(vbody,1752);
    IntArray boundaryBlend;
    boundaryBlend.push_back(boundaryBlendFace);

    //RPF does not have the thickness dependent property set so the blends should not
    //be recognized unless we are allowing mutually dependent blends
    
    PVMIdentifyBlendsOptions options(interaction->GetVersion(),tolerance,true,true,true,false,PVM_blend_convexity_any_c,attdef,integerFieldName,sheetMetalIntIndex);

    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(!PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(!PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    options.SetIncludeMutuallyDependentBlends(                          true );
    CheckM(PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    


    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(!PVMUtils::FaceIsBlend(options,boundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(!PVMUtils::FaceIsBlend(options,boundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    options.SetIncludeMutuallyDependentBlends(                          true );   
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));

    IntArray blends;
    
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    options.SetIncludeMutuallyDependentBlends(                          false );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.empty());

    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    options.SetIncludeMutuallyDependentBlends(                          true );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();
    
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    options.SetIncludeMutuallyDependentBlends(                          false );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.empty());

    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    options.SetIncludeMutuallyDependentBlends(                          true );    
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();

    //Set the RPF as thickness dependent
    FeatureProperties props(interaction->GetFeaturePropertiesAttDefTag(), interaction->GetVersion(), rpfs[0]);
    props.SetThicknessDependent(true);

    //The boundary blend should now be recognized if the mutually dependent override is on
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(!PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(!PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(!PVMUtils::FaceIsBlend(options,boundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));


    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.empty());

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();

    //Unset the thickness dependent property - the blends should not be recognized unless we are allowing 
    //mutually dependent blends
    props.SetThicknessDependent(false);

    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(!PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(!PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    CheckM(PVMUtils::FaceIsBlend(options,nonBoundaryBlendFace));
    
    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(!PVMUtils::FaceIsBlend(options,boundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );    
    CheckM(!PVMUtils::FaceIsBlend(options,boundaryBlendFace));

    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );    
    CheckM(PVMUtils::FaceIsBlend(options,boundaryBlendFace));

    


    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.empty());
    
    options.SetIncludeMutuallyDependentBlends(                          true );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   false );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();

    options.SetIncludeMutuallyDependentBlends(                          false );
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.empty());
    
    options.SetIncludeMutuallyDependentBlendsIfBoundarySheetMetalRPF(   true );
    options.SetIncludeMutuallyDependentBlends(                          true );
    PVMUtils::FaceIdentifyBlends(options,layer,PVM_blend_identify_dependent_c,NULL,&blends);
    CheckM(blends.size() == 8);
    blends.clear();

    interaction->Destroy();

    return true;
}

VTK_TEST_FN(TestPKUtils010)
{
    // Test DebugGeomCompare with strict flag set to false (does coi test rather than
    //  exact test)

    IntIntPair geomPair;

    // cylinder compare
    PK_CYL_sf_t pkCylSF = {0};
    pkCylSF.basis_set.axis.coord[0] = 1; 
    pkCylSF.basis_set.axis.coord[1] = 0;
    pkCylSF.basis_set.axis.coord[2] = 0;
    pkCylSF.basis_set.location.coord[0] = 0;
    pkCylSF.basis_set.location.coord[1] = 0; 
    pkCylSF.basis_set.location.coord[2] = 0;
    pkCylSF.basis_set.ref_direction.coord[0] = 0; 
    pkCylSF.basis_set.ref_direction.coord[1] = 0; 
    pkCylSF.basis_set.ref_direction.coord[2] = -1;
    pkCylSF.radius = 0.1;

    PK_CYL_create( &pkCylSF, &geomPair.first );
    CheckTagM(geomPair.first);

    // change location but still on axis
    pkCylSF.basis_set.location.coord[0] = 1;
    geomPair.second = 0;
    CheckPKOkM(PK_CYL_create( &pkCylSF, &geomPair.second ));
    CheckTagM(geomPair.second);
    CheckM(false == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second));
    CheckM(true == PKUtils().DebugGeomCompare(geomPair.first, geomPair.second, false));

    return true;
}

VTK_TEST_FN(TestPKUtils011)
{
    //Test for PKUtils().FaceMakeSheetBody and loops
    VTKBody vtkBody = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube2ConeBosses.x_t");
    
    //Single face sheet
    VTKBody singleFaceSheetVBody = VTKBody::MakeSheetBody(FaceOfId(vtkBody,33));
    CheckM(singleFaceSheetVBody.IsNotNull() && singleFaceSheetVBody != vtkBody);

    //Sheet face should have 2 inner loops and an outer loop
    IntArray sheetFace;    
    singleFaceSheetVBody.GetFaces(TestGetCurrentVersion(),&sheetFace);
    CheckM(sheetFace.size() == 1);
    IntArray loops;
    CheckM(PKUtils().FaceGetLoops(sheetFace[0]) == 3);
    CheckM(PKUtils().FaceGetLoops(sheetFace[0],&loops) == 3 && loops.size() == 3);
    size_t i;
    IntSet inner;
    IntSet outer;
    for ( i = 0; i < 3; ++i )
    {
        PK_LOOP_type_t type = PKUtils().LoopGetType(loops[i]);
        if ( type == PK_LOOP_type_inner_c )
        {
            //Should be one edge
            IntArray edges;
            CheckM(PKUtils().LoopGetEdges(loops[i]) == 1);
            CheckM(PKUtils().LoopGetEdges(loops[i],&edges) == 1 && edges.size() == 1);
            inner.insert(loops[i]);
        }
        else if ( type == PK_LOOP_type_outer_c )
        {
            //Should be 4 edges
            IntArray edges;
            CheckM(PKUtils().LoopGetEdges(loops[i]) == 4);
            CheckM(PKUtils().LoopGetEdges(loops[i],&edges) == 4 && edges.size() == 4);
            outer.insert(loops[i]);
        }
        else
        {
            CheckM(false);
        }
    }
    CheckM(inner.size() == 2 && outer.size() == 1);

    //Delete the inner loops from the sheet body
    IntArray innerLoopsToDelete;
    innerLoopsToDelete.assign(inner.begin(),inner.end());
    CheckM(PKUtils().LoopDeleteFromSheetBody(innerLoopsToDelete));
    CheckM(PKUtils().FaceGetLoops(sheetFace[0]) == 1);

    //Multi-face sheet - add one of the cone faces and a face disjoint from the other 2
    IntArray faces;
    faces.push_back(FaceOfId(vtkBody,33));
    faces.push_back(FaceOfId(vtkBody,125));
    faces.push_back(FaceOfId(vtkBody,27));

    //Should fail as faces are disconnected
    VTKBody multiFaceSheetVBody = VTKBody::MakeSheetBody(faces);
    CheckM(multiFaceSheetVBody.IsNull());

    //Remove the disconnected face
    faces.pop_back();

    multiFaceSheetVBody = VTKBody::MakeSheetBody(faces);
    CheckM(multiFaceSheetVBody.IsNotNull() && multiFaceSheetVBody != vtkBody);

    IntArray sheetFaces;
    multiFaceSheetVBody.GetFaces(TestGetCurrentVersion(),&sheetFaces);
    CheckM(sheetFaces.size() == 2);

    //Should be one face with 2 loops and one face with 3 loops
    bool twoLoops = false;
    bool threeLoops = false;
    IntSet innerLoops;
    PK_LOOP_t unsharedLoop = 0;
    for ( i = 0; i < 2; ++i )
    {
        IntArray faceLoops;
        PKUtils().FaceGetLoops(sheetFaces[i],&faceLoops);
        if ( faceLoops.size() == 2 )
        {
            twoLoops = true;
        }
        else if ( faceLoops.size() == 3 )
        {
            threeLoops = true;
        }
        else
        {
            CheckM(false);
        }

        for ( size_t j = 0; j < faceLoops.size(); ++j )
        {
            PK_LOOP_type_t type = PKUtils().LoopGetType(faceLoops[j]);
            if ( type == PK_LOOP_type_inner_c )
            {
                innerLoops.insert(faceLoops[j]);
                IntArray edges, edgeFaces;
                if ( PKUtils().LoopGetEdges(faceLoops[j],&edges) == 1 && 
                     PKUtils().EdgeGetFaces(edges[0],edgeFaces) == 1 )
                {
                    unsharedLoop = faceLoops[j];
                }
            }
        }
    }
    CheckM(twoLoops && threeLoops);

    //Delete the inner loops which should fail as one of the loops contains edges used by an outer loop for another face
    innerLoopsToDelete.assign(innerLoops.begin(),innerLoops.end());
    CheckM(!PKUtils().LoopDeleteFromSheetBody(innerLoopsToDelete));

    //Delete the unshared loop
    IntArray unsharedLoops;
    unsharedLoops.push_back(unsharedLoop);
    CheckM(PKUtils().LoopDeleteFromSheetBody(unsharedLoops));

    return true;
}

VTK_TEST_FN(TestPKUtils012)
{
    //Test for deleting an inner singularity loop which has an edge used by more than one face
    VTKBody vtkBody = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube2ConeBosses.x_t");
    PK_BODY_t body = vtkBody.GetPKBodyTag();

    IntArray faces;
    faces.push_back(FaceOfId(vtkBody,125));
    faces.push_back(FaceOfId(vtkBody,129));

    VTKBody sheetVBody = VTKBody::MakeSheetBody(faces);
    CheckM( sheetVBody.IsNotNull() );

    //Find the inner singularity loop and try to remove it
    IntArray sheetFaces;    
    sheetVBody.GetFaces(TestGetCurrentVersion(),&sheetFaces);
    CheckM(sheetFaces.size() == 2);
    PK_LOOP_t loop = 0;
    for ( size_t i = 0; i < 2 && loop == 0; ++i )
    {
        IntArray loops;
        PKUtils().FaceGetLoops(sheetFaces[i],&loops);
        for ( size_t j = 0; j < loops.size() && loop == 0; ++j )
        {
            if ( PKUtils().LoopGetType(loops[j]) == PK_LOOP_type_inner_sing_c )
            {
                loop = loops[j];
            }
        }
    }
    CheckM(loop != 0);

    //The delete will fail
    IntArray loopToDelete;
    loopToDelete.push_back(loop);
    bool deleted = PKUtils().LoopDeleteFromSheetBody(loopToDelete);
    CheckM(!deleted);

    return true;
}

VTK_TEST_FN(TestPKUtils013)
{
    //Test for deleting an inner singularity loop which has an edge used by more than one face
    VTKBody vtkBody = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube50mmBoss2.x_t");
    
    // Provide a planar face
    CheckM( PKUtils().EntityIsPlanarFace(FaceOfId(vtkBody,26)) == true );
    // Provide a cylinder
    CheckM( PKUtils().EntityIsPlanarFace(FaceOfId(vtkBody,128)) == false );
    // Provide a circular face
    CheckM( PKUtils().EntityIsPlanarFace(FaceOfId(vtkBody,135)) == true );   

    return true;
}

VTK_TEST_FN(TestPKUtils014)
{
    //Test for PKUtils().ComputeConeRadiusAtAxisPos
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/EquivalenceClass/anti_aligned_cones.xml").c_str());

    VTKBodyArray bodies;
    VTKBody::SessionGetAllPKBodies(bodies);
    CheckM(bodies.size() == 1);

    PK_FACE_t coneFace = FaceOfId(bodies[0],125);
    PK_EDGE_t smallEdge = EdgeOfId(bodies[0],122);
    PK_EDGE_t largeEdge = EdgeOfId(bodies[0],120);

    PK_CONE_t cone = 0;
    Vec conePos;
    double coneRadius = 0.0;
    PKUtils().GetGeo(coneFace,&cone,0,&conePos,0,&coneRadius);

    //Find the 2 circular edges
    Vec smallCentre;
    double smallRadius = 0.0;
    PKUtils().GetGeo(smallEdge,0,0,&smallCentre,0,&smallRadius);
    Vec largeCentre;
    double largeRadius = 0.0;
    PKUtils().GetGeo(largeEdge,0,0,&largeCentre,0,&largeRadius);

    //Find points on the edges (and hence on the cone)
    Vec smallPos (smallCentre + Vec(smallRadius,0.0,0.0));
    Vec largePos (largeCentre + Vec(largeRadius,0.0,0.0));

    //The cone radius at these positions should equal the circle radiuses
    double radius1 = 0.0;
    CheckM(!PKUtils().ConeComputeRadiusAtAxisPos(cone,smallPos,radius1)); //Position not on the axis
    CheckM(PKUtils().ConeComputeRadiusAtAxisPos(cone,smallCentre,radius1));
    CheckM(ResZero(radius1 - smallRadius));

    double radius2 = 0.0;
    CheckM(!PKUtils().ConeComputeRadiusAtAxisPos(cone,largePos,radius2)); //Position not on the axis
    CheckM(PKUtils().ConeComputeRadiusAtAxisPos(cone,largeCentre,radius2));
    CheckM(ResZero(radius2 - largeRadius));

    //Check the radius at the cone base point
    double radius3 = 0.0;
    Vec pos(conePos + Vec(coneRadius,0.0,0.0));
    CheckM(!PKUtils().ConeComputeRadiusAtAxisPos(cone,pos,radius3)); //Position not on the axis
    CheckM(PKUtils().ConeComputeRadiusAtAxisPos(cone,conePos,radius3));
    CheckM(ResZero(radius3 - coneRadius));

    //Pass a non-cone
    PK_GEOM_t cyl = 0;
    PKUtils().GetGeo(FaceOfId(bodies[0],130),&cyl);
    CheckM(cyl != 0 && !PKUtils().ConeComputeRadiusAtAxisPos(cyl,conePos,radius3));

    return true;
}

VTK_TEST_FN(TestPKUtils015)
{
     // Test for PKUtils().EntityIsAttribute
    VTKBody vtkBody = TestUtilLoadSingleBody("vtk_part_data:Misc/slab.x_t");

    // Provide an attribute, in this case a colour attribute
    CheckM( PKUtils().EntityIsAttribute(TagOfId(vtkBody,EntityClass::Attrib,190)) == true );

    // Negative test: provide a face
    CheckM( PKUtils().EntityIsAttribute(TagOfId(vtkBody,EntityClass::Face,145)) == false );

    return true;
}

VTK_TEST_FN(TestPKUtils016)
{
    //Test for PKUtils().AttribCreateEmpty, which uses PKUtils().EntityIsAttribute to validate result
    VTKBody vtkBody = TestUtilLoadSingleBody("vtk_part_data:Misc/slab.x_t");

    PK_ATTRIB_t newAttrib = PK_ENTITY_null;
    CheckM( PKUtils().EntityIsAttribute(newAttrib) == false );

    CheckM( PKUtils().AttribCreateEmpty(FaceOfId(vtkBody, 145),PKUtils().GetTempAttDefTag(), newAttrib));

    // Validating result
    CheckM( PKUtils().EntityIsAttribute(newAttrib) == true );

    return true;
}

VTK_TEST_FN(TestPKUtils017a)
{
    //Test for body comparison after centralising local diffs logic so it can be used for construction bodies

    VTKBody vbody1 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube.x_t");
    VTKBody vbody2 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube50mmBoss.x_t");

    InteractionTag interaction1 = CreateInteraction();

    //No construction bodies
    CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1),vbody1,vbody1));
    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1),vbody1,vbody2));

    return true;
}

VTK_TEST_FN(TestPKUtils017b)
{
    //Different bodies with construction bodies

    OptionSetAllowMultipleInteractions(true);

    InteractionTag interaction1 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/BodyWithConstructionBody.xml").c_str());
    InteractionTag interaction2 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/fort2bBodies.xml").c_str());

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction1),bodies);
    CheckM(bodies.size() == 2);

    CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1),bodies[0],bodies[0]));
    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1),bodies[0],bodies[1]));

    OptionSetAllowMultipleInteractions(false);

    return true;
}

VTK_TEST_FN(TestPKUtils017c)
{
    //Bodies with and without construction bodies
    InteractionTag interaction1 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/BodyWithConstructionBody.xml").c_str());
    VTKBody vbody1 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube.x_t");

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction1),bodies);
    CheckM(bodies.size() == 2);

    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1),bodies[0],bodies[1]));

    return true;
}

VTK_TEST_FN(TestPKUtils017d)
{
    //Same body with different construction bodies

    OptionSetAllowMultipleInteractions(true);

    InteractionTag interaction1 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/fort2bBodies.xml").c_str());
    InteractionTag interaction2 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/fort2Bodies.xml").c_str());

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction1),bodies);
    CheckM(bodies.size() == 2);

    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1),bodies[0],bodies[1]));

    OptionSetAllowMultipleInteractions(false);

    return true;
}

VTK_TEST_FN(TestPKUtils017e)
{
    //Same body with different numbers of construction bodies
    OptionSetAllowMultipleInteractions(true);

    InteractionTag interaction1 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/fort2Bodies.xml").c_str());
    InteractionTag interaction2 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ConstructionBody/fort3Bodies.xml").c_str());

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction1),bodies);
    CheckM(bodies.size() == 2);

    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1),bodies[0],bodies[1]));

    OptionSetAllowMultipleInteractions(false);

    return true;
}

VTK_TEST_FN(TestPKUtils017f)
{
    //Compare cubes in different positions
    VTKBody vbody1 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube.x_t");
    VTKBody vbody2 = TestUtilLoadSingleBody("vtk_part_data:Misc/OriginCentroidCube.x_t");

    InteractionTag interaction1 = CreateInteraction();

    //No construction bodies
    CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1),vbody1,vbody1));
    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1),vbody1,vbody2));

    return true;
}

VTK_TEST_FN(TestPKUtils018)
{
        //Test for detecting vertex tolerance changes in body comparison
        InteractionTag interaction1 = CreateInteraction();
        VTKBody vbody1 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube.x_t");
        VTKBody vbody2 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube.x_t");

        //Make an edge tolerant on vbody1
        int edge1 = EdgeOfId(vbody1, 12);
        IntArray edges1;
        PK_ERROR_t error = PKUtils().EdgeSetPrecision(edge1, 1e-7, edges1);
        CheckM(error == PK_ERROR_no_errors);

        //Bodies should be different
        CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1), vbody1, vbody2));

        //Make the edge tolerant on vbody2 but with a smaller precision than vbody1 - should not be seen as a different
        int edge2 = EdgeOfId(vbody2, 12);
        IntArray edges2;
        error = PKUtils().EdgeSetPrecision(edge2, 5e-8, edges2);
        CheckM(error == PK_ERROR_no_errors);

        //Bodies should be identical
        CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1), vbody1, vbody2));

        //Make the edge on vbody2 tolerant with a precision which is greater than on vbody1 but smaller than the modelling tolerance
        IntArray edges3;
        error = PKUtils().EdgeSetPrecision(edge2, 5e-7, edges3);
        CheckM(error == PK_ERROR_no_errors);

        //Bodies should be identical
        CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1), vbody1, vbody2));

        //Make the edge on vbody1 tolerant with a precision less than the modelling tolerance
        IntArray edges4;
        error = PKUtils().EdgeSetPrecision(edge2, 5e-6, edges4);
        CheckM(error == PK_ERROR_no_errors);


        //Bodies should be different
        CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction1), vbody1, vbody2));

        return true;
}

VTK_TEST_FN(TestPKUtils019)
{
    //Test for detecting changes larger than the modelling resolution
    InteractionTag interaction = CreateInteraction();
    VTKBody vbody1 = TestUtilLoadSingleBody("vtk_part_data:Misc/Origin100mmCube.x_t");
    VTKBody vbody2 = vbody1.CopyPKBody(GetVersion(interaction));

    AddSelection(interaction,CreateReference(interaction,FaceOfId(vbody1,26),0));

    double linePos [3] = {0.0,0.0,0.0};
    double lineDir [3] = {0.0,0.0,1.0};
    OperationSetDragLinear(interaction,linePos,lineDir,linePos);
    CheckM(OperationConfigure(interaction));
    OperationStart(interaction);
    double dragPos [3] = {0.0,0.0,2e-6};
    OperationStepDragLinear(interaction,dragPos);
    CheckM(OperationUpdate(interaction) == UpdateApplied);

    //Bodies should be different
    CheckM(TestUtilAreBodiesDifferent(GetVersion(interaction),vbody1,vbody2));
    CheckM(!PKUtils().DebugBodyCompare(GetVersion(interaction),vbody1,vbody2));

    return true;
}

VTK_TEST_FN(TestPKUtils020)
{
    //PKUtils().EdgeGetConnectedEdges
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_wire_body_with_closed_edges.xml").c_str(), result );
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::PartitionGetBodies(PKUtils().GetCurrentPartition(), &bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies[0];

    //Five connected edges, and two closed edges.
    TagArray wireEdges;
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 9));
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 14));
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 19));
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 24));
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 29));
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 2));
    wireEdges.Add(wireBody.FindEntityByIdentifier(EntityClass::Edge, 33));

    //For each edge, normalise the directions of all edges to be consistent with this edge,
    //query connected edges and directions, all should be in the same direction as this edge.
    for(int i=0; i<5; i++)
    {
        PK_EDGE_propagate_orientation(wireEdges[i]);
        IntIntPairArray connectedEdges;
        PKUtils().EdgeGetConnectedEdgeDirections(wireEdges[i], connectedEdges);
        CheckM(connectedEdges.size() == 5);
        for(int j=0; j<5; j++)
        {
            CheckM(connectedEdges[j].second == 1);
        }
    }

    //For each edge, normalise the directions of all edges to be consistent with this edge,
    //then reverse this edge.
    //query connected edges and directions, all should be in the opposite direction as this edge.
    for(int i=0; i<5; i++)
    {
        PK_EDGE_propagate_orientation(wireEdges[i]);
        PK_EDGE_reverse(wireEdges[i]);
        IntIntPairArray connectedEdges;
        PKUtils().EdgeGetConnectedEdgeDirections(wireEdges[i], connectedEdges);
        CheckM(connectedEdges.size() == 5);
        for(int j=0; j<5; j++)
        {
            if(connectedEdges[j].first != wireEdges[i])
                CheckM(connectedEdges[j].second == -1);
            else
                CheckM(connectedEdges[j].second == 1);
        }
    }

    //For each edge, first normalise the directions of all edges to be consistent with this edge,
    //then reverse the edges with even indices.
    //If the current edge has even index, then all connected edges with even indices should have the same
    //direction and those with odd indices should have opposite direction.
    for(int i=0; i<5; i++)
    {
        PK_EDGE_propagate_orientation(wireEdges[i]);
        for(int j=0; j<5; j++)
        {
            if(j%2 == 0)
                PK_EDGE_reverse(wireEdges[j]);
        }

        IntIntPairArray connectedEdges, edgeDirections;
        PKUtils().EdgeGetConnectedEdgeDirections(wireEdges[i], connectedEdges);
        CheckM(connectedEdges.size() == 5);
        for(int j=0; j<5; j++)
        {
            //find the index of this edge in the main array
            int idx = wireEdges.Find(connectedEdges[j].first);
            CheckM(idx >= 0);
            if(i%2 == idx%2)
                CheckM(connectedEdges[j].second == 1);
            else
                CheckM(connectedEdges[j].second == -1);
        }
    }

    return true;
}

//Compute given number of evenly distributed points along the edge.
//The start and end positions will NOT be included. So if 'count' is 1, the mid-point on the edge will be returned.
void s_EdgeGetSamplePoints(PK_EDGE_t edge, int count, std::vector<Vec>& points)
{
    // This is safe for tolerant edges...
    points.clear();
    if (count < 1)
        return;

    PK_CURVE_t      curve;
    PK_CLASS_t      pkClass;
    VecArray        ends;
    double          interval[2];
    bool            sense;
    PKUtils().EdgeGetGeometry(edge, true, curve, pkClass, ends, interval, sense);

    double increment = (interval[1] - interval[0]) / (1 + count);
    for (int i = 0; i<count; i++)
    {
        PK_VECTOR_t point[1];
        PK_CURVE_eval(curve, interval[0] + (i + 1)*increment, 0, point);
        points.push_back(Vec(point[0].coord));
    }
}

VTK_TEST_FN(TestPKUtils021)
{
    //PKUtils().EdgeFindProjectionOfPoint
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/profile_with_bcurve_and_ellipse.xml").c_str(), result );
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::PartitionGetBodies(PKUtils().GetCurrentPartition(), &bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies[0];
    //Profile* profile = (Profile*)(interaction->GetProfile(0));
    //CheckM(profile);
    Dir profileNormal = Normal(Vec(-0.7071067811865468, 0.7071067811865484, 7.113754030285689e-014));

    PK_EDGE_t lineEdge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 18);
    CheckTagM(lineEdge);
    PK_EDGE_t circEdge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 6);
    CheckTagM(circEdge);
    PK_EDGE_t icurveEdge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 11);
    CheckTagM(icurveEdge);
    PK_EDGE_t elliEdge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 22);
    CheckTagM(elliEdge);
    IntArray edges;
    edges.push_back(lineEdge);
    edges.push_back(circEdge);
    edges.push_back(icurveEdge);
    edges.push_back(elliEdge);

    for(int i=0; i<(int)edges.size(); i++)
    {
        PK_EDGE_t edge = edges[i];

        //Sample a number of points on the edge.
        VecArray points;
        s_EdgeGetSamplePoints(edge, 3, points);
        for(int j=0; j<(int)points.size(); j++)
        {
            //First check we can project the points on the edge.
            Vec proj;
            bool found = PKUtils().EdgeFindProjectionOfPoint(edge, points[j], proj);
            CheckM(found);
            CheckM(ResEqual(proj, points[j]));

            //Move the point along the normal of the edge a bit, then project it back.
            Dir tangent;
            PKUtils().EdgeFindTangentAtPoint(edge, points[j], tangent);
            Vec norm = tangent * profileNormal;

            found = PKUtils().EdgeFindProjectionOfPoint(edge, points[j]+norm*0.01, proj);
            CheckM(found);
            CheckM(ResEqual(proj, points[j]));

            found = PKUtils().EdgeFindProjectionOfPoint(edge, points[j]-norm*0.01, proj);
            CheckM(found);
            CheckM(ResEqual(proj, points[j]));
        }
    }
    return true;
}

bool TestPKUtils022_Base(PK_EDGE_t edge)
{
    VecArray vertexPoints;
    PKUtils().EdgeGetVertexPoints(edge, vertexPoints);
    Dir edgeDir = Normal(vertexPoints[1]-vertexPoints[0]);

    //Sample a number of points on the edge.
    VecArray points;
    s_EdgeGetSamplePoints(edge, 3, points);
    for(int j=0; j<(int)points.size(); j++)
    {
        Dir tangent;
        bool found = PKUtils().EdgeFindTangentAtPoint(edge, points[j], tangent);
        CheckM(found);

        //Check the tangent directions are indeed tangent to the edge
        //and more importantly check they are aligned with the edge direction (i.e. pointing from the start point to the end point)
        Vec pos;
        Dir dir;
        EntityClass edgeClass = PKUtils().GetGeo(edge, 0, 0, &pos, &dir);
        if(edgeClass == EntityClass::Line)
        {
            CheckM(ResParallel(tangent, dir));
            CheckM(dir.IsAligned(edgeDir));
        }
        else if(edgeClass == EntityClass::Circle)
        {
            //Parallel to the cross-product of the circle axis and radius vector
            CheckM(ResParallel(tangent, Normal(dir*Normal(points[j]-pos))));
            //Aligned with the edge direction.
            CheckM(tangent % edgeDir > Resabs);
        }
        else
        {
            //The tangent should be computed, but we are unable to check if its correct.
        }
    }

    return true;
}

VTK_TEST_FN(TestPKUtils022)
{
    //PKUtils().EdgeFindTangentAtPoint
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/wirebody_breadslice_blended.xml").c_str(), result );
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::PartitionGetBodies(PKUtils().GetCurrentPartition(), &bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies[0];

    PK_EDGE_t lineEdge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 6);
    CheckTagM(lineEdge);
    PK_EDGE_t circEdge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 26);
    CheckTagM(circEdge);
    IntArray edges;
    edges.push_back(lineEdge);
    edges.push_back(circEdge);

    //Check the tangents are computed correctly.
    for(int i=0; i<(int)edges.size(); i++)
    {
        PK_EDGE_t edge = edges[i];
        CheckM(TestPKUtils022_Base(edge));
    }

    //Reverse the edges and check again.
    for(int i=0; i<(int)edges.size(); i++)
    {
        PK_EDGE_t edge = edges[i];
        PK_EDGE_reverse(edge);
        CheckM(TestPKUtils022_Base(edge));
    }

    return true;
}

static int CompareArrays(const IntArray& arr1, const IntArray& arr2, bool sortAndCompare = true)
{
    int diff = int(arr1.size() - arr2.size());

    if(diff == 0)
    {
        if(sortAndCompare)
        {
            IntArray localArr1(arr1);
            IntArray localArr2(arr2);

            std::sort( localArr1.begin(), localArr1.end() );
            std::sort( localArr2.begin(), localArr2.end() );

            return CompareArrays(localArr1, localArr2, false);
        }
        else
        {
            for(int i=0; i<(int)arr1.size() && diff == 0; i++)
            {
                diff = arr1[i] - arr2[i];
            }
        }
    }

    return diff;    
}

VTK_TEST_FN(TestPKUtils023_EdgeGrouping01)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/gasket_port_profile.xml").c_str(), result );
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::PartitionGetBodies(PKUtils().GetCurrentPartition(), &bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies[0];

    IntArray allEdges;
    wireBody.GetEdges(interaction->GetVersion(), &allEdges);
    CheckM(allEdges.size() == 53);

    IntArrayArray groups, groupEnds;
    PKUtils().EdgesGroupConnectedEdges(allEdges, groups, &groupEnds);

    CheckM(groups.size() == 14);
    CheckM(groupEnds.size() == 14);

    IntToIntMap counts;
    IntSet ringEdges;
    IntToIntArrayMap edgeToLoopMap;
    PK_EDGE_t edge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 17);
    PKUtils().WireBodyEdgeGetConnectedEdges(edge, edgeToLoopMap[edge]); 
    edge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 191);
    PKUtils().WireBodyEdgeGetConnectedEdges(edge, edgeToLoopMap[edge]);
    edge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 236);
    PKUtils().WireBodyEdgeGetConnectedEdges(edge, edgeToLoopMap[edge]);
    edge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 212);
    PKUtils().WireBodyEdgeGetConnectedEdges(edge, edgeToLoopMap[edge]);
    edge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 41);
    PKUtils().WireBodyEdgeGetConnectedEdges(edge, edgeToLoopMap[edge]);
    for(int i=0; i<(int)groups.size(); i++)
    {
        //All groups are close loops, so no ends should be returned.
        CheckM(groupEnds[i].empty());
        counts[(int)groups[i].size()]++;
        if(groups[i].size() == 1)
        {
            ringEdges.insert(groups[i][0]);
        }
        else if(groups[i].size() == 4 || groups[i].size() == 28)
        {
            IntSet set1;
            set1.insert(groups[i].begin(), groups[i].end());
            IntToIntArrayMapIter it=edgeToLoopMap.begin();
            for(; it!=edgeToLoopMap.end(); it++)
            {
                if(set1.find(it->first) != set1.end())
                {
                    IntSet set2;
                    set2.insert(it->second.begin(), it->second.end());
                    CheckM(set1 == set2);
                    break;
                }
            }
            //Must be found in the expected groups.
            CheckM(it != edgeToLoopMap.end());
        }
        else
        {
            return false;
        }
    }
    CheckM(counts.size() == 3);
    CheckM(counts[1] == 9);
    CheckM(counts[4] == 4);
    CheckM(counts[28] == 1);

    //Now remove some of the input edges so that they are broken into segments.
    ArrayUtils::EraseByValue(allEdges, wireBody.FindEntityByIdentifier(EntityClass::Edge, 71));
    ArrayUtils::EraseByValue(allEdges, wireBody.FindEntityByIdentifier(EntityClass::Edge, 151));
    ArrayUtils::EraseByValue(allEdges, wireBody.FindEntityByIdentifier(EntityClass::Edge, 26));
    ArrayUtils::EraseByValue(allEdges, wireBody.FindEntityByIdentifier(EntityClass::Edge, 177));
    ArrayUtils::EraseByValue(allEdges, wireBody.FindEntityByIdentifier(EntityClass::Edge, 245));
    ArrayUtils::EraseByValue(allEdges, wireBody.FindEntityByIdentifier(EntityClass::Edge, 217));
    CheckM(allEdges.size() == 47);
    edgeToLoopMap.clear();
    IntToIntArrayMap edgeToEndsMap;
    IntArray edges;
    edges.clear();
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 22));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 17));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 12));
    edgeToLoopMap[edges[0]] = edges;
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 8));
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 21));
    edges.clear();
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 191));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 187));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 182));
    edgeToLoopMap[edges[0]] = edges;
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 173));
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 176));
    edges.clear();
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 231));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 236));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 241));
    edgeToLoopMap[edges[0]] = edges;
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 227));
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 240));
    edges.clear();
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 212));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 207));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 221));
    edgeToLoopMap[edges[0]] = edges;
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 211));
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 216));
    edges.clear();
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 66));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 61));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 56));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 51));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 46));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 41));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 36));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 170));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 166));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 161));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 156));
    edgeToLoopMap[edges[0]] = edges;
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 65));
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 150));
    edges.clear();
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 76));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 81));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 86));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 91));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 96));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 101));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 106));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 111));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 116));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 121));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 126));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 131));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 136));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 141));
    edges.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 146));
    edgeToLoopMap[edges[0]] = edges;
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 70));
    edgeToEndsMap[edges[0]].push_back(wireBody.FindEntityByIdentifier(EntityClass::Vertex, 145));

    groups.clear();
    groupEnds.clear();
    PKUtils().EdgesGroupConnectedEdges(allEdges, groups, &groupEnds);
    CheckM(groups.size() == 15);
    CheckM(groupEnds.size() == 15);
    counts.clear();
    for(int i=0; i<(int)groups.size(); i++)
    {
        counts[(int)groups[i].size()]++;

        if(groups[i].size() == 1)
        {
            //All groups are close loops, so no ends should be returned.
            CheckM(groupEnds[i].empty());
            ringEdges.insert(groups[i][0]);
        }
        else
        {
            CheckM(groupEnds[i].size() == 2);

            bool foundInExpectedGroups = false;
            for(int j=0; j<(int)groups[i].size(); j++)
            {
                IntToIntArrayMapIter it1=edgeToLoopMap.find(groups[i][j]);
                IntToIntArrayMapIter it2=edgeToEndsMap.find(groups[i][j]);
                if(it1 != edgeToLoopMap.end() && it2 != edgeToEndsMap.end())
                {
                    foundInExpectedGroups = CompareArrays(groups[i], it1->second) == 0 &&
                                            CompareArrays(groupEnds[i], it2->second) == 0;
                    break;
                }
            }
            CheckM(foundInExpectedGroups);
        }
    }
    CheckM(counts.size() == 4);
    CheckM(counts[1] == 9);
    CheckM(counts[3] == 4);
    CheckM(counts[11] == 1);
    CheckM(counts[15] == 1);

    return true;
}

VTK_TEST_FN(TestPKUtils024_EdgeDeleteFromWireHealAuto_PR6786955_Case01)
{
    //PK_EDGE_delete throws unhandled exception.
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/castle_wire_body_with_distance.xml").c_str(),result);
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies.at(0);

    IntArray edgesToDelete;
    edgesToDelete.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 51));

    PKTopolTrackResults pkTopolTracking;
    PKTopolLocalResults pkTopolLocalResults;
    PK_EDGE_delete_o_t opts;
    PK_EDGE_delete_o_m(opts);

    ParasolidJournalUtils::StartParasolidJournal("PK_EDGE_delete", true);
    PK_ERROR_t error = PK_EDGE_delete((int)edgesToDelete.size(), &(edgesToDelete.front()), &opts, &pkTopolTracking, &pkTopolLocalResults);
    ParasolidJournalUtils::StopParasolidJournal();

    //The wound cannot be healed, so we expect an error (ideally PK_ERROR_cant_heal_wound) and a valid body.
    CheckM(wireBody.CheckOk() == true);
    CheckM(error != PK_ERROR_no_errors);
    
    return true;
}

VTK_TEST_FN(TestPKUtils024_EdgeDeleteFromWireHealAuto_PR6786955_Case02)
{
    //PK_EDGE_delete returns no errors but leaves the body in an invalid state.
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_profile_and_closed_edges.xml").c_str(),result);
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies.at(0);
    
    IntArray edgesToDelete;
    edgesToDelete.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 9));
    edgesToDelete.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 14)); 
    PKTopolTrackResults pkTopolTracking;
    PKTopolLocalResults pkTopolLocalResults;
    PK_EDGE_delete_o_t opts;
    PK_EDGE_delete_o_m(opts);

    ParasolidJournalUtils::StartParasolidJournal("PK_EDGE_delete", true);
    PK_ERROR_t error = PK_EDGE_delete((int)edgesToDelete.size(), &(edgesToDelete.front()), &opts, &pkTopolTracking, &pkTopolLocalResults);
    ParasolidJournalUtils::StopParasolidJournal();
    CheckM(error == PK_ERROR_no_errors);

    //The wound cannot be healed, so we expect an error (ideally PK_ERROR_cant_heal_wound) and a valid body.
    int nFault = 0;
    PK_check_fault_t* faults = 0;
    wireBody.CheckOk(&nFault, &faults);
    CheckM(faults == NULL);
    CheckM(nFault == 0);

    return true;
}

VTK_TEST_FN(TestPKUtils025_CombinePKTopolTrackResults)
{
    PKTopolTrackResults combined;

    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Misc/CubeWithPlanes.xml").c_str(),result);
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);

    PK_FACE_t face1 = bodies[0].FaceFindByIdentifier(69); 
    PK_FACE_t face2 = bodies[0].FaceFindByIdentifier(67);
    PK_FACE_t face3 = bodies[0].FaceFindByIdentifier(26);

    Xform xfm1;
    xfm1.SetTranslation(Vec(0.01, 0, 0)); //Move a face along +x
    Xform xfm2;
    xfm2.SetTranslation(Vec(0, 0.01, 0)); //Move another face along +y 
    Xform xfm3;
    xfm3.SetTranslation(Vec(0, 0, 0.01)); //Move a third face along +z.
    PK_TRANSF_t transf1 = interaction->GetPKUtils().GetParasolidTransformTag(xfm1);
    PK_TRANSF_t transf2 = interaction->GetPKUtils().GetParasolidTransformTag(xfm2);
    PK_TRANSF_t transf3 = interaction->GetPKUtils().GetParasolidTransformTag(xfm3);
    PK_FACE_transform_o_t options;
    PK_FACE_transform_o_m(options);
    options.transform_step = PK_transform_step_all_c;
    PKTopolLocalResults localRes1;
    PKTopolTrackResults tracking1;
    PK_FACE_transform_2(1, &face1, &transf1, Resabs, &options, &tracking1, &localRes1);
    combined.Combine(tracking1);

    PKTopolTrackResults empty;
    combined.Combine(empty);

    PKTopolTrackResults tracking2;
    PKTopolLocalResults localRes2;
    PK_FACE_transform_2(1, &face2, &transf2, Resabs, &options, &tracking2, &localRes2);
    combined.Combine(tracking2);

    PKTopolTrackResults tracking3;
    PKTopolLocalResults localRes3;
    PK_FACE_transform_2(1, &face3, &transf3, Resabs, &options, &tracking3, &localRes3);
    combined.Combine(tracking3);

    //Check the content of the combined structure.
    CheckM(combined.m_Structure.n_track_records == (tracking1.m_Structure.n_track_records+tracking2.m_Structure.n_track_records+tracking3.m_Structure.n_track_records));
    int j = 0;
    PK_TOPOL_track_r_t* currentTracking = &tracking1;
    for(int i=0; i<combined.m_Structure.n_track_records; i++, j++)
    {
        if(i == tracking1.m_Structure.n_track_records)
        {
            currentTracking = &tracking2;
            j = 0;
        }
        else if(i == (tracking1.m_Structure.n_track_records+tracking2.m_Structure.n_track_records))
        {
            currentTracking = &tracking3;
            j = 0;
        }

        CheckM(combined.m_Structure.track_records[i].track == currentTracking->track_records[j].track);
        CheckM(combined.m_Structure.track_records[i].n_original_topols == currentTracking->track_records[j].n_original_topols);
        for(int k=0; k<combined.m_Structure.track_records[i].n_original_topols; k++)
        {
            CheckM(combined.m_Structure.track_records[i].original_topols[k] == currentTracking->track_records[j].original_topols[k]);
            CheckM(combined.m_Structure.track_records[i].original_classes[k] == currentTracking->track_records[j].original_classes[k]);
        }
        CheckM(combined.m_Structure.track_records[i].n_product_topols == currentTracking->track_records[j].n_product_topols);
        for(int k=0; k<combined.m_Structure.track_records[i].n_product_topols; k++)
        {
            CheckM(combined.m_Structure.track_records[i].product_topols[k] == currentTracking->track_records[j].product_topols[k]);
        }
    }

    tracking1.Clear();
    tracking2.Clear();
    tracking3.Clear();

    return true;
}

VTK_TEST_FN(TestPKUtils026_AreWireBodyEdgesConnected)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/gasket_port_profile.xml").c_str(), result );
    CheckResultM(result);
    VTKBodyArray bodies;
    VTKBody::PartitionGetBodies(PKUtils().GetCurrentPartition(), &bodies);
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies[0];

    IntArray allEdges;
    wireBody.GetEdges(interaction->GetVersion(), &allEdges);
    CheckM(allEdges.size() == 53);

    for(int i=0; i<(int)allEdges.size(); i++)
    {
        IntIntPairArray connectedEdges;
        PKUtils().EdgeGetConnectedEdgeDirections(allEdges[i], connectedEdges);
        IntSet connectedEdgeSet;
        for(int j=0; j<(int)connectedEdges.size(); j++)
            connectedEdgeSet.insert(connectedEdges[j].first);
        for(int j=0; j<(int)allEdges.size(); j++)
        {
            bool connected = connectedEdgeSet.find(allEdges[j]) != connectedEdgeSet.end();
            CheckM(connected == PKUtils().WireBodyEdgesAreConnected(allEdges[i], allEdges[j]));
        }
    }
    return true;
}


VTK_TEST_FN(TestPKUtils_FaceGetAdjacentFace)
{
    ResultType result = ResultTypeOk;
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/PkGeomUtilsTest.x_t");
    CheckVTKBodyM(vbody);

    PK_FACE_t face67Tag = FaceOfId(vbody, 67);
    CheckM(0 != face67Tag);
    PK_FACE_t face403Tag = FaceOfId(vbody, 403);
    CheckM(0 != face403Tag);
    PK_EDGE_t edge18Tag = EdgeOfId(vbody, 18);
    CheckM(0 != edge18Tag);

    PK_FACE_t adjFace = PKUtils().FaceGetAdjacentFace(face67Tag, edge18Tag);
    CheckM(adjFace == face403Tag);

    return true;
}

VTK_TEST_FN(TestPKUtils_FaceGetOuterLoopEdges)
{
    ResultType result = ResultTypeOk;
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/PkGeomUtilsTest.x_t");
    CheckVTKBodyM(vbody);

    PK_FACE_t face26Tag = FaceOfId(vbody, 26);
    CheckM(0 != face26Tag);

    IntSet outerEdges;
    PKUtils().FaceGetOuterLoopEdges(face26Tag, outerEdges);

    PK_EDGE_t edge23Tag = EdgeOfId(vbody, 23);
    CheckM(0 != edge23Tag);
    PK_EDGE_t edge6Tag = EdgeOfId(vbody, 6);
    CheckM(0 != edge6Tag);
    PK_EDGE_t edge400Tag = EdgeOfId(vbody, 400);
    CheckM(0 != edge400Tag);
    PK_EDGE_t edge392Tag = EdgeOfId(vbody, 392);
    CheckM(0 != edge392Tag);

    IntSet outerEdgesExpected;
    outerEdgesExpected.insert(edge23Tag);
    outerEdgesExpected.insert(edge6Tag);
    outerEdgesExpected.insert(edge400Tag);
    outerEdgesExpected.insert(edge392Tag);

    CheckM(outerEdges == outerEdgesExpected);

    return true;
}

VTK_TEST_FN(TestPKUtils_EdgeGetFacesByVertices)
{
    ResultType result = ResultTypeOk;
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Misc/PkGeomUtilsTest.x_t");
    CheckVTKBodyM(vbody);


    PK_EDGE_t edge12Tag = EdgeOfId(vbody, 12);
    CheckM(0 != edge12Tag);

    IntArray faces;
    PKUtils().EdgeGetFacesByVertices(edge12Tag, faces);

    PK_FACE_t face67Tag = FaceOfId(vbody, 67);
    CheckM(0 != face67Tag);
    PK_FACE_t face403Tag = FaceOfId(vbody, 403);
    CheckM(0 != face403Tag);
    PK_FACE_t face69Tag = FaceOfId(vbody, 69);
    CheckM(0 != face69Tag);
    PK_FACE_t face71Tag = FaceOfId(vbody, 71);
    CheckM(0 != face71Tag);

    IntSet expectedFaces;
    expectedFaces.insert(face67Tag);
    expectedFaces.insert(face69Tag);
    expectedFaces.insert(face403Tag);
    expectedFaces.insert(face71Tag);

    IntSet recivedFaces(faces.begin(), faces.end());

    CheckM(recivedFaces == expectedFaces);

    return true;
}

VTK_TEST_FN(TestPKUtils027)
{
    //Test for finding the boundary and interior of a set of edges
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_profile_and_closed_edges.xml").c_str(),result);

    Profile* profile = interaction->GetProfile(0);
    CheckM(profile != 0);

    VTKBodyArray vbodies = profile->GetWireBodies();
    CheckM(vbodies.size() == 1);

    IntArray edges;
    edges.push_back(EdgeOfId(vbodies[0],14));
    edges.push_back(EdgeOfId(vbodies[0],19));
    edges.push_back(EdgeOfId(vbodies[0],24));
    edges.push_back(EdgeOfId(vbodies[0],2));
    edges.push_back(EdgeOfId(vbodies[0],33));

    IntArray interior,boundary;
    PKUtils().EdgesGetInterior(edges,interior);
    PKUtils().EdgesGetBoundary(edges,boundary);

    IntSet foundInterior;
    foundInterior.insert(interior.begin(),interior.end());
    IntSet foundBoundary;
    foundBoundary.insert(boundary.begin(),boundary.end());

    CheckM(foundInterior.size() == 3);
    CheckM(foundInterior.find(EdgeOfId(vbodies[0],19)) != foundInterior.end());
    CheckM(foundInterior.find(EdgeOfId(vbodies[0],2)) != foundInterior.end());
    CheckM(foundInterior.find(EdgeOfId(vbodies[0],33)) != foundInterior.end());

    CheckM(foundBoundary.size() == 2);
    CheckM(foundBoundary.find(EdgeOfId(vbodies[0],14)) != foundBoundary.end());
    CheckM(foundBoundary.find(EdgeOfId(vbodies[0],24)) != foundBoundary.end());

    return true;
}

VTK_TEST_FN(TestPKUtils_VectorDifference)
{
    IntArray a;
    a.push_back(10);
    a.push_back(5);
    a.push_back(0);
    a.push_back(10);

    IntArray b;
    b.push_back(10);
    b.push_back(5);

    IntArray res;
    ArrayUtils::VectorDifference(a, b, res, false);

    IntArray expectedRes;
    expectedRes.push_back(0);
    expectedRes.push_back(10);

    return expectedRes == res;
}

VTK_TEST_FN(TestPKUtils_VectorIntersection)
{
    IntArray a;
    a.push_back(10);
    a.push_back(5);
    a.push_back(0);
    a.push_back(10);

    IntArray b;
    b.push_back(10);
    b.push_back(5);

    IntArray res;
    ArrayUtils::VectorIntersection(a, b, res, false);

    IntArray expectedRes;
    expectedRes.push_back(5);
    expectedRes.push_back(10);

    return expectedRes == res;
}

VTK_TEST_FN(TestPKUtils028_WireBodyEdgeSplitAndKnit)
{
    //Test the situation where removing disjoint edges from a body will leave the body disjoint
    //Depending on the option we may get a single disjoint body, or several bodies each of which is a single wire.
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/profile_castle.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = interaction->GetProfile(0);
    CheckM(profile);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    VTKBody wireBody = bodies[0];
    VTKBody originalBody = bodies[0].CopyPKBody(interaction->GetVersion());

    IntArray edgesToSplit;
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 56));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 61));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 66));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 11));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 6));
    IntSet edgesToSplitSet;
    edgesToSplitSet.insert(edgesToSplit.begin(), edgesToSplit.end());

    VTKBodyArray mainBodies, newBodies;
    IntIntPairArray splitVerts;
    //Allow bodies to be disjoint - we should have only one new body
    CheckM(PKUtils().WireBodyEdgeSplitToNewBodies(interaction->GetVersion(), edgesToSplit, true, mainBodies, newBodies, splitVerts, interaction->GetReferenceAttDefTag()));
    CheckM(mainBodies.size() == 1);
    CheckM(mainBodies[0] == wireBody);
    CheckM(newBodies.size() == 1);
    CheckM(splitVerts.size() == 4);
    CheckM(mainBodies[0].GetEdges(interaction->GetVersion()) == 11);
    //The edges in the new body should have the same tags (their ids will be different).
    IntSet newBodyEdges;
    CheckM(newBodies[0].GetEdges(interaction->GetVersion(), newBodyEdges) == 5);
    CheckM(newBodyEdges == edgesToSplitSet);

    //Test we can knit the edges back.
    CheckM(PKUtils().BodyKnitAtTopols(interaction->GetVersion(), wireBody, splitVerts));
    CheckM(wireBody.GetEdges(interaction->GetVersion()) == 16);
    for(int i=0; i<(int)edgesToSplit.size(); i++)
    {
        CheckM(interaction->EntityGetParentBody(edgesToSplit[i]) == wireBody);
    }
    PK_DEBUG_BODY_compare_o_t compareOpts;
    PK_DEBUG_BODY_compare_o_m(compareOpts);
    compareOpts.max_diffs = 1; compareOpts.tol_dev_tests = PK_LOGICAL_true;
    PK_DEBUG_BODY_compare_r_t compareRet;
    VTK::ResultType compareResult = VTK::ResultTypeOk;
    VTKBody::DebugBodyCompare(interaction->GetVersion(), wireBody, originalBody, &compareOpts, &compareRet, &compareResult);
    CheckResultM(compareResult);
    CheckM(compareRet.global_result == PK_DEBUG_global_res_no_diffs_c && compareRet.local_result == PK_DEBUG_local_res_no_diffs_c);
    PK_DEBUG_BODY_compare_r_f(&compareRet);

    //Do not allow disjoint bodies - the main body will be split into two, and we will have two new bodies.
    mainBodies.clear();
    newBodies.clear();
    splitVerts.clear();
    CheckM(PKUtils().WireBodyEdgeSplitToNewBodies(interaction->GetVersion(), edgesToSplit, false, mainBodies, newBodies, splitVerts, interaction->GetReferenceAttDefTag()));
    CheckM(mainBodies.size() == 2);
    CheckM(newBodies.size() == 2);
    CheckM(splitVerts.size() == 4);

    return true;
}

VTK_TEST_FN(TestPKUtils028_WireBodyEdgeSplitAndCombine)
{
    //Test the situation where a loop is removed from the wire-body (which contains multiple loops) so that 
    //there is no split vertices.
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/wire_body_with_closed_edges.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = interaction->GetProfile(0);
    CheckM(profile);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    VTKBody wireBody = bodies[0];
    VTKBody originalBody = bodies[0].CopyPKBody(interaction->GetVersion());

    IntArray edgesToSplit;
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 16));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 11));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 6));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 30));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 26));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 21));
    edgesToSplit.push_back(wireBody.FindEntityByIdentifier(EntityClass::Edge, 33));
    IntSet edgesToSplitSet;
    edgesToSplitSet.insert(edgesToSplit.begin(), edgesToSplit.end());

    VTKBodyArray mainBodies, newBodies;
    IntIntPairArray splitVerts;
    //Allow bodies to be disjoint - we should have only one new body
    CheckM(PKUtils().WireBodyEdgeSplitToNewBodies(interaction->GetVersion(), edgesToSplit, true, mainBodies, newBodies, splitVerts, interaction->GetReferenceAttDefTag()));
    CheckM(mainBodies.size() == 1);
    CheckM(mainBodies[0] == wireBody);
    CheckM(newBodies.size() == 1);
    CheckM(splitVerts.empty());
    CheckM(mainBodies[0].GetEdges(interaction->GetVersion()) == 1);
    //The edges in the new body should have the same tags (their ids will be different).
    IntSet newBodyEdges;
    CheckM(newBodies[0].GetEdges(interaction->GetVersion(), newBodyEdges) == 7);
    CheckM(newBodyEdges == edgesToSplitSet);

    //Test we can knit the edges back (there is no split vertices therefore we cannot use BodyKnitAtTopols).
    CheckM(VTKBody::CombineBodies(interaction->GetVersion(), wireBody, newBodies[0]));
    CheckM(wireBody.GetEdges(interaction->GetVersion()) == 8);
    for(int i=0; i<(int)edgesToSplit.size(); i++)
    {
        CheckM(interaction->EntityGetParentBody(edgesToSplit[i]) == wireBody);
    }
    PK_DEBUG_BODY_compare_o_t compareOpts;
    PK_DEBUG_BODY_compare_o_m(compareOpts);
    compareOpts.max_diffs = 1; compareOpts.tol_dev_tests = PK_LOGICAL_true;
    PK_DEBUG_BODY_compare_r_t compareRet;
    VTK::ResultType compareResult = VTK::ResultTypeOk;
    VTKBody::DebugBodyCompare(interaction->GetVersion(), wireBody, originalBody, &compareOpts, &compareRet, &compareResult);
    CheckResultM(compareResult);
    CheckM(compareRet.global_result == PK_DEBUG_global_res_no_diffs_c && compareRet.local_result == PK_DEBUG_local_res_no_diffs_c);
    PK_DEBUG_BODY_compare_r_f(&compareRet);

    //Do not allow disjoint bodies - the main body will remain a single body.
    //but PK_EDGE_remove_to_bodies only produces one new body instead of two. This is not an issue for us though as long as 
    //the new body contains all the split edges.
    mainBodies.clear();
    newBodies.clear();
    splitVerts.clear();
    CheckM(PKUtils().WireBodyEdgeSplitToNewBodies(interaction->GetVersion(), edgesToSplit, false, mainBodies, newBodies, splitVerts, interaction->GetReferenceAttDefTag()));
    CheckM(mainBodies.size() == 1);
    CheckM(newBodies.size() == 1);
    CheckM(splitVerts.empty());
    CheckM(newBodies[0].GetEdges(interaction->GetVersion(), newBodyEdges) == 7);
    CheckM(newBodyEdges == edgesToSplitSet);
    return true;
}

VTK_TEST_FN(TestPKUtils029_VertexIsSmooth)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/NonManifold/six_types_of_tae_profile.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(2));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 4);
    GeometricFindOptions * findOptions = interaction->GetGeometricFindOptions();
    CheckM(findOptions);

    //line1 and circ1 
    PK_EDGE_t line1 = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 6);
    PK_EDGE_t circ1 = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 14);
    //circ2 tae to line1 and circ1 at T-junctions on circ2
    PK_EDGE_t circ2 = bodies[1].FindEntityByIdentifier(EntityClass::Edge, 2);
    //circ3 tae to line1 and circ1 at T-junctions on line1 and circ1 respectively
    PK_EDGE_t circ3 = bodies[2].FindEntityByIdentifier(EntityClass::Edge, 6);
    //circ4 tae to line1 and circ1 at Y-junctions
    PK_EDGE_t circ4 = bodies[3].FindEntityByIdentifier(EntityClass::Edge, 6);

    //Vertex of line1 on circ2
    PK_VERTEX_t vert1 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 2);
    //Vertex of line1 coi to circ4's vertex
    PK_VERTEX_t vert2 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 5);
    //Vertex of circ1 on circ2
    PK_VERTEX_t vert3 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 13);
    //Vertex of circ1 coi to circ4's vertex
    PK_VERTEX_t vert4 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 10);
    //Vertex of circ3 on line1
    PK_VERTEX_t vert5 = bodies[2].FindEntityByIdentifier(EntityClass::Vertex, 2);
    //Vertex of circ3 on circ1
    PK_VERTEX_t vert6 = bodies[2].FindEntityByIdentifier(EntityClass::Vertex, 5);
    //Vertex of circ4 coi to vert2
    PK_VERTEX_t vert7 = bodies[3].FindEntityByIdentifier(EntityClass::Vertex, 5);
    //Vertex of circ4 coi to vert4
    PK_VERTEX_t vert8 = bodies[3].FindEntityByIdentifier(EntityClass::Vertex, 2);

    double angularTolerance = 1e-11;
    CheckM(PKUtils().VertexIsTangent(vert1, line1, circ2, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert2, line1, circ4, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert3, circ1, circ2, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert4, circ1, circ4, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert5, circ3, line1, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert6, circ3, circ1, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert7, circ4, line1, angularTolerance));
    CheckM(PKUtils().VertexIsTangent(vert8, circ4, circ1, angularTolerance));

    return true;
}
VTK_TEST_FN(TestPKUtils030_EdgesGetCoincidentVertices)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/NonManifold/six_types_of_tae_profile.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(2));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 4);
    GeometricFindOptions * findOptions = interaction->GetGeometricFindOptions();
    CheckM(findOptions);

    //line1 and circ1 
    PK_EDGE_t line1 = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 6);
    PK_EDGE_t circ1 = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 14);
    //circ2 tae to line1 and circ1 at T-junctions on circ2
    PK_EDGE_t circ2 = bodies[1].FindEntityByIdentifier(EntityClass::Edge, 2);
    //circ3 tae to line1 and circ1 at T-junctions on line1 and circ1 respectively
    PK_EDGE_t circ3 = bodies[2].FindEntityByIdentifier(EntityClass::Edge, 6);
    //circ4 tae to line1 and circ1 at Y-junctions
    PK_EDGE_t circ4 = bodies[3].FindEntityByIdentifier(EntityClass::Edge, 6);

    //Vertex of line1 on circ2
    PK_VERTEX_t vert1 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 2);
    //Vertex of line1 coi to circ4's vertex
    PK_VERTEX_t vert2 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 5);
    //Vertex of circ1 on circ2
    PK_VERTEX_t vert3 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 13);
    //Vertex of circ1 coi to circ4's vertex
    PK_VERTEX_t vert4 = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 10);
    //Vertex of circ3 on line1
    PK_VERTEX_t vert5 = bodies[2].FindEntityByIdentifier(EntityClass::Vertex, 2);
    //Vertex of circ3 on circ1
    PK_VERTEX_t vert6 = bodies[2].FindEntityByIdentifier(EntityClass::Vertex, 5);
    //Vertex of circ4 coi to vert2
    PK_VERTEX_t vert7 = bodies[3].FindEntityByIdentifier(EntityClass::Vertex, 5);
    //Vertex of circ4 coi to vert4
    PK_VERTEX_t vert8 = bodies[3].FindEntityByIdentifier(EntityClass::Vertex, 2);

    IntArray coiVertices;
    PKUtils().EdgesGetCoincidentVertices(line1, circ2, coiVertices);
    CheckM(coiVertices.size() == 1);
    CheckM(coiVertices[0] == vert1);
    PKUtils().EdgesGetCoincidentVertices(line1, circ4, coiVertices);
    CheckM(coiVertices.size() == 2);
    CheckM(std::find(coiVertices.begin(), coiVertices.end(), vert2) != coiVertices.end());
    CheckM(std::find(coiVertices.begin(), coiVertices.end(), vert7) != coiVertices.end());
    PKUtils().EdgesGetCoincidentVertices(circ1, circ2, coiVertices);
    CheckM(coiVertices.size() == 1);
    CheckM(coiVertices[0] == vert3);
    PKUtils().EdgesGetCoincidentVertices(circ1, circ4, coiVertices);
    CheckM(coiVertices.size() == 2);
    CheckM(std::find(coiVertices.begin(), coiVertices.end(), vert4) != coiVertices.end());
    CheckM(std::find(coiVertices.begin(), coiVertices.end(), vert8) != coiVertices.end());
    PKUtils().EdgesGetCoincidentVertices(circ3, line1, coiVertices);
    CheckM(coiVertices.size() == 1);
    CheckM(coiVertices[0] == vert5);
    PKUtils().EdgesGetCoincidentVertices(circ3, circ1, coiVertices);
    CheckM(coiVertices.size() == 1);
    CheckM(coiVertices[0] == vert6);

    return true;
}

VTK_TEST_FN(TestPKUtils031_EdgesGetCoincidentVertices)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/profile_with_tolerant_vertex.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(22));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t e1 = EdgeOfId(bodies[0], 16);
    PK_EDGE_t e2 = EdgeOfId(bodies[0], 21);
    //V1 is a tolerant vertex.
    PK_VERTEX_t v1 = VertexOfId(bodies[0], 15);
    IntArray coiVertices;
    PKUtils().EdgesGetCoincidentVertices(e1, e2, coiVertices);
    CheckM(coiVertices.size() == 1);
    CheckM(coiVertices[0] == v1);

    Vec pos = PKUtils().VertexGetPoint(v1);

    Vec linePos, circPos;
    Dir lineDir, circAxis;
    double radius;
    PKUtils().GetGeo(e1, NULL, NULL, &circPos, &circAxis, &radius);
    PKUtils().GetGeo(e2, NULL, NULL, &linePos, &lineDir);
    
    //Compute the intersection of the edges analytically and compare with the vertex's position.
    Vec chordCent = GeomUtils::ProjectPointOnLine(circPos, linePos, lineDir);
    Dir chordDir = CommonNormal(lineDir, circAxis);
    double centToChord = GeomUtils::PointLineDistance(circPos, linePos, lineDir);
    double chordHalfLen = sqrt(radius*radius - centToChord*centToChord);

    //The vertex is not coincident to the intersection within default tolerance.
    CheckM(ResEqual(pos, chordCent+lineDir*chordHalfLen) == false);

    //It is coincident to the intersection within vertex's precision.
    double vertPrecision = 0;
    PKUtils().VertexGetPrecision(v1, vertPrecision);
    CheckM(vertPrecision > Resabs);
    CheckM(TolerantEqual(pos, chordCent+lineDir*chordHalfLen, vertPrecision));

    return true;
}

VTK_TEST_FN(TestPKUtils031_DetectionOfEdgesWithOpenEndVertex)
{
    // check that only the correct edges are found with open end vertices

    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_end_wirebodies2_to_compare.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(30));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 3);

    PK_EDGE_t e1 = EdgeOfId(bodies[0], 6);
    PK_EDGE_t e2 = EdgeOfId(bodies[0], 11);
    PK_EDGE_t e3 = EdgeOfId(bodies[1], 6);
    PK_EDGE_t e4 = EdgeOfId(bodies[2], 6);

    CheckM(PKUtils().EdgeContainsOpenVertices(e1));
    CheckM(PKUtils().EdgeContainsOpenVertices(e2));
    CheckM(PKUtils().EdgeContainsOpenVertices(e3));
    CheckM(PKUtils().EdgeContainsOpenVertices(e4));
    
    return true;
}

VTK_TEST_FN(TestPKUtils032_DetectionOfEdgesWithOpenEndVertex)
{
    // check that only the correct edges are found with open end vertices

    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_profile_and_closed_edges.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(149));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t e1 = EdgeOfId(bodies[0], 6);

    // line edges
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 9)));
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 14)) == false);
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 19)) == false);
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 24)) == false);
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 29)));

    // full circular edges
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 2)) == false);
    CheckM(PKUtils().EdgeContainsOpenVertices(EdgeOfId(bodies[0], 33)) == false);

    return true;
}

VTK_TEST_FN(TestPKUtils033_BcurveGetGeo01)
{
    //Test output of PKUtils().BcurveGetGeo.
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurvs_deg4.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(5));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 2);

    PK_EDGE_t e1 = EdgeOfId(bodies[0], 6);
    CheckTagM(e1);
    PK_EDGE_t e2 = EdgeOfId(bodies[1], 16);
    CheckTagM(e2);
    PK_EDGE_t e3 = EdgeOfId(bodies[1], 31);
    CheckTagM(e3);

    PK_GEOM_t geom = 0;
    int degree = 0;
    std::vector<Vec> controlpoints; 
    std::vector<double> weights;
    std::vector<double> knots;
    bool isRational = true;
    bool isPeriodic = true;
    bool isClosed = true;
    bool suc = PKUtils().BcurveGetGeo(e1, &geom, 0, &degree, &controlpoints, &weights, &knots, &isRational, &isPeriodic, &isClosed);
    CheckM(suc);
    CheckM(PKUtils().TopolGetGeom(e1) == geom);
    CheckM(degree == 3);
    CheckM(controlpoints.size() == 6);
    CheckM(weights.empty());//empty due to non-rational
    CheckM(knots.size() == 10);
    CheckM(isRational == false);
    CheckM(isPeriodic == false);
    CheckM(isClosed == false);

    suc = PKUtils().BcurveGetGeo(e2, &geom, 0, &degree, &controlpoints, &weights, &knots, &isRational, &isPeriodic, &isClosed);
    CheckM(suc);
    CheckM(PKUtils().TopolGetGeom(e2) == geom);
    CheckM(degree == 3);
    CheckM(controlpoints.size() == 5);
    CheckM(weights.empty());//empty due to non-rational
    CheckM(knots.size() == 9);
    CheckM(isRational == false);
    CheckM(isPeriodic == false);
    CheckM(isClosed == false);

    suc = PKUtils().BcurveGetGeo(e3, &geom, 0, &degree, &controlpoints, &weights, &knots, &isRational, &isPeriodic, &isClosed);
    CheckM(suc);
    CheckM(PKUtils().TopolGetGeom(e3) == geom);
    CheckM(degree == 4);
    CheckM(controlpoints.size() == 7);
    CheckM(weights.empty());//empty due to non-rational
    CheckM(knots.size() == 12);
    CheckM(isRational == false);
    CheckM(isPeriodic == false);
    CheckM(isClosed == false);

    return true;
}

VTK_TEST_FN(TestPKUtils033_BcurveGetGeo02)
{
    //Test output of PKUtils().BcurveGetGeo on non-rational bcurve
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/non_rational_conic.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(2));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t e1 = EdgeOfId(bodies[0], 10);
    CheckTagM(e1);

    PK_GEOM_t geom = 0;
    int degree = 0;
    std::vector<Vec> controlpoints; 
    std::vector<double> weights;
    std::vector<double> knots;
    bool isRational = true;
    bool isPeriodic = true;
    bool isClosed = true;
    bool suc = PKUtils().BcurveGetGeo(e1, &geom, 0, &degree, &controlpoints, &weights, &knots, &isRational, &isPeriodic, &isClosed);
    CheckM(suc);
    CheckM(PKUtils().TopolGetGeom(e1) == geom);
    CheckM(degree == 2);
    CheckM(controlpoints.size() == 3);
    CheckM(weights.size() == 3); //rational curve with weights
    CheckM(knots.size() == 6);
    CheckM(isRational == true);
    CheckM(isPeriodic == false);
    CheckM(isClosed == false);

    return true;
}

VTK_TEST_FN(TestPKUtils033_BcurveGetGeo03)
{
    //Test output of PKUtils().BcurveGetGeo on non-rational bcurve
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurve_closed.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(2));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t e1 = EdgeOfId(bodies[0], 2);
    CheckTagM(e1);

    PK_GEOM_t geom = 0;
    int degree = 0;
    std::vector<Vec> controlpoints; 
    std::vector<double> weights;
    std::vector<double> knots;
    bool isRational = true;
    bool isPeriodic = true;
    bool isClosed = true;
    bool suc = PKUtils().BcurveGetGeo(e1, &geom, 0, &degree, &controlpoints, &weights, &knots, &isRational, &isPeriodic, &isClosed);
    CheckM(suc);
    CheckM(PKUtils().TopolGetGeom(e1) == geom);
    CheckM(degree == 3);
    CheckM(controlpoints.size() == 8);
    CheckM(weights.empty()); //empty due to non-rational
    CheckM(knots.size() == 12);
    CheckM(isRational == false);
    CheckM(isPeriodic == true);
    CheckM(isClosed == true);

    return true;
}

VTK_TEST_FN(TestPKUtils034_Bcurve01)
{
    //Unit test for PKUtils().BcurveInsertKnot
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurvs_deg4.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(5));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 2);

    //A degree-4 b-curve edge
    PK_EDGE_t edge = EdgeOfId(bodies[1], 31);
    CheckTagM(edge);

    PK_BCURVE_sf_t bcurveSf;
    PK_BCURVE_ask(PKUtils().EntityGetGeom(edge), &bcurveSf);

    PK_BCURVE_t copyBcurve = 0;
    PK_BCURVE_create(&bcurveSf, &copyBcurve);
    CheckTagM(copyBcurve);

    std::vector<double> knots;
    bool suc = PKUtils().BcurveGetGeo(edge, 0, 0, 0, 0, 0, &knots);
    CheckM(suc);

    //Insert an existing knot twice
    suc = PKUtils().BcurveInsertKnot(copyBcurve, knots[5], 2);
    CheckM(suc);
    std::vector<double> newKnots1;
    suc = PKUtils().BcurveGetGeo(copyBcurve, 0, 0, 0, 0, 0, &newKnots1);
    CheckM(suc);
    CheckM(newKnots1.size() == knots.size() + 2);
    CheckM(ResEqual(newKnots1[5], knots[5]));
    CheckM(ResEqual(newKnots1[6], knots[5]));
    CheckM(ResEqual(newKnots1[7], knots[5]));
    CheckM(ResEqual(newKnots1[8], knots[6]));
    PKUtils().EntityDelete(copyBcurve);

    //Insert a new knot 3 times
    PK_BCURVE_create(&bcurveSf, &copyBcurve);
    CheckTagM(copyBcurve);
    double newKnot = (knots[5] + knots[6])/2;
    suc = PKUtils().BcurveInsertKnot(copyBcurve, newKnot, 3);
    CheckM(suc);
    suc = PKUtils().BcurveGetGeo(copyBcurve, 0, 0, 0, 0, 0, &newKnots1);
    CheckM(suc);
    CheckM(newKnots1.size() == knots.size() + 3);
    CheckM(ResEqual(newKnots1[5], knots[5]));
    CheckM(ResEqual(newKnots1[6], newKnot));
    CheckM(ResEqual(newKnots1[7], newKnot));
    CheckM(ResEqual(newKnots1[8], newKnot));
    CheckM(ResEqual(newKnots1[9], knots[6]));

    suc = PKUtils().BcurveInsertKnot(copyBcurve, newKnot, 1);
    CheckM(suc);

    //inserting the fifth one will fail.
    suc = PKUtils().BcurveInsertKnot(copyBcurve, newKnot, 1);
    CheckM(suc == false);

    PKUtils().EntityDelete(copyBcurve);

    //Clean up
    PK_MEMORY_free(bcurveSf.vertex);
    PK_MEMORY_free(bcurveSf.knot);
    PK_MEMORY_free(bcurveSf.knot_mult);
    return true;
}

VTK_TEST_FN(TestPKUtils034_Bcurve02)
{
    //Unit test for PKUtils().BcurveTrim
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurvs_deg4.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(5));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 2);

    //A degree-4 b-curve edge
    PK_EDGE_t edge = EdgeOfId(bodies[1], 31);
    CheckTagM(edge);

    PK_BCURVE_t bcurve = PKUtils().TopolGetGeom(edge);
    PK_INTERVAL_t interval;
    PKUtils().CurveGetInterval(bcurve, interval);

    double lowKnot = interval.value[0] + (interval.value[1]-interval.value[0]) * 0.2;
    double highKnot = interval.value[1] - (interval.value[1]-interval.value[0]) * 0.2;

    //Trim without higher bound.
    PK_BCURVE_t newCurve1 = PKUtils().BcurveTrim(bcurve, &lowKnot, NULL);
    CheckTagM(newCurve1);
    
    std::vector<Vec> controlpoints; 
    std::vector<double> knots;
    bool suc = PKUtils().BcurveGetGeo(newCurve1, 0, 0, 0, &controlpoints, 0, &knots);
    CheckM(suc);
    PK_VECTOR_t point;
    //Check the end positions of the new curve.
    PK_CURVE_eval(bcurve, lowKnot, 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[0]));
    PK_CURVE_eval(bcurve, interval.value[1], 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[controlpoints.size()-1]));
    PKUtils().EntityDelete(newCurve1);

    //Trim without lower bound.
    PK_BCURVE_t newCurve2 = PKUtils().BcurveTrim(bcurve, NULL, &highKnot);
    CheckTagM(newCurve2);
    suc = PKUtils().BcurveGetGeo(newCurve2, 0, 0, 0, &controlpoints, 0, &knots);
    CheckM(suc);
    //Check the end positions of the new curve.
    PK_CURVE_eval(bcurve, interval.value[0], 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[0]));
    PK_CURVE_eval(bcurve, highKnot, 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[controlpoints.size()-1]));
    PKUtils().EntityDelete(newCurve2);

    //Trim both ends.
    PK_BCURVE_t newCurve3 = PKUtils().BcurveTrim(bcurve, &lowKnot, &highKnot);
    CheckTagM(newCurve3);
    suc = PKUtils().BcurveGetGeo(newCurve3, 0, 0, 0, &controlpoints, 0, &knots);
    CheckM(suc);
    //Check the end positions of the new curve.
    PK_CURVE_eval(bcurve, lowKnot, 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[0]));
    PK_CURVE_eval(bcurve, highKnot, 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[controlpoints.size()-1]));
    PKUtils().EntityDelete(newCurve3);

    //Trim with the same ends (a new copy curve will be created);
    lowKnot = interval.value[0];
    highKnot = interval.value[1];
    PK_BCURVE_t newCurve4 = PKUtils().BcurveTrim(bcurve, &lowKnot, &highKnot);
    CheckTagM(newCurve4);
    suc = PKUtils().BcurveGetGeo(newCurve4, 0, 0, 0, &controlpoints, 0, &knots);
    CheckM(suc);
    //Check the end positions of the new curve.
    PK_CURVE_eval(bcurve, lowKnot, 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[0]));
    PK_CURVE_eval(bcurve, highKnot, 0, &point);
    CheckM(ResEqual(Vec(point), controlpoints[controlpoints.size()-1]));
    PKUtils().EntityDelete(newCurve4);

    return true;
}


VTK_TEST_FN(TestPKUtils035_Mirroring_01)
{
    // Test that the new mirroring function produces the same result as Xform::mirror

    // check a transform
    Xform xform;
    xform.SetTranslation(Vec(0.5, 0.5, 0));
    Dir dir = Dir::XDir();
    Vec pos(0.1, 0, 0);
    Xform newXFrom = xform.Mirror(pos, dir);
    PK_TRANSF_sf_t pkTransf;
    xform.GetTransform(pkTransf.matrix);
    PK_TRANSF_sf_t newPKTransf{ PKUtils().TransformMirror(pkTransf, pos, dir) };
    Xform comparisonXForm(newPKTransf.matrix);
    CheckM(newXFrom.IsIdentical(comparisonXForm));

    return true;
}

VTK_TEST_FN(TestPKUtils035_Mirroring_02)
{
    // Test that the new mirroring supports scaling

    PK_TRANSF_sf_t pkTransf;
    pkTransf.matrix[0][0] = 1.0000000000000000; pkTransf.matrix[0][1] = 0.0000000000000000; pkTransf.matrix[0][2] = 0.0000000000000000; pkTransf.matrix[0][3] = 0.00059074552600355587;
    pkTransf.matrix[1][0] = 0.0000000000000000; pkTransf.matrix[1][1] = 1.0000000000000000; pkTransf.matrix[1][2] = 0.0000000000000000; pkTransf.matrix[1][3] = -0.0011111111111110894;
    pkTransf.matrix[2][0] = 0.0000000000000000; pkTransf.matrix[2][1] = 0.0000000000000000; pkTransf.matrix[2][2] = 1.0000000000000000; pkTransf.matrix[2][3] = 0.010526357174624862;
    pkTransf.matrix[3][0] = 0.0000000000000000; pkTransf.matrix[3][1] = 0.0000000000000000; pkTransf.matrix[3][2] = 0.0000000000000000; pkTransf.matrix[3][3] = 1.1111111111111112;

    Dir dir = Dir::XDir();
    Vec pos(0, 0, 0);

    PK_TRANSF_sf_t newPKTransf{ PKUtils().TransformMirror(pkTransf, pos, dir) };

    PK_TRANSF_sf_t expectedPKTransf;
    expectedPKTransf.matrix[0][0] = 1.0000000000000000;  expectedPKTransf.matrix[0][1] = 0.0000000000000000; expectedPKTransf.matrix[0][2] = 0.0000000000000000; expectedPKTransf.matrix[0][3] = -0.00059074552600355587;
    expectedPKTransf.matrix[1][0] = -0.0000000000000000; expectedPKTransf.matrix[1][1] = 1.0000000000000000; expectedPKTransf.matrix[1][2] = 0.0000000000000000; expectedPKTransf.matrix[1][3] = -0.0011111111111110894;
    expectedPKTransf.matrix[2][0] = -0.0000000000000000; expectedPKTransf.matrix[2][1] = 0.0000000000000000; expectedPKTransf.matrix[2][2] = 1.0000000000000000; expectedPKTransf.matrix[2][3] = 0.010526357174624862;
    expectedPKTransf.matrix[3][0] = 0.0000000000000000;  expectedPKTransf.matrix[3][1] = 0.0000000000000000; expectedPKTransf.matrix[3][2] = 0.0000000000000000; expectedPKTransf.matrix[3][3] = 1.1111111111111112;

    CheckM(memcmp(newPKTransf.matrix, expectedPKTransf.matrix, sizeof(double)*16) == 0);

    return true;
}

VTK_TEST_FN(TestPKUtils035_Mirroring_03)
{
    // Test that the new mirroring supports scaling with a reflection not through the origin

    PK_TRANSF_sf_t pkTransf;
    pkTransf.matrix[0][0] = 1.0000000000000000; pkTransf.matrix[0][1] = 0.0000000000000000; pkTransf.matrix[0][2] = 0.0000000000000000; pkTransf.matrix[0][3] = 0.1250000000000000;
    pkTransf.matrix[1][0] = 0.0000000000000000; pkTransf.matrix[1][1] = 1.0000000000000000; pkTransf.matrix[1][2] = 0.0000000000000000; pkTransf.matrix[1][3] = 0.5000000000000000;
    pkTransf.matrix[2][0] = 0.0000000000000000; pkTransf.matrix[2][1] = 0.0000000000000000; pkTransf.matrix[2][2] = 1.0000000000000000; pkTransf.matrix[2][3] = 0.6250000000000000;
    pkTransf.matrix[3][0] = 0.0000000000000000; pkTransf.matrix[3][1] = 0.0000000000000000; pkTransf.matrix[3][2] = 0.0000000000000000; pkTransf.matrix[3][3] = 2.0000000000000000;

    Dir dir = Dir::XDir();
    Vec pos(1.5, 0, 0);

    PK_TRANSF_sf_t newPKTransf{ PKUtils().TransformMirror(pkTransf, pos, dir) };

    PK_TRANSF_sf_t expectedPKTransf;
    expectedPKTransf.matrix[0][0] = 1.0000000000000000;  expectedPKTransf.matrix[0][1] = 0.0000000000000000; expectedPKTransf.matrix[0][2] = 0.0000000000000000; expectedPKTransf.matrix[0][3] = 2.8750000000000000;
    expectedPKTransf.matrix[1][0] = -0.0000000000000000; expectedPKTransf.matrix[1][1] = 1.0000000000000000; expectedPKTransf.matrix[1][2] = 0.0000000000000000; expectedPKTransf.matrix[1][3] = 0.5000000000000000;
    expectedPKTransf.matrix[2][0] = -0.0000000000000000; expectedPKTransf.matrix[2][1] = 0.0000000000000000; expectedPKTransf.matrix[2][2] = 1.0000000000000000; expectedPKTransf.matrix[2][3] = 0.6250000000000000;
    expectedPKTransf.matrix[3][0] = 0.0000000000000000;  expectedPKTransf.matrix[3][1] = 0.0000000000000000; expectedPKTransf.matrix[3][2] = 0.0000000000000000; expectedPKTransf.matrix[3][3] = 2.0000000000000000;

    CheckM(memcmp(newPKTransf.matrix, expectedPKTransf.matrix, sizeof(double) * 16) == 0);

    return true;
}

VTK_TEST_FN(TestPKUtils036_CircularEdgeReplaceCurve)
{
    // Test that CircularEdgeReplaceCurve will fail when given a bad replacement circle
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_profile_and_closed_edges.xml").c_str(), result );
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(149));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);
    VTKBody wireBody = bodies[0];

    PK_EDGE_t edge = wireBody.FindEntityByIdentifier(EntityClass::Edge, 14);

    Dir dir1 = Dir::XDir();
    Dir ref1 = Dir::YDir();
    Vec pos1(0, 0, 0);
    double radius1 = 1.0;

    PK_CIRCLE_t badCircle = PK_ENTITY_null;
    PK_CIRCLE_t goodCircle = PK_ENTITY_null;

    badCircle =  PKUtils().CreateGeom(
        EntityClass::Circle,
        PK_ENTITY_null,
        NULL,
        &pos1,
        &dir1,
        &radius1,
        NULL,
        NULL,
        &ref1);

    // Invalid edge
    CheckM(!PKUtils().CircularEdgeReplaceCurve(PK_ENTITY_null, badCircle, 1));

    // Use edge values for replacement circle
    PK_GEOM_t edgeGeom = PKUtils().EntityGetGeom(edge);
    Dir newAxis;
    Dir newEdgeRef;
    Vec newPos;
    double newRadius = 0.0;
    PKUtils().CircleData(edgeGeom,newAxis,newEdgeRef,newPos,newRadius);

    goodCircle =  PKUtils().CreateGeom(
        EntityClass::Circle,
        PK_ENTITY_null,
        NULL,
        &newPos,
        &newAxis,
        &newRadius,
        NULL,
        NULL,
        &newEdgeRef);

    // Good replacement
    CheckM(PKUtils().CircularEdgeReplaceCurve(edge, goodCircle, 1));

    // Bad replacement
    CheckM(!PKUtils().CircularEdgeReplaceCurve(edge, badCircle, 1));

    return true;
}

VTK_TEST_FN(TestPKUtils037_DetectScalingTransform)
{
    PK_TRANSF_sf_t transform;
    transform.matrix[0][0] = 1.0;
    transform.matrix[0][1] = 0.0;
    transform.matrix[0][2] = 0.0;
    transform.matrix[0][3] = 0.0;

    transform.matrix[1][0] = 0.0;
    transform.matrix[1][1] = 1.0;
    transform.matrix[1][2] = 0.0;
    transform.matrix[1][3] = 0.0;

    transform.matrix[2][0] = 0.0;
    transform.matrix[2][1] = 0.0;
    transform.matrix[2][2] = 1.0;
    transform.matrix[2][3] = 0.0;

    transform.matrix[3][0] = 0.0;
    transform.matrix[3][1] = 0.0;
    transform.matrix[3][2] = 0.0;
    transform.matrix[3][3] = 1.0;

    CheckM(PKUtils().TransformIsScaling(transform.matrix) ==  false);

    transform.matrix[3][3] = 1.00000000001; // 1e-11
    CheckM(PKUtils().TransformIsScaling(transform.matrix) ==  true);
    return true;
}

VTK_TEST_FN(TestPKUtils038_GenerateTranslationFromScalingTransform01)
{
    double scalingValue = 2.0;
    Vec scaleCentre(1,3,0);
    PK_TRANSF_t scalingTransform = PKUtils().TransformCreateScaling(scalingValue, scaleCentre);
    PK_TRANSF_sf_t transSf;
    PK_TRANSF_ask(scalingTransform,&transSf);

    // Create a plane to workout a translation for
    PK_PLANE_t plane = 0;
    PK_PLANE_sf_t planeSf = {0} ;
    planeSf.basis_set.location.coord[0] = 1.0;
    planeSf.basis_set.location.coord[1] = 1.0;
    planeSf.basis_set.location.coord[2] = 1.0;
    planeSf.basis_set.axis.coord[0] = 0.0;
    planeSf.basis_set.axis.coord[1] = 1.0;
    planeSf.basis_set.axis.coord[2] = 0.0;
    planeSf.basis_set.ref_direction.coord[0] = 1.0;
    planeSf.basis_set.ref_direction.coord[1] = 0.0;
    planeSf.basis_set.ref_direction.coord[2] = 0.0;
    PK_PLANE_create(&planeSf, &plane);
    CheckM(plane != TagNull);

    // Compute the translation equivalent to the scaling transform
    PK_TRANSF_t translationTransform;
    CheckM(PKUtils().TransformCreateTranslationFromScalingTransform(scalingTransform, plane, translationTransform));
    double matrix[4][4];
    CheckM(PKUtils().TransformGetMatrix(translationTransform, matrix));

    // Compare the translation to what we expect
    CheckM(ResEqual(matrix[0][3], 0));
    CheckM(ResEqual(matrix[1][3], -2));
    CheckM(ResEqual(matrix[2][3], 0));
    CheckM(ResEqual(matrix[3][3], 1));

    return true;
}

VTK_TEST_FN(TestPKUtils038_GenerateTranslationFromScalingTransform02)
{
    double scalingValue = 0.5;
    Vec scaleCentre(5,4,0);
    PK_TRANSF_t scalingTransform = PKUtils().TransformCreateScaling(scalingValue, scaleCentre);

    // Create a plane to workout a translation for
    PK_PLANE_t plane = 0;
    PK_PLANE_sf_t planeSf = {0} ;
    planeSf.basis_set.location.coord[0] = 0.0;
    planeSf.basis_set.location.coord[1] = 0.0;
    planeSf.basis_set.location.coord[2] = 0.0;
    Vec planeNormal(1, 1, 0);
    Dir planeDir(planeNormal, false);
    planeSf.basis_set.axis.coord[0] = planeDir.X();
    planeSf.basis_set.axis.coord[1] = planeDir.Y();
    planeSf.basis_set.axis.coord[2] = planeDir.Z();

    Dir refDir = GeomUtils::ComputeOrthogonalDir(planeDir);
    planeSf.basis_set.ref_direction.coord[0] = refDir.X();
    planeSf.basis_set.ref_direction.coord[1] = refDir.Y();
    planeSf.basis_set.ref_direction.coord[2] = refDir.Z();
    PK_PLANE_create(&planeSf, &plane);
    CheckM(plane != TagNull);

    // Compute the translation equivalent to the scaling transform
    PK_TRANSF_t translationTransform;
    CheckM(PKUtils().TransformCreateTranslationFromScalingTransform(scalingTransform, plane, translationTransform));
    double matrix[4][4];
    CheckM(PKUtils().TransformGetMatrix(translationTransform, matrix));

    // Compare the translation to what we expect
    CheckM(ResEqual(matrix[0][3], 2.25));
    CheckM(ResEqual(matrix[1][3], 2.25));
    CheckM(ResEqual(matrix[2][3], 0));
    CheckM(ResEqual(matrix[3][3], 1));

    return true;
}

VTK_TEST_FN(TestPKUtils038_GenerateTranslationFromScalingTransform03)
{
    double scalingValue = 0.5;
    Vec scaleCentre(0,5,4);
    PK_TRANSF_t scalingTransform = PKUtils().TransformCreateScaling(scalingValue, scaleCentre);

    // Create a plane to workout a translation for
    PK_PLANE_t plane = 0;
    PK_PLANE_sf_t planeSf = {0} ;
    planeSf.basis_set.location.coord[0] = 0.0;
    planeSf.basis_set.location.coord[1] = 0.0;
    planeSf.basis_set.location.coord[2] = 0.0;
    Vec planeNormal(0, 1, 1);
    Dir planeDir(planeNormal, false);
    planeSf.basis_set.axis.coord[0] = planeDir.X();
    planeSf.basis_set.axis.coord[1] = planeDir.Y();
    planeSf.basis_set.axis.coord[2] = planeDir.Z();

    Dir refDir = GeomUtils::ComputeOrthogonalDir(planeDir);
    planeSf.basis_set.ref_direction.coord[0] = refDir.X();
    planeSf.basis_set.ref_direction.coord[1] = refDir.Y();
    planeSf.basis_set.ref_direction.coord[2] = refDir.Z();
    PK_PLANE_create(&planeSf, &plane);
    CheckM(plane != TagNull);

    // Compute the translation equivalent to the scaling transform
    PK_TRANSF_t translationTransform;
    CheckM(PKUtils().TransformCreateTranslationFromScalingTransform(scalingTransform, plane, translationTransform));
    double matrix[4][4];
    CheckM(PKUtils().TransformGetMatrix(translationTransform, matrix));

    // Compare the translation to what we expect
    CheckM(ResEqual(matrix[0][3], 0));
    CheckM(ResEqual(matrix[1][3], 2.25));
    CheckM(ResEqual(matrix[2][3], 2.25));
    CheckM(ResEqual(matrix[3][3], 1));

    return true;
}

VTK_TEST_FN(TestPKUtils039_FaceIsSheet01)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/mixed_faces_and_wire_edges_in_general_body.xml").c_str(), result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vbody = bodies.at(0);

    PK_FACE_t face = FaceOfId(vbody, 21);
    CheckTagM(face);

    CheckM(PKUtils().FaceIsSheet(face));
    return true;
}

VTK_TEST_FN(TestPKUtils039_FaceIsSheet02)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/LaminarEdgeControl/SheetBody_with_laminaredges2.xml").c_str(), result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vbody = bodies.at(0);

    PK_FACE_t face = FaceOfId(vbody, 33);
    CheckTagM(face);

    CheckM(PKUtils().FaceIsSheet(face));
    return true;
}

VTK_TEST_FN(TestPKUtils039_FaceIsSheet03)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Find/simple_shell.xml").c_str(), result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vbody = bodies.at(0);

    PK_FACE_t face = FaceOfId(vbody, 35);
    CheckTagM(face);

    CheckM(PKUtils().FaceIsSheet(face) == false);
    return true;
}

VTK_TEST_FN(TestPKUtils040_FaceGetPeriodicRange01)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Feature/surfaceUnders/08_torus_under.xml").c_str(), result);
    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vbody = bodies.at(0);

    PK_FACE_t torusFace = FaceOfId(vbody, 511);
    CheckTagM(torusFace);

    double uRange;
    double vRange;
    PKUtils().FaceGetPeriodicRange(torusFace, uRange, vRange);

    ResEqual(uRange, PI);
    ResEqual(vRange, 2*PI);

    PK_FACE_t planeFace = FaceOfId(vbody, 637);
    CheckTagM(planeFace);

    PKUtils().FaceGetPeriodicRange(planeFace, uRange, vRange);

    ResEqual(uRange, 0);
    ResEqual(vRange, 0);

    return true;
}

int GenerateGridBody(int xCount, int yCount)
{
    // Generates a grid of squares of the input dimensions.  The curves will be 10mm apart in both x and y.
    double spacing = 0.01;

    IntArray lines;
    std::vector<PK_INTERVAL_t> intervals;
    PK_INTERVAL_t interval;
    interval.value[0] = 0.00;
    interval.value[1] = 0.01;

    for (int y = 0; y < yCount; y++)
    {
        for (int x = 0; x < xCount; x++)
        {
            double xPos = x * spacing;
            double yPos = y * spacing;
            PK_LINE_sf_t lineSf;
            PK_LINE_t line = PK_ENTITY_null;

            // X line
            lineSf.basis_set.location.coord[0] = xPos;
            lineSf.basis_set.location.coord[1] = yPos;
            lineSf.basis_set.location.coord[2] = 0.0;
            lineSf.basis_set.axis.coord[0] = 1.0;
            lineSf.basis_set.axis.coord[1] = 0.0;
            lineSf.basis_set.axis.coord[2] = 0.0;
            PK_LINE_create(&lineSf, &line);
            lines.push_back(line);
            intervals.push_back(interval);

            // Y line
            lineSf.basis_set.location.coord[0] = xPos;
            lineSf.basis_set.location.coord[1] = yPos;
            lineSf.basis_set.location.coord[2] = 0.0;
            lineSf.basis_set.axis.coord[0] = 0.0;
            lineSf.basis_set.axis.coord[1] = 1.0;
            lineSf.basis_set.axis.coord[2] = 0.0;
            PK_LINE_create(&lineSf, &line);
            lines.push_back(line);
            intervals.push_back(interval);

            if (x == xCount - 1)
            {
                lineSf.basis_set.location.coord[0] = xPos + spacing;
                lineSf.basis_set.location.coord[1] = yPos;
                lineSf.basis_set.location.coord[2] = 0.0;
                lineSf.basis_set.axis.coord[0] = 0.0;
                lineSf.basis_set.axis.coord[1] = 1.0;
                lineSf.basis_set.axis.coord[2] = 0.0;
                PK_LINE_create(&lineSf, &line);
                lines.push_back(line);
                intervals.push_back(interval);
            }

            if (y == yCount - 1)
            {
                lineSf.basis_set.location.coord[0] = xPos;
                lineSf.basis_set.location.coord[1] = yPos + spacing;
                lineSf.basis_set.location.coord[2] = 0.0;
                lineSf.basis_set.axis.coord[0] = 1.0;
                lineSf.basis_set.axis.coord[1] = 0.0;
                lineSf.basis_set.axis.coord[2] = 0.0;
                PK_LINE_t line = PK_ENTITY_null;
                PK_LINE_create(&lineSf, &line);
                lines.push_back(line);
                intervals.push_back(interval);
            }
        }
    }

    PK_CURVE_make_wire_body_o_t options;
    PK_CURVE_make_wire_body_o_m(options);
    //options.allow_disjoint = PK_LOGICAL_false;
    options.check = PK_LOGICAL_false;

    PK_BODY_t body = PK_ENTITY_null;
    int nEdges = 0;
    PK_EDGE_t* edges = NULL;
    int* edgeIndices = NULL;
    PK_ERROR_code_t error = PK_SESSION_set_general_topology(PK_LOGICAL_true);
    UtilTimer timer;
    error = PK_CURVE_make_wire_body_2((int)lines.size(), &lines.front(), &intervals.front(), &options, &body, &nEdges, &edges, &edgeIndices);
    double createTme = timer.Time();

    //printf("\nTime to create wire containing %i edges = %g seconds\n", lines.size(), createTme);

    return body;
}


VTK_TEST_FN(TestPKUtils041_WireBodyCreation)
{
    int body = GenerateGridBody(10, 10);

    VTK::InteractionTag interaction = CreateInteraction();
    TagArray bodies;

    int nNewBodies = 0;
    int* NewBodies = NULL;
    PK_BODY_disjoin(body, &nNewBodies, &NewBodies);

    for (int i = 0; i < nNewBodies; i++)
    {
        bodies.Add(NewBodies[i]);
    }

    PK_MEMORY_free(NewBodies);

    double planePosition[3] = { 0.0, 0.0, 0.0 };
    double planeNormal[3] = { 0.0, 0.0, 1.0 };
    double planeXDirection[3] = { 1.0, 0.0, 0.0 };

    VTK::ProfileTag profile = CreateProfile(interaction, bodies, planePosition, planeNormal, planeXDirection);
    SetLiveInteraction(interaction);

    return true;
}

VTK_TEST_FN(TestPKUtils042_MakeWirebodyFromCurves1)
{
    //General body option off
    IntArray curves, newEdges;
    DoublePairArray intervals;
    //Input is a 'cross'; we must have 'check' switched off and 'allowdisjoin' switched on in the function for it to work.
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::XDir()));
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::YDir()));
    intervals.push_back(DoublePair(-1, 1));
    intervals.push_back(DoublePair(-1, 1));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, false, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 2);
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }

    return true;
}

VTK_TEST_FN(TestPKUtils042_MakeWirebodyFromCurves2)
{
    //General body option on
    IntArray curves, newEdges;
    DoublePairArray intervals;
    //Input is a 'cross'; we must have 'check' switched off and 'allowdisjoin' switched on in the function for it to work.
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::XDir()));
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::YDir()));
    intervals.push_back(DoublePair(-1, 1));
    intervals.push_back(DoublePair(-1, 1));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, false, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 2);
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }
    VTKBody vbody(wireBody);
    CheckM(!vbody.IsGeneral());
    IntArray vertices;
    vbody.GetVertices(GetCurrentVersion(),&vertices);
    CheckM(vertices.size() == 4);

    return true;
}

VTK_TEST_FN(TestPKUtils042_MakeWirebodyFromCurves3)
{
    //Simple L-shape body with general body option on
    IntArray curves, newEdges;
    DoublePairArray intervals;
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::XDir()));
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::YDir()));
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(0, 1));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, true, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 2);
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }
    VTKBody vbody(wireBody);
    CheckM(!vbody.IsGeneral());
    IntArray vertices;
    vbody.GetVertices(GetCurrentVersion(),&vertices);
    CheckM(vertices.size() == 3);

    return true;
}

VTK_TEST_FN(TestPKUtils042_MakeWirebodyFromCurves4)
{
    //Y-junction with general body on
    IntArray curves, newEdges;
    DoublePairArray intervals;
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::XDir()));
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::YDir()));
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Normal(Vec(-1.0,-1.0,0.0))));
    intervals.push_back(DoublePair(0, 0.1));
    intervals.push_back(DoublePair(0, 0.1));
    intervals.push_back(DoublePair(0, 0.1));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, true, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 3);
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }
    VTKBody vbody(wireBody);
    IntArray vertices;
    vbody.GetVertices(GetCurrentVersion(),&vertices);
    CheckM(vertices.size() == 4);
    CheckM(vbody.IsGeneral());

    return true;
}

VTK_TEST_FN(TestPKUtils042_MakeWirebodyFromCurves5)
{
    //1-curve body with general body option on
    IntArray curves, newEdges;
    DoublePairArray intervals;
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::XDir()));
    intervals.push_back(DoublePair(0, 1));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, true, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 1);
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }
    VTKBody vbody(wireBody);
    CheckM(!vbody.IsGeneral());

    return true;
}

VTK_TEST_FN(TestPKUtils042_MakeWirebodyFromCurves6_ZeroLengthBaseEdge)
{
    //Check we can create a wirebody that contains a zero-length base edge. A root-base edge cannot be zero-length in PGM model,
    //so this can only be an intermediate base curve that is an offset of another curve.
    InteractionTag interaction = CreateInteraction();
    VTK::OptionSetProfileSolverType(interaction, ProfileSolverType2D);

    ConicData conicData(Position(-0.2, 0.1, 0), Position(0.5, 0.0, 0), Position(0.1, -0.4, 0), 0.4);
    int conic = VTK::CreateConic(interaction, conicData);
    CheckTagM(conic);

    OffsetCurveData offsetData1(conic, 0.05, OffsetCurveSideRight);
    int offsetCurve1 = VTK::CreateOffsetCurve(interaction, offsetData1);
    CheckTagM(offsetCurve1);

    OffsetCurveData offsetData2(offsetCurve1, 0.05, OffsetCurveSideRight);
    int offsetCurve2 = VTK::CreateOffsetCurve(interaction, offsetData2);
    CheckTagM(offsetCurve2);

    PKUtils pkUtils;
    IntArray curves, newEdges;
    DoublePairArray intervals;
    curves.push_back(conic);
    curves.push_back(offsetCurve1);
    curves.push_back(offsetCurve2);
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(0.1, 0.1));
    intervals.push_back(DoublePair(0, 1));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, true, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 3);

    //Check the edges use the input curves.
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }
    //Check the second edge is zero-length.
    CheckM(pkUtils.EdgeIsZeroLength(newEdges[1]));
    //Check the offset curve data are correct.
    int baseCurve = 0;
    CheckM(pkUtils.OffsetCurveGetBaseEdgeOrVirtualCurve(newEdges[1], baseCurve));
    CheckM(baseCurve == newEdges[0]);
    CheckM(pkUtils.OffsetCurveGetBaseEdgeOrVirtualCurve(newEdges[2], baseCurve));
    CheckM(baseCurve == newEdges[1]);
    IntArray offsetEnts;
    CheckM(pkUtils.EntityIsBaseCurveOfOffsetCurve(newEdges[0], &offsetEnts));
    CheckM(offsetEnts.size() == 1 && offsetEnts[0] == newEdges[1]);
    offsetEnts.clear();
    CheckM(pkUtils.EntityIsBaseCurveOfOffsetCurve(newEdges[1], &offsetEnts));
    CheckM(offsetEnts.size() == 1 && offsetEnts[0] == newEdges[2]);
    //There should be six vertices in total.
    VTKBody vbody(wireBody);
    IntArray vertices;
    vbody.GetVertices(GetCurrentVersion(),&vertices);
    CheckM(vertices.size() == 6);

    return true;
}

VTK_TEST_FN(TestPKUtils043_SplitWirebodyIntoDisjointBodies)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/uk_flag_profile_15mm_offset.xml").c_str(), result);
    CheckResultM(result);

    CheckM(interaction != NULLPTR);

    Profile* prof = (Profile*)(interaction->FindTaggedEntityById(2));
    CheckM(prof);
    VTKBody vbody = prof->GetWireBodies()[0];
    CheckVTKBodyM(vbody);

    IntArray newBodies;
    bool ok = PKUtils().WirebodySplitIntoDisjointBodies(vbody.GetPKBodyTag(), newBodies);
    CheckM(ok);
    CheckM(newBodies.size() == 9);
    //The input body should be one of the output.
    CheckM(std::find(newBodies.begin(), newBodies.end(), vbody.GetPKBodyTag()) != newBodies.end());

    //None of the output bodies can be split further.
    for (PK_BODY_t body : newBodies)
    {
        IntArray bodies;
        ok = PKUtils().WirebodySplitIntoDisjointBodies(body, bodies);
        CheckM(ok);
        CheckM(bodies.size() == 1);
        CheckM(bodies[0] == body);
    }
    return true;
}

VTK_TEST_FN(TestPKUtils044_CurveHasAtLeastG1Continuity)
{
    //Load a bcurve and keep inserting a knot until its G0
    ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/three_Bcurves.xml").c_str());
    CheckTagM(interaction);
    ProfileTag profile1 = ObjectFindByIdentifier(interaction, 1);
    CheckTagM(profile1);
    TagArray bodies;
    VTK::ProfileGetBodies(interaction, profile1, bodies);
    CheckM(bodies.GetSize() == 1);
    VTKBody wireBody1(bodies[0]);

    for (int i = 1; i <= 3; i++)
    {
        PK_PARTITION_t partition = PKUtils().EntityGetPartition(EdgeOfId(wireBody1, 20));
        PK_CURVE_t bcurveOfEdge = PKUtils().GeomCopy(PKUtils().TopolGetGeom(EdgeOfId(wireBody1, 20)), NULL, &partition);
        CheckTagM(bcurveOfEdge);
        //Insert a new knot repeatedly so the curve is potentially G0-continuous at this knot param.
        CheckM(PKUtils().BcurveInsertKnot(bcurveOfEdge, 0.5, i));

        BcurveDef bcurveData(bcurveOfEdge);
        //Move the first four control points so the bcurve is G0 continuous.
        for (int c = 0; c < 4; c++)
            bcurveData.m_controlPoints[c].x += 0.05;

        bool disc = PKUtils().CurveHasAtLeastG1Continuity(PKUtils().BcurveCreate(bcurveData));
        CheckM(i==3 ? !disc : disc);
    }

    return true;
}

VTK_TEST_FN(TestPKUtils045_EllipseData01)
{
    //Check basic queries:
    //  - EllipseData
    //  - EntityGetClass
    //  - GetGeo
    //  - TopolGetGeomClass
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/intersecting_flipped_and_equal_radius_elliptic_edges.xml").c_str(), result);
    CheckResultM(result);

    Reference* ref1 = (Reference*)(interaction->FindTaggedEntityById(1));
    CheckM(ref1);
    Reference* ref2 = (Reference*)(interaction->FindTaggedEntityById(2));
    CheckM(ref2);

    //EllipseData dataIn1(Position(0.0, 0.0, 0.0), Direction(0, 0, 1), Direction(1, 0, 0), 0.1, 0.2);
    PK_EDGE_t flippedEdge = ref1->GetEntityTag();
    PK_ELLIPSE_t flipped = PKUtils().TopolGetGeom(ref1->GetEntityTag());
    //EllipseData dataIn2(Position(0.1, 0.0, 0.0), Direction(0, 0, 1), Direction(0, 1, 0), 0.1, 0.1);
    PK_EDGE_t circularEdge = ref2->GetEntityTag();
    PK_ELLIPSE_t circular = PKUtils().TopolGetGeom(ref2->GetEntityTag());

    Vec cent;
    Dir axis, refAxis;
    double majorRadius, minorRadius;
    PKUtils().EllipseData(flipped, axis, refAxis, cent, minorRadius, majorRadius);
    CheckM(ResEqual(cent, Vec(0, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::XDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.2));
    PKUtils().EllipseData(circular, axis, refAxis, cent, minorRadius, majorRadius);
    CheckM(ResEqual(cent, Vec(0.1, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::YDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.1));

    CheckM(PKUtils().EntityGetClass(flipped) == EntityClass::Ellipse);
    CheckM(PKUtils().EntityGetClass(circular) == EntityClass::Ellipse);

    CheckM(PKUtils().TopolGetGeomClass(flippedEdge) == EntityClass::Ellipse);
    CheckM(PKUtils().TopolGetGeomClass(circularEdge) == EntityClass::Ellipse);

    CheckM(PKUtils().GetGeo(flipped, 0, 0, &cent, &axis, &majorRadius, 0, &minorRadius, &refAxis) == EntityClass::Ellipse);
    CheckM(ResEqual(cent, Vec(0, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::XDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.2));

    CheckM(PKUtils().GetGeo(circular, 0, 0, &cent, &axis, &majorRadius, 0, &minorRadius, &refAxis) == EntityClass::Ellipse);
    CheckM(ResEqual(cent, Vec(0.1, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::YDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.1));

    return true;
}

VTK_TEST_FN(TestPKUtils045_EllipseData02)
{
    //Check PKUtils methods that require input parameter/interval to be adjusted for flipped ellipses.
    //CurveComputeIntersectionsWithCurve
    //CurveEvaluateAtParam
    //CurveEvaluateAtParamHanded
    //CurveGetTangentDirectionAtPosition
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/intersecting_flipped_ellipse_and_equal_radius_ellipse.xml").c_str(), result);
    CheckResultM(result);

    Reference* ref1 = (Reference*)(interaction->FindTaggedEntityById(2));
    CheckM(ref1);
    Reference* ref2 = (Reference*)(interaction->FindTaggedEntityById(4));
    CheckM(ref2);

    //EllipseData dataIn1(Position(0.0, 0.0, 0.0), Direction(0, 0, 1), Direction(1, 0, 0), 0.1, 0.2);
    PK_ELLIPSE_t flipped = ref1->GetEntityTag();
    //EllipseData dataIn2(Position(0.1, 0.0, 0.0), Direction(0, 0, 1), Direction(0, 1, 0), 0.1, 0.1);
    PK_ELLIPSE_t circular = ref2->GetEntityTag();

    Vec pos;
    VecArray posAndDirs;

    //Check CurveEvaluateAtParam for flipped and equal-radius ellipses. 
    PKUtils().CurveEvaluateAtParam(flipped, 0, &pos);
    CheckM(ResEqual(pos, Vec(0.1, 0, 0)));
    PKUtils().CurveEvaluateAtParam(flipped, 0, 1, posAndDirs);
    CheckM(ResEqual(pos, posAndDirs[0]));
    PKUtils().CurveEvaluateAtParam(circular, 0, &pos);
    CheckM(ResEqual(pos, Vec(0.1, 0.1, 0)));
    PKUtils().CurveEvaluateAtParam(circular, 0, 1, posAndDirs);
    CheckM(ResEqual(pos, posAndDirs[0]));

    //Check CurveEvaluateAtParamHanded for flipped and equal-radius ellipses. 
    PKUtils().CurveEvaluateAtParamHanded(flipped, 0, 1, true, posAndDirs);
    CheckM(ResEqual(Vec(0.1, 0, 0), posAndDirs[0]));
    PKUtils().CurveEvaluateAtParamHanded(circular, 0, 1, true, posAndDirs);
    CheckM(ResEqual(Vec(0.1, 0.1, 0), posAndDirs[0]));

    //Check CurveGetTangentDirectionAtPosition
    Dir tangent;
    PKUtils().CurveGetTangentDirectionAtPosition(flipped, 0, tangent);
    CheckM(tangent.IsAligned(Dir::YDir()));
    PKUtils().CurveGetTangentDirectionAtPosition(circular, 0, tangent);
    CheckM(tangent.IsAligned(-Dir::XDir()));

    //Check CurveComputeIntersectionsWithCurve
    double bounds1[2] = { 0, PI / 2 };
    double bounds2[2] = { 0, PI / 2 };
    VecArray intersections;
    PKUtils().CurveComputeIntersectionsWithCurve(flipped, circular, bounds1, bounds2, 0, 0, &intersections);
    CheckM(intersections.size() == 1);
    CheckM(ResEqual(intersections[0], Vec(0.086851709182132975, 0.099131843766616171, 0)));
    PKUtils().CurveComputeIntersectionsWithCurve(circular, flipped, bounds2, bounds1, 0, 0, &intersections);
    CheckM(intersections.size() == 1);
    CheckM(ResEqual(intersections[0], Vec(0.086851709182132975, 0.099131843766616171, 0)));

    return true;
}

VTK_TEST_FN(TestPKUtils045_EllipseData03)
{
    //Check PKUtils methods that require output parameter/interval to be adjusted for flipped ellipses.
    //CurveGetClosestParamsForPos
    //CurveGetInterval
    //CurveGetParamForPos
    //CurveGetParamRange
    //GeomFindMinimalDistanceToPoint
    //EdgeGetInterval
    //EdgeGetMidPointParam
    //EdgeGetMidPoint
    //EdgeGetTangentDirectionAtPosition

    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/intersecting_flipped_and_equal_radius_elliptic_edges.xml").c_str(), result);
    CheckResultM(result);

    Reference* ref1 = (Reference*)(interaction->FindTaggedEntityById(1));
    CheckM(ref1);
    Reference* ref2 = (Reference*)(interaction->FindTaggedEntityById(2));
    CheckM(ref2);

    //EllipseData dataIn1(Position(0.0, 0.0, 0.0), Direction(0, 0, 1), Direction(1, 0, 0), 0.1, 0.2);
    PK_EDGE_t flippedEdge = ref1->GetEntityTag();
    PK_ELLIPSE_t flipped = PKUtils().TopolGetGeom(ref1->GetEntityTag());
    //EllipseData dataIn2(Position(0.1, 0.0, 0.0), Direction(0, 0, 1), Direction(0, 1, 0), 0.1, 0.1);
    PK_EDGE_t circularEdge = ref2->GetEntityTag();
    PK_ELLIPSE_t circular = PKUtils().TopolGetGeom(ref2->GetEntityTag());

    PK_INTERVAL_t interval;
    PKUtils().CurveGetInterval(flipped, interval);
    CheckM(ResnorZero(interval.value[0]));
    CheckM(ResnorEqual(interval.value[1], TWO_PI));
    PKUtils().CurveGetInterval(circular, interval);
    CheckM(ResnorZero(interval.value[0]));
    CheckM(ResnorEqual(interval.value[1], TWO_PI));

    Vec pos;
    pos = Vec(0.1, 0, 0);
    double param = -1;
    PKUtils().CurveGetClosestParamsForPos(flipped, Vec(0.1, 0, 0), &param);
    CheckM(ResnorEqual(param, 0));
    PKUtils().CurveGetClosestParamsForPos(flipped, Vec(-0.1, 0, 0), &param);
    CheckM(ResnorEqual(param, PI));
    PKUtils().CurveGetClosestParamsForPos(circular, Vec(0.1, 0.1, 0), &param);
    CheckM(ResnorEqual(param, 0));
    PKUtils().CurveGetClosestParamsForPos(circular, Vec(0.1, -0.1, 0), &param);
    CheckM(ResnorEqual(param, PI));

    PKUtils().CurveGetParamForPos(flipped, Vec(0.1, 0, 0), &param);
    CheckM(ResnorEqual(param, 0));
    PKUtils().CurveGetParamForPos(flipped, Vec(-0.1, 0, 0), &param);
    CheckM(ResnorEqual(param, PI));
    PKUtils().CurveGetParamForPos(circular, Vec(0.1, 0.1, 0), &param);
    CheckM(ResnorEqual(param, 0));
    PKUtils().CurveGetParamForPos(circular, Vec(0.1, -0.1, 0), &param);
    CheckM(ResnorEqual(param, PI));

    double range[2] = { -1, -1 };
    bool periodic = false;
    PKUtils().CurveGetParamRange(flipped, range, &periodic);
    CheckM(ResnorZero(range[0]));
    CheckM(ResnorEqual(range[1], TWO_PI));
    CheckM(periodic);
    PKUtils().CurveGetParamRange(circular, range, &periodic);
    CheckM(ResnorZero(range[0]));
    CheckM(ResnorEqual(range[1], TWO_PI));
    CheckM(periodic);

    double dis = -1;
    double params[2];
    pos = Vec(0.11, 0, 0);
    PKUtils().GeomFindMinimalDistanceToPoint(flipped, pos, dis, 0, false, 0, params);
    CheckM(ResEqual(0.01, dis));
    CheckM(ResnorEqual(params[0], 0));
    pos = Vec(-0.11, 0, 0);
    PKUtils().GeomFindMinimalDistanceToPoint(flipped, pos, dis, 0, false, 0, params);
    CheckM(ResEqual(0.01, dis));
    CheckM(ResnorEqual(params[0], PI));
    pos = Vec(0.1, 0.11, 0);
    PKUtils().GeomFindMinimalDistanceToPoint(circular, pos, dis, 0, false, 0, params);
    CheckM(ResEqual(0.01, dis));
    CheckM(ResnorEqual(params[0], 0));
    pos = Vec(0.1, -0.11, 0);
    PKUtils().GeomFindMinimalDistanceToPoint(circular, pos, dis, 0, false, 0, params);
    CheckM(ResEqual(0.01, dis));
    CheckM(ResnorEqual(params[0], PI));

    PKUtils().EdgeGetInterval(flippedEdge, params);
    CheckM(ResnorEqual(params[0], TWO_PI - PI / 3));
    CheckM(ResnorEqual(params[1], TWO_PI + PI / 3));
    PKUtils().EdgeGetInterval(circularEdge, params);
    CheckM(ResnorEqual(params[0], 0));
    CheckM(ResnorEqual(params[1], PI));

    PKUtils().EdgeGetMidPointParam(flippedEdge, param);
    CheckM(ResnorEqual(param, 0));
    PKUtils().EdgeGetMidPointParam(circularEdge, param);
    CheckM(ResnorEqual(param, PI/2));

    PKUtils().EdgeGetMidPoint(flippedEdge, pos);
    CheckM(ResEqual(pos, Vec(0.1, 0, 0)));
    PKUtils().EdgeGetMidPoint(circularEdge, pos);
    CheckM(ResEqual(pos, Vec(0, 0, 0)));

    Dir tangent;
    PKUtils().EdgeGetTangentDirectionAtParameter(flippedEdge, 0, tangent);
    CheckM(tangent.IsAligned(Dir::YDir()));
    PKUtils().EdgeGetTangentDirectionAtParameter(circularEdge, 0, tangent);
    CheckM(tangent.IsAligned(-Dir::XDir()));
    return true;
}

#define ResBetween(v, low, high) (v >= (low-Resabs) && v <=(high+Resabs) )
#define ResnorBetween(v, low, high) (v >= (low-Resnor) && v <=(high+Resnor) )

VTK_TEST_FN(TestPKUtils045_EllipseData04)
{
    //Check input interval and output params are correctly adjusted for 
    //CurveOutputVectors and CurveOutputVectorsWithAngularTolerance

    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/intersecting_flipped_and_equal_radius_elliptic_edges.xml").c_str(), result);
    CheckResultM(result);

    Reference* ref1 = (Reference*)(interaction->FindTaggedEntityById(1));
    CheckM(ref1);
    Reference* ref2 = (Reference*)(interaction->FindTaggedEntityById(2));
    CheckM(ref2);

    //EllipseData dataIn1(Position(0.0, 0.0, 0.0), Direction(0, 0, 1), Direction(1, 0, 0), 0.1, 0.2);
    PK_EDGE_t flippedEdge = ref1->GetEntityTag();
    PK_ELLIPSE_t flipped = PKUtils().TopolGetGeom(ref1->GetEntityTag());
    //EllipseData dataIn2(Position(0.1, 0.0, 0.0), Direction(0, 0, 1), Direction(0, 1, 0), 0.1, 0.1);
    PK_EDGE_t circularEdge = ref2->GetEntityTag();
    PK_ELLIPSE_t circular = PKUtils().TopolGetGeom(ref2->GetEntityTag());

    VecArray pts;
    DoubleVector params;
    //Use the curve's full interval - all points are contained in the ellipse's bounding box
    int n = PKUtils().CurveOutputVectors(flipped, Resabs * 10000, 0.0, 0.0, NULL, pts, &params);
    CheckM(n > 0);
    for (size_t i = 0; i < pts.size(); i++)
    {
        CheckM(ResBetween(pts[i].x, -0.1, 0.1));
        CheckM(ResBetween(pts[i].y, -0.2, 0.2));
        CheckM(ResnorBetween(params[i], 0, TWO_PI));
        //Check the param and position are paired up.
        VecArray vecs;
        PKUtils().CurveEvaluateAtParam(flipped, params[i], 0, vecs);
        CheckM(vecs.size() == 1 && ResEqual(vecs[0], pts[i]));
    }

    //Use a given interval (this is the interval of the edge).
    double interval[2] = { TWO_PI - PI / 3, TWO_PI + PI / 3 };
    pts.clear();
    params.clear();
    n = PKUtils().CurveOutputVectors(flipped, Resabs * 10000, 0.0, 0.0, interval, pts, &params);
    CheckM(n > 0);
    for (size_t i = 0; i < pts.size(); i++)
    {
        CheckM(ResBetween(pts[i].x, 0.05, 0.1));
        CheckM(ResBetween(pts[i].y, -0.1732050807568877, 0.1732050807568877));
        CheckM(ResnorBetween(params[i], TWO_PI - PI, TWO_PI) || ResnorBetween(params[i], 0, PI / 3));
        //Check the param and position are paired up.
        VecArray vecs;
        PKUtils().CurveEvaluateAtParam(flipped, params[i], 0, vecs);
        CheckM(vecs.size() == 1 && ResEqual(vecs[0], pts[i]));
    }

    //Check CurveOutputVectorsWithAngularTolerance
    //Use the curve's full interval - all points are contained in the ellipse's bounding box
    pts.clear();
    params.clear();
    n = PKUtils().CurveOutputVectors(flipped, 0.0, TWO_PI/100, 0.0, NULL, pts, &params);
    CheckM(n > 0);
    for (size_t i = 0; i < pts.size(); i++)
    {
        CheckM(ResBetween(pts[i].x, -0.1, 0.1));
        CheckM(ResBetween(pts[i].y, -0.2, 0.2));
        CheckM(ResnorBetween(params[i], 0, TWO_PI));
        //Check the param and position are paired up.
        VecArray vecs;
        PKUtils().CurveEvaluateAtParam(flipped, params[i], 0, vecs);
        CheckM(vecs.size() == 1 && ResEqual(vecs[0], pts[i]));
    }

    //Use a given interval (this is the interval of the edge).
    pts.clear();
    params.clear();
    n = PKUtils().CurveOutputVectors(flipped, 0.0, TWO_PI / 100, 0.0, interval, pts, &params);
    CheckM(n > 0);
    for (size_t i = 0; i < pts.size(); i++)
    {
        CheckM(ResBetween(pts[i].x, 0.05, 0.1));
        CheckM(ResBetween(pts[i].y, -0.1732050807568877, 0.1732050807568877));
        CheckM(ResnorBetween(params[i], TWO_PI - PI, TWO_PI) || ResnorBetween(params[i], 0, PI / 3));
        //Check the param and position are paired up.
        VecArray vecs;
        PKUtils().CurveEvaluateAtParam(flipped, params[i], 0, vecs);
        CheckM(vecs.size() == 1 && ResEqual(vecs[0], pts[i]));
    }

    return true;
}

VTK_TEST_FN(TestPKUtils045_EllipseData05)
{
    //Test entity copy and delete
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/intersecting_flipped_ellipse_and_equal_radius_ellipse.xml").c_str(), result);
    CheckResultM(result);

    Reference* ref1 = (Reference*)(interaction->FindTaggedEntityById(2));
    CheckM(ref1);
    Reference* ref2 = (Reference*)(interaction->FindTaggedEntityById(4));
    CheckM(ref2);

    //EllipseData dataIn1(Position(0.0, 0.0, 0.0), Direction(0, 0, 1), Direction(1, 0, 0), 0.1, 0.2);
    PK_ELLIPSE_t flipped = ref1->GetEntityTag();
    //EllipseData dataIn2(Position(0.1, 0.0, 0.0), Direction(0, 0, 1), Direction(0, 1, 0), 0.1, 0.1);
    PK_ELLIPSE_t circular = ref2->GetEntityTag();

    Vec cent;
    Dir axis, refAxis;
    double majorRadius, minorRadius;

    //Check EntityCopy copies the curve data attributes
    PK_ELLIPSE_t flippedCopy1 = PKUtils().EntityCopy(flipped);
    PK_ELLIPSE_t circularCopy1 = PKUtils().EntityCopy(circular);
    PKUtils().EllipseData(flippedCopy1, axis, refAxis, cent, minorRadius, majorRadius);
    CheckM(ResEqual(cent, Vec(0, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::XDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.2));
    PKUtils().EllipseData(circularCopy1, axis, refAxis, cent, minorRadius, majorRadius);
    CheckM(ResEqual(cent, Vec(0.1, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::YDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.1));

    //Check GeomCopy copies the curve data attributes
    PK_ELLIPSE_t flippedCopy2 = PKUtils().GeomCopy(flipped);
    PK_ELLIPSE_t circularCopy2 = PKUtils().GeomCopy(circular);
    PKUtils().EllipseData(flippedCopy2, axis, refAxis, cent, minorRadius, majorRadius);
    CheckM(ResEqual(cent, Vec(0, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::XDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.2));
    PKUtils().EllipseData(circularCopy2, axis, refAxis, cent, minorRadius, majorRadius);
    CheckM(ResEqual(cent, Vec(0.1, 0, 0)));
    CheckM(axis.IsAligned(Dir::ZDir()));
    CheckM(refAxis.IsAligned(Dir::YDir()));
    CheckM(ResEqual(majorRadius, 0.1));
    CheckM(ResEqual(minorRadius, 0.1));

    //Check EntityDelete deletes the exclusive owning body
    VTKBody body1 = VTKBody::EntityGetConstructionBody(flipped);
    CheckM(body1 == VTKBody::EntityGetConstructionBody(flippedCopy1));
    CheckM(body1 == VTKBody::EntityGetConstructionBody(flippedCopy2));
    PKUtils().EntityDelete(flippedCopy1);
    PKUtils().EntityDelete(flippedCopy2);
    PKUtils().EntityDelete(flipped);
    CheckM(PKUtils().EntityIs(body1.GetPKBodyTag()) == false);

    VTKBody body2 = VTKBody::EntityGetConstructionBody(circular);
    CheckM(body2 == VTKBody::EntityGetConstructionBody(circularCopy1));
    CheckM(body2 == VTKBody::EntityGetConstructionBody(circularCopy2));
    PKUtils().EntityDelete(circularCopy1);
    PKUtils().EntityDelete(circularCopy2);
    PKUtils().EntityDelete(circular);
    CheckM(PKUtils().EntityIs(body2.GetPKBodyTag()) == false);

    return true;
}

VTK_TEST_FN(TestPKUtils045_EllipseData06)
{
    //Unit test PKUtils::WirebodyMakeFromCurves with labelled ellipses.
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/intersecting_flipped_ellipse_and_equal_radius_ellipse.xml").c_str(), result);
    CheckResultM(result);

    Reference* ref1 = (Reference*)(interaction->FindTaggedEntityById(2));
    CheckM(ref1);
    Reference* ref2 = (Reference*)(interaction->FindTaggedEntityById(4));
    CheckM(ref2);

    //EllipseData dataIn1(Position(0.0, 0.0, 0.0), Direction(0, 0, 1), Direction(1, 0, 0), 0.1, 0.2);
    PK_ELLIPSE_t flipped = ref1->GetEntityTag();
    //EllipseData dataIn2(Position(0.1, 0.0, 0.0), Direction(0, 0, 1), Direction(0, 1, 0), 0.1, 0.1);
    PK_ELLIPSE_t circular = ref2->GetEntityTag();

    VTKBody vbody1 = VTKBody::EntityGetConstructionBody(flipped);
    VTKBody vbody2 = VTKBody::EntityGetConstructionBody(circular);

    //Make the wire-body and check correct portion of the curves are used for the edges.
    IntArray curves, newEdges;
    DoublePairArray intervals;
    curves.push_back(flipped);
    curves.push_back(circular);
    intervals.push_back(DoublePair(-PI/2, PI/2));
    intervals.push_back(DoublePair(-PI/2, PI/2));
    PK_BODY_t wireBody = PKUtils().WirebodyMakeFromCurves(GetCurrentVersion(), curves, intervals, false, NULL, &newEdges);
    CheckTagM(wireBody);
    CheckM(newEdges.size() == 2);
    for (size_t i = 0; i < newEdges.size(); i++)
    {
        CheckM(PKUtils().TopolGetGeom(newEdges[i]) == curves[i]);
    }
    CheckM(PKUtils().EntityIs(vbody1.GetPKBodyTag()) == false);
    CheckM(PKUtils().EntityIs(vbody2.GetPKBodyTag()) == false);
    VecArray points;
    PKUtils().EdgeGetEndPoints(newEdges[0], points);
    CheckM(ResEqual(points[0].x, 0));
    CheckM(ResEqual(points[1].x, 0));
    CheckM(ResEqual(points[0].y, -0.2));
    CheckM(ResEqual(points[1].y, 0.2));
    PKUtils().EdgeGetMidPoint(newEdges[0], points[0]);
    CheckM(ResEqual(points[0].x, 0.1));
    CheckM(ResEqual(points[0].y, 0));

    PKUtils().EdgeGetEndPoints(newEdges[1], points);
    CheckM(ResEqual(points[0].x, 0.2));
    CheckM(ResEqual(points[1].x, 0.0));
    CheckM(ResEqual(points[0].y, 0));
    CheckM(ResEqual(points[1].y, 0));
    PKUtils().EdgeGetMidPoint(newEdges[1], points[0]);
    CheckM(ResEqual(points[0].x, 0.1));
    CheckM(ResEqual(points[0].y, 0.1));

    return true;
}

VTK_TEST_FN(TestPKUtils046_CylinderRangeAndMidpoint_bad_inputs)
{
    // check that the function fails correctly with bad inputs

    ResultType result = ResultTypeOk;
    VTKBody vbody = TestUtilLoadSingleBody("vtk_part_data:Find/Cyl_Canted.x_t");
    CheckVTKBodyM(vbody);

    PK_FACE_t cylinder = FaceOfId(vbody, 161);
    PK_FACE_t plane = FaceOfId(vbody, 71);

    // Valid inputs
    Vec pointA(0.0519296914780194, -0.0342221125807619, 0);
    Vec pointB(0.02682154840200155, -0.0522831591304638, 0);
    Vec expectedMidPoint(0.044003874018814490, -0.049686751690649650, 0.00000000000000000);
    Vec midPoint;
    CheckM(PKUtils().CylinderGetMidPoint(cylinder, pointA, pointB, midPoint));
    CheckM(midPoint == expectedMidPoint);
    // reverse the point but expect the same result
    Vec reverseMidPoint;
    CheckM(PKUtils().CylinderGetMidPoint(cylinder, pointB, pointA, reverseMidPoint));
    CheckM(midPoint == reverseMidPoint);


    // Not a cylinder
    CheckM(PKUtils().CylinderGetMidPoint(plane, pointA, pointB, midPoint) == false);

    // Points not on geom
    Vec badPointA(pointA);
    badPointA.x += 0.01;
    CheckM(PKUtils().CylinderGetMidPoint(cylinder, badPointA, pointB, midPoint) == false);
    Vec badPointB(pointB);
    badPointB.x += 0.01;
    CheckM(PKUtils().CylinderGetMidPoint(cylinder, pointA, badPointB, midPoint) == false);


    // valid input 
    double expectedRange = 1.8944290214670825;
    double range = 0.0;
    CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(cylinder, pointA, pointB, range));
    CheckM(ResnorEqual(range, expectedRange));
    // reverse the points and expect the same angle
    double reversedRange = 0.0;
    CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(cylinder, pointB, pointA, reversedRange));
    CheckM(ResnorEqual(range, reversedRange));

    // Not a cylinder
    CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(plane, pointA, pointB, range) == false);

    // Points not on geom
    CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(cylinder, badPointA, pointB, range) == false);
    CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(cylinder, pointA, badPointB, range) == false);

    return true;
}

VTK_TEST_FN(TestPKUtils046_CylinderRangeAndMidpoint_points_across_cut)
{
    // check that the function works when points are across a cut, the mid point and range should be on the other side of the cut

    ResultType result = ResultTypeOk;
    VTKBody vbody = TestUtilLoadSingleBody("vtk_part_data:Misc/incomplete_cylinder.x_t");
    CheckVTKBodyM(vbody);

    PK_FACE_t cylinder = FaceOfId(vbody, 191);

    // Valid inputs
    Vec pointA(0.04987838536578432, 0.01901548379625803, 0.1000000000000226);
    Vec pointB(0.05507669759717528, 0.01901548379625803, 0.06000000000002269);
    Vec expectedMidPoint(0.052477541481479803, 0.038671797144852450, 0.080000000000022636);
    Vec midPoint;
    CheckM(PKUtils().CylinderGetMidPoint(cylinder, pointA, pointB, midPoint));
    CheckM(midPoint == expectedMidPoint);

    double range = 0.0;
    double expectedRange = 5.7573156873706628;
    CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(cylinder, pointA, pointB, range));
    CheckM(ResnorEqual(range, expectedRange));


    // take the same points on the other complete cylinder and check that the mid point is now on the small side
    {
        PK_FACE_t cylinder = FaceOfId(vbody, 148);
        Vec pointA(0.05025808898703322, 0.06181712916806808, 0.1000000000000226);
        Vec pointB(0.05545640121842418, 0.06181712916806808, 0.06000000000002269);
        Vec expectedMidPoint(0.052857245102728698, 0.061473442516662501, 0.080000000000022636);
        Vec midPoint;
        CheckM(PKUtils().CylinderGetMidPoint(cylinder, pointA, pointB, midPoint));
        CheckM(midPoint == expectedMidPoint);

        double range = 0.0;
        CheckM(PKUtils().CylinderGetFaceAngleBetweenPoints(cylinder, pointA, pointB, range));
        CheckM(ResnorEqual(range, TWO_PI - expectedRange));
    }

    return true;
}

VTK_TEST_FN(TestPKUtils047_InterpolatedBcurve01)
{
    //Test BcurveCreate, BcurveGet, and BcurveIsInterpolationBased
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/various_interpolated_bcurves.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 2);

    PK_BCURVE_t localBcurve = CurveOfId(bodies[0], 5);
    CheckTagM(localBcurve);
    BcurveDef bcurveData(localBcurve);

    IntArray geoms;
    VTKBody newBody = VTKBody::MakeMinimumBody(PKUtils().CreatePoint(Vec::Origin()));

    //Make an interpolated bcurve without supplying a body.
    PK_CURVE_t bcurve1 = PKUtils().BcurveCreate(bcurveData);
    VTKBody bcurve1Body = VTKBody::EntityGetConstructionBody(bcurve1);
    CheckM(bcurve1Body.GetType() == PK_BODY_type_minimum_c);
    bcurve1Body.GetGeoms(VTK::GetVersion(interaction), geoms);
    CheckM(geoms.size() == 1 && geoms[0] == bcurve1);

    //Make another in the given body.
    PK_CURVE_t bcurve2 = PKUtils().BcurveCreate(bcurveData, &newBody);
    CheckM(VTKBody::EntityGetConstructionBody(bcurve2) == newBody);

    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve1));
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve2));

    BcurveDef bcurveDataOut;
    CheckM(PKUtils().BcurveGet(bcurve1, bcurveDataOut));
    CheckM(bcurveDataOut == bcurveData);

    return true;
}

bool s_CheckPointsAreTransformed(const VecArray& before, const VecArray& after, const Xform& xform)
{
    CheckM(before.size() == after.size());

    for (size_t i = 0; i < before.size(); i++)
    {
        Vec pt = before[i];
        xform.Apply(&pt);
        CheckM(ResEqual(pt, after[i]));
    }

    return true;
}

bool s_CheckDerivativesAreTransformed(const IntToDirMap& before, const IntToDirMap& after, const Xform& xform)
{
    CheckM(before.size() == after.size());

    IntToDirMapConstIter it1 = before.begin();
    IntToDirMapConstIter it2 = after.begin();
    for (; it1 != before.end() && it2 != after.end(); it1++, it2++)
    {
        CheckM(it1->first == it2->first);
        Dir dir = it1->second;
        xform.Apply(&dir);

        CheckM(dir.IsAligned(it2->second));
    }

    return true;
}

bool s_CheckDerivativesAreTransformed(const IntToVecMap& before, const IntToVecMap& after, const Xform& xform)
{
    CheckM(before.size() == after.size());

    IntToVecMapConstIter it1 = before.begin();
    IntToVecMapConstIter it2 = after.begin();
    for (; it1 != before.end() && it2 != after.end(); it1++, it2++)
    {
        CheckM(it1->first == it2->first);
        Vec pt = it1->second;
        double mag = pt.Mag();
        Dir dir = Normal(pt);
        xform.Apply(&dir);

        CheckM(ResEqual(mag, it2->second.Mag()));
        CheckM(dir.IsAligned(Normal(it2->second)));
    }

    return true;
}

VTK_TEST_FN(TestPKUtils047_InterpolatedBcurve02)
{
    //Test GeomTransform (note there are four overloaded GeomTransform methods)
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/various_interpolated_bcurves.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 2);

    PK_BCURVE_t localBcurve = CurveOfId(bodies[0], 5);
    CheckTagM(localBcurve);
    BcurveDef bcurveData(localBcurve);

    Xform xform;
    xform.SetTranslationOnly(Vec(0.1, 0.1, 0));
    PK_TRANSF_t transf = PKUtils().TransformCreateTranslation(Vec(0.1, 0.1, 0));

    //Single-geom in-place transform
    //bool             GeomTransform(PK_GEOM_t geom, PK_TRANSF_t transf);
    PK_BCURVE_t bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    CheckM(PKUtils().GeomTransform(bcurve, transf));
    BcurveDef bcurveDataTransformed(bcurve);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));

    //Single-geom transform to copy
    //bool             GeomTransform(PK_GEOM_t geomIn, const Xform& transform, PK_GEOM_t& geomOut);
    bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    PK_BCURVE_t bcurveOut = 0;
    CheckM(PKUtils().GeomTransform(bcurve, transf, bcurveOut));
    bcurveDataTransformed = BcurveDef(bcurveOut);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));

    //Multi-geom transform to copy
    //bool             GeomTransform(const IntArray& geomsIn, PK_TRANSF_t pkTransform, IntArray& geomsOut);
    IntArray geomsIn, geomsOut;
    bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    geomsIn.push_back(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    CheckM(PKUtils().GeomTransform(geomsIn, transf, geomsOut));
    bcurveDataTransformed = BcurveDef(geomsOut[0]);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));

    //Multi-geom in-place transform
    //bool             GeomTransform(const IntArray& geoms, PK_TRANSF_t transf, PK_GEOM_transform_o_t* options = NULL);
    CheckM(PKUtils().BcurveIsInterpolationBased(geomsIn[0]));
    CheckM(PKUtils().GeomTransform(geomsIn, transf));
    bcurveDataTransformed = BcurveDef(geomsIn[0]);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));
    return true;
}

VTK_TEST_FN(TestPKUtils047_InterpolatedBcurve03)
{
    //Test EntityTransformed
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/interpolation_bcurve_deg3.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);

    PK_EDGE_t bcurveEdge = EdgeOfId(bodies[0], 6);
    CheckTagM(bcurveEdge);

    BcurveDef bcurveData(bcurveEdge);
    CheckM(bcurveData.IsInitialised() && bcurveData.IsInterpolationBcurve());

    Xform xform;
    xform.SetTranslationOnly(Vec(0.1, 0.1, 0));
    PK_TRANSF_t transf = PKUtils().TransformCreateTranslation(Vec(0.1, 0.1, 0));

    //Use raw PK function to transform the body.
    PK_TOPOL_track_r_t trackingLocal = { 0 };
    PK_TOPOL_local_r_t resultsLocal = { 0 };
    PK_BODY_transform_o_t optionsLocal = { 0 };
    PK_BODY_transform_o_m(optionsLocal);
    PK_ERROR_t error = PK_BODY_transform_2(bodies[0].GetPKBodyTag(), transf, Resabs, &optionsLocal, &trackingLocal, &resultsLocal);
    PK_TOPOL_track_r_f(&trackingLocal);
    PK_TOPOL_local_r_f(&resultsLocal);
    CheckM(error == PK_ERROR_no_errors);
    CheckM(PKUtils().EntityTransformCurveDataAttributes(bcurveEdge, transf));
    BcurveDef bcurveDataTransformed(bcurveEdge);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));

    return true;
}

VTK_TEST_FN(TestPKUtils048_PointIsAtEndOfEdgeCurve01)
{
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/AllCurveTypes.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);
    
    PK_EDGE_t lineEdge = EdgeOfId(bodies[0], 78);
    CheckTagM(lineEdge);
    PK_EDGE_t circEdge = EdgeOfId(bodies[0], 21);
    CheckTagM(circEdge);
    PK_EDGE_t elliEdge = EdgeOfId(bodies[0], 16);
    CheckTagM(elliEdge);
    PK_EDGE_t splineEdge = EdgeOfId(bodies[0], 36);
    CheckTagM(splineEdge);
    PK_EDGE_t interpSplineEdge = EdgeOfId(bodies[0], 96);
    CheckTagM(interpSplineEdge);
    PK_EDGE_t conicEdge = EdgeOfId(bodies[0], 6);
    CheckTagM(conicEdge);

    PK_VERTEX_t vert1 = VertexOfId(bodies[0], 35); //between line and spline
    CheckTagM(vert1);
    PK_VERTEX_t vert2 = VertexOfId(bodies[0], 20); // between circ and interpolated spline
    CheckTagM(vert2);
    PK_VERTEX_t vert3 = VertexOfId(bodies[0], 5); // between ellipse and conic
    CheckTagM(vert3);

    Vec pt1 = PKUtils().VertexGetPoint(vert1);
    Vec pt2 = PKUtils().VertexGetPoint(vert2);
    Vec pt3 = PKUtils().VertexGetPoint(vert3);

    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt1, lineEdge) == false);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt1, splineEdge) == true);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt2, circEdge) == false);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt2, interpSplineEdge) == true);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt3, elliEdge) == false);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt3, conicEdge) == true);

    //Some points on the bcurve or conic edge but not at the end.
    Vec pt;
    PKUtils().EdgeGetMidPoint(splineEdge, pt);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt, splineEdge) == false);
    PKUtils().EdgeGetMidPoint(interpSplineEdge, pt);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt, interpSplineEdge) == false);
    PKUtils().EdgeGetMidPoint(conicEdge, pt);
    CheckM(PKUtils().PointIsAtEndOfEdgeCurve(pt, conicEdge) == false);

    return true;
}

VTK_TEST_FN(TestPKUtils048_VertexIsAtEndOfEdgeCurve01)
{
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/AllCurveTypes.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);

    PK_EDGE_t lineEdge = EdgeOfId(bodies[0], 78);
    CheckTagM(lineEdge);
    PK_EDGE_t circEdge = EdgeOfId(bodies[0], 21);
    CheckTagM(circEdge);
    PK_EDGE_t elliEdge = EdgeOfId(bodies[0], 16);
    CheckTagM(elliEdge);
    PK_EDGE_t splineEdge = EdgeOfId(bodies[0], 36);
    CheckTagM(splineEdge);
    PK_EDGE_t interpSplineEdge = EdgeOfId(bodies[0], 96);
    CheckTagM(interpSplineEdge);
    PK_EDGE_t conicEdge = EdgeOfId(bodies[0], 6);
    CheckTagM(conicEdge);

    PK_VERTEX_t vert1 = VertexOfId(bodies[0], 35); //between line and spline
    CheckTagM(vert1);
    PK_VERTEX_t vert2 = VertexOfId(bodies[0], 20); // between circ and interpolated spline
    CheckTagM(vert2);
    PK_VERTEX_t vert3 = VertexOfId(bodies[0], 5); // between ellipse and conic
    CheckTagM(vert3);

    CheckM(PKUtils().VertexIsAtEndOfEdgeCurve(vert1, lineEdge) == false);
    CheckM(PKUtils().VertexIsAtEndOfEdgeCurve(vert1, splineEdge) == true);
    CheckM(PKUtils().VertexIsAtEndOfEdgeCurve(vert2, circEdge) == false);
    CheckM(PKUtils().VertexIsAtEndOfEdgeCurve(vert2, interpSplineEdge) == true);
    CheckM(PKUtils().VertexIsAtEndOfEdgeCurve(vert3, elliEdge) == false);
    CheckM(PKUtils().VertexIsAtEndOfEdgeCurve(vert3, conicEdge) == true);

    return true;
}

bool s_CompareInterpConditions(const BcurveDef& lhs, const BcurveDef& rhs)
{
    bool result = true;
    if (lhs.m_1stDerivDirs.size() != rhs.m_1stDerivDirs.size() ||
        lhs.m_1stDerivMags.size() != rhs.m_1stDerivMags.size() ||
        lhs.m_2ndDerivs.size() != rhs.m_2ndDerivs.size())
    {
        result = false;
    }

    if (result)
    {
        result = lhs.m_interpolationData == rhs.m_interpolationData;
    }

    if (result)
    {
        //Compare first derivative directions.
        for (IntToDirMapConstIter it1 = lhs.m_1stDerivDirs.begin(), it2 = rhs.m_1stDerivDirs.begin();
        it1 != lhs.m_1stDerivDirs.end() && it2 != rhs.m_1stDerivDirs.end() && result; it1++, it2++)
        {
            result = it1->first == it2->first && it1->second.IsAligned(it2->second);
        }
        //Compare first derivative magnitudes.
        for (IntToDoubleMapConstIter it1 = lhs.m_1stDerivMags.begin(), it2 = rhs.m_1stDerivMags.begin();
        it1 != lhs.m_1stDerivMags.end() && it2 != rhs.m_1stDerivMags.end() && result; it1++, it2++)
        {
            result = it1->first == it2->first && ResEqual(it1->second, it2->second);
        }

        //Compare second derivatives.
        for (IntToVecMapConstIter it1 = lhs.m_2ndDerivs.begin(), it2 = rhs.m_2ndDerivs.begin();
        it1 != lhs.m_2ndDerivs.end() && it2 != rhs.m_2ndDerivs.end() && result; it1++, it2++)
        {
            result = it1->first == it2->first && ResEqual(it1->second, it2->second);
        }
    }
    return result;
}

VTK_TEST_FN(TestPKUtils047_InterpolatedBcurveWithDers01)
{
    //Interpolation data for this edge is:
    //  0.167 0.1280796357856054 0
    //  0.1809432069256405 0.1166853418591691 0
    //  0.1941507282943724 0.1046570638343454 0; der1=0.05 0 0; der2=0 0.2 0
    //  0.2013441104684139 0.1178645848027793 0
    //  0.2230421812884734 0.1281239984121877 0
    //  0.2455657221940786 0.1080768683708149 0; der2=0 1.5 0
    //  0.2553534389226924 0.1310721057712131 0; der1=0 0.2 0
    //  0.266 0.1280796357856054 0

    BcurveDef interpDataToCheck;
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.167, 0.1280796357856054, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.1809432069256405, 0.1166853418591691, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.1941507282943724, 0.1046570638343454, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.2013441104684139, 0.1178645848027793, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.2230421812884734, 0.1281239984121877, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.2455657221940786, 0.1080768683708149, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.2553534389226924, 0.1310721057712131, 0));
    interpDataToCheck.m_interpolationData.m_interpolationPoints.push_back(Vec(0.266, 0.1280796357856054, 0));
    interpDataToCheck.m_1stDerivDirs[2] = Normal(1, 0, 0);
    interpDataToCheck.m_1stDerivMags[2] = 0.05;
    interpDataToCheck.m_1stDerivDirs[6] = Normal(0, 1, 0);
    interpDataToCheck.m_1stDerivMags[6] = 0.2;
    interpDataToCheck.m_2ndDerivs[2] = Vec(0, 0.2, 0);
    interpDataToCheck.m_2ndDerivs[5] = Vec(0, 1.5, 0);
    GeomUtils::ComputeChordLengthParameterisation(interpDataToCheck.m_interpolationData.m_interpolationPoints, false, NULL, interpDataToCheck.m_interpolationData.m_interpolationParams);

    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/interpolated_bcurve_with_ders_01.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);

    PK_EDGE_t interpBcurveEdge = EdgeOfId(bodies[0], 6);
    CheckTagM(interpBcurveEdge);
    BcurveDef bcurveData(interpBcurveEdge);
    CheckM(s_CompareInterpConditions(bcurveData, interpDataToCheck));

    IntArray geoms;
    VTKBody newBody = VTKBody::MakeMinimumBody(PKUtils().CreatePoint(Vec::Origin()));

    //Make an interpolated bcurve without supplying a body.
    PK_CURVE_t bcurve1 = PKUtils().BcurveCreate(bcurveData);
    VTKBody bcurve1Body = VTKBody::EntityGetConstructionBody(bcurve1);
    CheckM(bcurve1Body.GetType() == PK_BODY_type_minimum_c);
    bcurve1Body.GetGeoms(VTK::GetVersion(interaction), geoms);
    CheckM(geoms.size() == 1 && geoms[0] == bcurve1);

    //Make another in the given body.
    PK_CURVE_t bcurve2 = PKUtils().BcurveCreate(bcurveData, &newBody);
    CheckM(VTKBody::EntityGetConstructionBody(bcurve2) == newBody);

    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve1));
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve2));

    BcurveDef bcurveDataOut;
    CheckM(PKUtils().BcurveGet(bcurve1, bcurveDataOut));
    CheckM(bcurveDataOut == bcurveData);
    CheckM(s_CompareInterpConditions(bcurveData, interpDataToCheck));

    CheckM(PKUtils().BcurveGet(bcurve2, bcurveDataOut));
    CheckM(bcurveDataOut == bcurveData);
    CheckM(s_CompareInterpConditions(bcurveData, interpDataToCheck));
    return true;
}

VTK_TEST_FN(TestPKUtils047_InterpolatedBcurveWithDers02)
{
    //Test GeomTransform (note there are four overloaded GeomTransform methods)
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/interpolated_bcurve_with_ders_01.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);

    PK_EDGE_t interpBcurveEdge = EdgeOfId(bodies[0], 6);
    CheckTagM(interpBcurveEdge);
    BcurveDef bcurveData(interpBcurveEdge);
    //CheckM(bcurveData.m_interpolationData == interpDataToCheck);

    //It is important to rotate about a point that is not the origin, as the rotation of derivatives should be translation-free.
    Xform xform;
    xform.SetRotation(Vec(0.1, 0.1, 0), Dir::XDir(), Dir::YDir());
    PK_TRANSF_sf_t transf_t;
    xform.GetTransform(transf_t.matrix);
    PK_TRANSF_t transf = PKUtils().TransformCreate(transf_t);
    CheckTagM(transf);

    //Single-geom in-place transform
    //bool             GeomTransform(PK_GEOM_t geom, PK_TRANSF_t transf);
    PK_BCURVE_t bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    CheckM(PKUtils().GeomTransform(bcurve, transf));
    BcurveDef bcurveDataTransformed(bcurve);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));
    //CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_interpolationData.m_interpolation1stDerivDirs, bcurveDataTransformed.m_interpolationData.m_interpolation1stDerivDirs, xform));
    CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_2ndDerivs, bcurveDataTransformed.m_2ndDerivs, xform));

    //Single-geom transform to copy
    //bool             GeomTransform(PK_GEOM_t geomIn, const Xform& transform, PK_GEOM_t& geomOut);
    bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    PK_BCURVE_t bcurveOut = 0;
    CheckM(PKUtils().GeomTransform(bcurve, transf, bcurveOut));
    bcurveDataTransformed = BcurveDef(bcurveOut);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));
    //CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_interpolationData.m_interpolation1stDerivDirs, bcurveDataTransformed.m_interpolationData.m_interpolation1stDerivDirs, xform));
    CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_2ndDerivs, bcurveDataTransformed.m_2ndDerivs, xform));

    //Multi-geom transform to copy
    //bool             GeomTransform(const IntArray& geomsIn, PK_TRANSF_t pkTransform, IntArray& geomsOut);
    IntArray geomsIn, geomsOut;
    bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    geomsIn.push_back(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    CheckM(PKUtils().GeomTransform(geomsIn, transf, geomsOut));
    bcurveDataTransformed = BcurveDef(geomsOut[0]);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));
    //CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_interpolationData.m_interpolation1stDerivDirs, bcurveDataTransformed.m_interpolationData.m_interpolation1stDerivDirs, xform));
    CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_2ndDerivs, bcurveDataTransformed.m_2ndDerivs, xform));

    //Multi-geom in-place transform
    //bool             GeomTransform(const IntArray& geoms, PK_TRANSF_t transf, PK_GEOM_transform_o_t* options = NULL);
    CheckM(PKUtils().GeomTransform(geomsIn, transf));
    bcurveDataTransformed = BcurveDef(geomsIn[0]);
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_controlPoints, bcurveDataTransformed.m_controlPoints, xform));
    CheckM(s_CheckPointsAreTransformed(bcurveData.m_interpolationData.m_interpolationPoints, bcurveDataTransformed.m_interpolationData.m_interpolationPoints, xform));
    //CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_interpolationData.m_interpolation1stDerivDirs, bcurveDataTransformed.m_interpolationData.m_interpolation1stDerivDirs, xform));
    CheckM(s_CheckDerivativesAreTransformed(bcurveData.m_2ndDerivs, bcurveDataTransformed.m_2ndDerivs, xform));

    return true;
}

bool s_CheckDerivativesAreSatisfiedOnCurve(const BcurveDef& bcurveData, PK_CURVE_t curve)
{
    for (int i = 0; i < (int)bcurveData.m_interpolationData.m_interpolationParams.size(); i++)
    {
        double param = bcurveData.m_interpolationData.m_interpolationParams[i];
        VecArray vecs;
        PKUtils().CurveEvaluateAtParam(curve, param, 2, vecs);
        double mag = vecs[1].Mag();
        Dir dir = Normal(vecs[1]);
        IntToDirMapConstIter it1 = bcurveData.m_1stDerivDirs.find(i);
        if (it1 != bcurveData.m_1stDerivDirs.end())
        {
            CheckM(dir.IsAligned(it1->second));
        }
        IntToDoubleMapConstIter it2 = bcurveData.m_1stDerivMags.find(i);
        if (it2 != bcurveData.m_1stDerivMags.end())
        {
            CheckM(ResEqual(mag, it2->second));
        }
        IntToVecMapConstIter it3 = bcurveData.m_2ndDerivs.find(i);
        if (it3 != bcurveData.m_2ndDerivs.end())
        {
            CheckM(ResEqual(vecs[2].Mag(), it3->second.Mag()));
            Dir dir2 = Normal(vecs[2]);
            CheckM(dir2.IsAligned(Normal(it3->second)));
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils047_InterpolatedBcurveWithDers03)
{
    //Test GeomTransform with scaling transform, especially for 1st der magnitude
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/interpolated_bcurve_with_ders_01.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);

    PK_EDGE_t interpBcurveEdge = EdgeOfId(bodies[0], 6);
    CheckTagM(interpBcurveEdge);
    BcurveDef bcurveData(interpBcurveEdge);
    CheckM(s_CheckDerivativesAreSatisfiedOnCurve(bcurveData, PKUtils().EdgeGetCurve(interpBcurveEdge)));

    Xform xform;
    PK_TRANSF_sf_t transf_t;
    xform.GetTransform(transf_t.matrix);
    transf_t.matrix[3][3] = 1.5;
    PK_TRANSF_t transf = PKUtils().TransformCreate(transf_t);
    CheckTagM(transf);

    PK_BCURVE_t bcurve = PKUtils().BcurveCreate(bcurveData);
    CheckTagM(bcurve);
    CheckM(PKUtils().BcurveIsInterpolationBased(bcurve));
    CheckM(PKUtils().GeomTransform(bcurve, transf));

    BcurveDef bcurveData2(bcurve);
    CheckM(bcurveData2.m_1stDerivDirs.size() == 2);
    CheckM(bcurveData2.m_1stDerivMags.size() == 2);
    CheckM(bcurveData2.m_2ndDerivs.size() == 2);
    CheckM(s_CheckDerivativesAreSatisfiedOnCurve(bcurveData2, bcurve));

    return true;
}

VTK_TEST_FN(TestPKUtils048_PerformanceTest_FindNABox)
{
    // Set to true to see time taken:
    bool displayTime = false;



    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Misc/80ToothCog.xml").c_str(), &result);

    VTK::Tag instanceTag = ObjectFindByIdentifier(interaction, 11);
    PK_BODY_t pkBody = InstanceGetBody(interaction, instanceTag);
    VTKBody body(pkBody);

    IntSet faceSet;
    body.GetFaces(Version_17_00, faceSet);

    PK_TOPOL_t topol = FaceOfId(body, 1709);
    
    clock_t startTime = clock();

    for (int i = 0; i < 10000; i++)
    {
        PK_TRANSF_t* transform = NULLPTR;
        PK_TOPOL_find_nabox_o_t options;
        PK_TOPOL_find_nabox_o_m(options);
        PK_NABOX_sf_t naBox;

        ParasolidJournalUtils::StartParasolidJournal("TestPKUtils048_PerformanceTest_FindNABox", true);
        PK_ERROR_t error = PK_TOPOL_find_nabox(1, &topol, transform, &options, &naBox);
        ParasolidJournalUtils::StopParasolidJournal();
    }

    clock_t stopTime = clock();
    double elapsedTime = (double)(stopTime - startTime) / CLOCKS_PER_SEC;

    int faceID = PKUtils().EntityGetIdentifier(topol);

    if (displayTime)
    {
        printf("Time of finding for face ID (%d):    %lf\n", faceID, elapsedTime);
    }

    return true;
}

VTK_TEST_FN(TestPKUtils049)
{
    //Test composing a rotation followed by a scaling and see what we get

    //Rotate by PI/2 around (2,1,0)
    Vec pos1(2.0,1.0,0.0);
    Xform xform;
    xform.SetRotationRightHanded(pos1,Dir::ZDir(),1.0,0.0);
    PK_TRANSF_sf_t sf1;
    xform.GetTransform(sf1.matrix);
    int pkTrans1 = PKUtils().TransformCreate(sf1);

    //Scale by 3 around (1,2,0)
    Vec pos2(1.0,2.0,0.0);
    int pkTrans2 = PKUtils().TransformCreateScaling(3.0,pos2);
    PK_TRANSF_sf_t sf2;
    PK_TRANSF_ask(pkTrans2,&sf2);

    //Multiply
    int mult = 0;
    CheckM(PK_TRANSF_transform(pkTrans1,pkTrans2,&mult) == PK_ERROR_no_errors);

    PK_TRANSF_sf_t sf3;
    PK_TRANSF_ask(mult,&sf3);

    return true;
}

VTK_TEST_FN(TestEllipseDataUnit001)
{
    EllipseDef ellipseUnit1;

    CheckM(ResEqual(ellipseUnit1.GetPos(), Vec()));
    CheckM(ResEqual(ellipseUnit1.GetAxis().GetVec(), Vec()));
    CheckM(ResEqual(ellipseUnit1.GetMajorAxis().GetVec(), Vec()));
    CheckM(ResEqual(ellipseUnit1.GetMajorRadius(), 0.0));
    CheckM(ResEqual(ellipseUnit1.GetMinorRadius(), 0.0));

    Dir tempAx = Normal(Vec(0.0, 0.0, 1.0));
    Dir tempMajAx = Normal(Vec(1.0, 0.0, 0.0));
    Vec tempPos(1.0, 1.0, 0.0);
    ellipseUnit1.SetAllData(tempAx, tempMajAx, tempPos, 1.2, 3.4);

    CheckM(ResEqual(ellipseUnit1.GetPos(), Vec(1.0, 1.0, 0.0)));
    CheckM(ResEqual(ellipseUnit1.GetAxis().GetVec(), Vec(0.0, 0.0, 1.0)));
    CheckM(ResEqual(ellipseUnit1.GetMajorAxis().GetVec(), Vec(1.0, 0.0, 0.0)));
    CheckM(ResEqual(ellipseUnit1.GetMajorRadius(), 3.4));
    CheckM(ResEqual(ellipseUnit1.GetMinorRadius(), 1.2));

    EllipseDef ellipseUnit2(ellipseUnit1);

    CheckM(ResEqual(ellipseUnit2.GetPos(), Vec(1.0, 1.0, 0.0)));
    CheckM(ResEqual(ellipseUnit2.GetAxis().GetVec(), Vec(0.0, 0.0, 1.0)));
    CheckM(ResEqual(ellipseUnit2.GetMajorAxis().GetVec(), Vec(1.0, 0.0, 0.0)));
    CheckM(ResEqual(ellipseUnit2.GetMajorRadius(), 3.4));
    CheckM(ResEqual(ellipseUnit2.GetMinorRadius(), 1.2));

    EllipseDef ellipseUnit3;
    ellipseUnit3 = ellipseUnit1;

    CheckM(ResEqual(ellipseUnit3.GetPos(), Vec(1.0, 1.0, 0.0)));
    CheckM(ResEqual(ellipseUnit3.GetAxis().GetVec(), Vec(0.0, 0.0, 1.0)));
    CheckM(ResEqual(ellipseUnit3.GetMajorAxis().GetVec(), Vec(1.0, 0.0, 0.0)));
    CheckM(ResEqual(ellipseUnit3.GetMajorRadius(), 3.4));
    CheckM(ResEqual(ellipseUnit3.GetMinorRadius(), 1.2));

    EllipseDef ellipseUnit4(3.4, 1.2, tempAx, tempMajAx, tempPos);

    CheckM(ResEqual(ellipseUnit4.GetPos(), Vec(1.0, 1.0, 0.0)));
    CheckM(ResEqual(ellipseUnit4.GetAxis().GetVec(), Vec(0.0, 0.0, 1.0)));
    CheckM(ResEqual(ellipseUnit4.GetMajorAxis().GetVec(), Vec(1.0, 0.0, 0.0)));
    CheckM(ResEqual(ellipseUnit4.GetMajorRadius(), 3.4));
    CheckM(ResEqual(ellipseUnit4.GetMinorRadius(), 1.2));

    return true;
}


VTK_TEST_FN(TestPKUtils049_ZeroRadiusCircle01)
{
    //Unit test CreateCircle, GetCircleData, CircleData, CurveGetLength, CurveIsZeroLength, GeomMoveToBody, CurveEvaluateAtParam, CurveGetParamForPos
    PKUtils pkUtils;
    Vec cent(1, 1, 0);
    Dir axis = Dir::ZDir();
    Dir refDir = Dir::XDir();
    double radius = 0;
    PK_CIRCLE_t circ1 = pkUtils.CreateCircle(cent, axis, radius, &refDir);
    CheckTagM(circ1);

    //Create another circle in a given body.
    VTKBody vbody = VTKBody::EntityGetConstructionBody(circ1);
    CheckM(vbody.IsNotNull());
    PK_CIRCLE_t circ2 = pkUtils.CreateCircle(cent, axis, radius, &refDir, &vbody);
    CheckTagM(circ2);
    CheckM(vbody == VTKBody::EntityGetConstructionBody(circ2));
    PK_CIRCLE_t circ3 = pkUtils.CreateCircle(cent, axis, radius, &refDir);
    CheckTagM(circ3);
    CheckM(vbody != VTKBody::EntityGetConstructionBody(circ3));
    CheckM(pkUtils.GeomMoveToBody(circ3, vbody));
    CheckM(vbody == VTKBody::EntityGetConstructionBody(circ3));

    Vec centOut;
    Dir axisOut, refDirOut;
    double radiusOut = -1;
    CheckM(pkUtils.GetCircleData(circ1, &centOut, &axisOut, &refDirOut, &radiusOut, 0, 0));
    CheckM(ResEqual(centOut, cent));
    CheckM(axis.IsAligned(axisOut));
    CheckM(refDir.IsAligned(refDirOut));
    CheckM(ResEqual(radiusOut, radius));

    pkUtils.CircleData(circ1, axisOut, refDirOut, centOut, radiusOut);
    CheckM(ResEqual(centOut, cent));
    CheckM(axis.IsAligned(axisOut));
    CheckM(refDir.IsAligned(refDirOut));
    CheckM(ResEqual(radiusOut, radius));

    PK_INTERVAL_t pkInterval;
    double len = pkUtils.CurveGetLength(circ1, &pkInterval);
    CheckM(ResZero(len));
    CheckM(ResnorEqual(pkInterval.value[0], 0));
    CheckM(ResnorEqual(pkInterval.value[1], TWO_PI));

    CheckM(pkUtils.CurveIsZeroLength(circ1));
    CheckM(pkUtils.CurveIsZeroRadiusCircle(circ1));

    Vec pos = cent;
    double p = -1;
    CheckM(pkUtils.CurveGetParamForPos(circ1, pos, &p));
    CheckM(ResnorZero(p));
    pos += Vec(2, 2, 0);
    CheckM(!pkUtils.CurveGetParamForPos(circ1, pos, &p));

    for (double p = 0; p < TWO_PI; p += PI / 8)
    {
        Vec pos, tang;
        double curvature = -1;
        CheckM(pkUtils.CurveEvaluateAtParam(circ1, p, &pos, &tang, NULL));
        CheckM(ResEqual(pos, cent));
        Dir tanDir = Normal(cos(p+PI/2), sin(p + PI / 2), 0);
        CheckM(tanDir.IsAligned(Normal(tang)));

        VecArray posAndDers;
        CheckM(pkUtils.CurveEvaluateAtParam(circ1, p, 2, posAndDers));
        CheckM(posAndDers.size() == 3);
        CheckM(ResEqual(pos, posAndDers[0]));
        CheckM(tanDir.IsAligned(Normal(posAndDers[1])));

        CheckM(pkUtils.CurveEvaluateAtParamHanded(circ1, p, 2, false, posAndDers));
        CheckM(posAndDers.size() == 3);
        CheckM(ResEqual(pos, posAndDers[0]));
        CheckM(tanDir.IsAligned(Normal(posAndDers[1])));
    }
    return true;
}

VTK_TEST_FN(TestPKUtils049_ZeroRadiusCircle02)
{
    //Unit test TransformCreateRadiusScaling and GeomTransform for zero-radius circles.
    PKUtils pkUtils;
    Vec cent(1, 1, 0);
    Dir axis = Dir::ZDir();
    Dir refDir = Dir::XDir();
    double radius = 0;
    PK_CIRCLE_t circ1 = pkUtils.CreateCircle(cent, axis, radius, &refDir);
    CheckTagM(circ1);
    PK_CIRCLE_t circ2 = pkUtils.CreateCircle(cent, axis, radius, &refDir);
    CheckTagM(circ2);
    PK_CIRCLE_t circ3 = pkUtils.CreateCircle(cent, axis, radius, &refDir);
    CheckTagM(circ3);

    PK_GEOM_t geomOut = 0;
    Xform xform;
    xform.SetTranslation(Vec(2, 0, 0));
    CheckM(pkUtils.GeomTransform(circ1, xform, geomOut));
    CheckTagM(geomOut);
    CheckM(pkUtils.CurveIsZeroRadiusCircle(geomOut));

    //Key of the folllowing GeomTransform functions is the transform can contain scaling, which 
    //will change the radius of a zero-radius circle.
    //Create a scaling transform that will make the circle radius 0.1.
    PK_TRANSF_t scalingTransf = pkUtils.TransformCreateRadiusScaling(circ1, 0.1);
    double scalingFactor = 1;
    const double PK_CIRCLE_RADIUS_FOR_ZERO_RADIUS = (Resabs * 100);
    CheckTagM(scalingTransf);
    CheckM(pkUtils.TransformGetScaleFactor(scalingTransf, scalingFactor));
    CheckM(ResEqual(scalingFactor, 0.1 / PK_CIRCLE_RADIUS_FOR_ZERO_RADIUS));
    CheckM(pkUtils.GeomTransform(circ1, scalingTransf));
    CheckM(pkUtils.CurveIsZeroRadiusCircle(circ1) == false);
    Vec centOut;
    Dir axisOut, refDirOut;
    double radiusOut = -1;
    CheckM(pkUtils.GetCircleData(circ1, &centOut, &axisOut, &refDirOut, &radiusOut, 0, 0));
    CheckM(ResEqual(radiusOut, 0.1));

    IntArray geomsIn, geomsOut;
    geomsIn.push_back(circ2);
    geomsIn.push_back(circ3);
    CheckM(pkUtils.GeomTransform(geomsIn, scalingTransf, geomsOut));
    CheckM(geomsOut.size() == 2);
    for (PK_GEOM_t geom : geomsOut)
    {
        CheckM(pkUtils.CurveIsZeroRadiusCircle(geom) == false);
        CheckM(pkUtils.GetCircleData(geom, &centOut, &axisOut, &refDirOut, &radiusOut, 0, 0));
        CheckM(ResEqual(radiusOut, 0.1));
    }

    //In-place transform fails if the geoms are attached to different bodies (possible inconsistent behaviour of PK_GEOM_transform_2). 
    VTKBody vbody = VTKBody::EntityGetConstructionBody(circ2);
    pkUtils.GeomMoveToBody(circ3, vbody);
    pkUtils.GeomTransform(geomsIn, scalingTransf);
    CheckM(geomsIn.size() == 2);
    for (PK_GEOM_t geom : geomsIn)
    {
        CheckM(pkUtils.CurveIsZeroRadiusCircle(geom) == false);
        CheckM(pkUtils.GetCircleData(geom, &centOut, &axisOut, &refDirOut, &radiusOut, 0, 0));
        CheckM(ResEqual(radiusOut, 0.1));
    }

    return true;
}

VTK_TEST_FN(TestPKUtils050_ZeroLengthEdge01)
{
    //Unit test:
    //  EntityIsValidVirtualGeom
    //  TopolGetOrientedGeom
    //  TopolGetGeom
    //  EdgeGetCurve
    //  BodyGetZeroLengthEdges
    //GetGeo
    //EdgeContainsPoint
    //CurveIsUsedByZeroLengthEdge
    //EdgeAttachCurve
    //EdgeGetInterval
    //TopolFindBox

    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/ZPlaneProfiles/diamond.xml").c_str(), &result);
    CheckM(result == ResultTypeOk && interaction != 0);

    VTKBodyArray bodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodies);
    CheckM(bodies.size() == 1);

    //Try creating a zero-length edge at the tip of the diamond
    PK_VERTEX_t vert = VertexOfId(bodies[0], 25);
    CheckTagM(vert);
    Vec pos = PKUtils().VertexGetPoint(vert);

    Vec linePos = pos;
    linePos.x -= 0.8;   //Offset a bit to the left so the interval won't be [0,0];
    VTK::LineData lineData;
    lineData.SetPosition(Position(linePos.x, linePos.y, linePos.z));
    lineData.SetDirection(Direction(1, 0, 0));
    int line = VTK::CreateLine(interaction, lineData);
    CheckTagM(line);

    DoublePair interval;
    interval.first = interval.second = VTK::CurveGetParameterAtPosition(interaction, line, Position(pos.x, pos.y, pos.z), &result);
    CheckResultM(result);

    PK_EDGE_t newEdge = PKUtils().CreateZeroLengthEdge(GetVersion(interaction), line, interval, &bodies[0]);
    CheckTagM(newEdge);

    PK_CURVE_t curve = 0;

    //No PK curve attached to the edge.
    PK_CLASS_t geomClassUnused = PK_CLASS_null;
    VecArray endsUnused;
    double intervalUnused[2];
    bool senseUnused;
    bool err = PKUtils().EdgeGetGeometry(newEdge, false, curve, geomClassUnused, endsUnused, intervalUnused, senseUnused);
    CheckM(curve == 0);
    CheckM(err);

    //The curve is not attached to the edge either in PK
    int edgeCount = 0;
    PK_EDGE_t* edgeTags = NULL;
    err = PK_CURVE_ask_edges_nmnl(line, &edgeCount, &edgeTags);
    CheckM(err == PK_ERROR_none);
    CheckM(edgeCount == 0);
    CheckM(edgeTags == 0);

    CheckM(PKUtils().EdgeIsZeroLength(newEdge, &curve));
    CheckM(curve == line);

    double len = -1;
    CheckM(PKUtils().EdgeGetLength(newEdge, len));
    CheckM(ResZero(len));

    CheckM(PKUtils().EdgeGetCurve(newEdge) == curve);
    PK_LOGICAL_t sense = PK_LOGICAL_false;
    CheckM(PKUtils().TopolGetOrientedGeom(newEdge, sense) == curve);
    CheckM(sense == PK_LOGICAL_true); //zero-length edge always has true sense.
    CheckM(PKUtils().TopolGetGeom(newEdge) == curve);

    IntArray edges;
    CheckM(PKUtils().CurveGetEdges(curve, edges) == 1 && edges.size() == 1 && edges[0] == newEdge);

    PK_EDGE_t edgeOut = 0;
    CheckM(PKUtils().CurveIsUsedByZeroLengthEdge(curve, &edgeOut));
    CheckM(edgeOut == newEdge);

    double intervalArr[2] = { 0,0 };
    PKUtils().EdgeGetInterval(newEdge, intervalArr, &curve);
    CheckM(ResEqual(intervalArr[0], interval.first));
    CheckM(ResEqual(intervalArr[1], interval.second));
    CheckM(curve == line);

    CheckM(PKUtils().EntityGetGeomClass(newEdge) == EntityClass::Line);

    IntArray zeroLenEdges;
    int n = PKUtils().BodyGetZeroLengthEdges(bodies[0].GetPKBodyTag(), zeroLenEdges);
    CheckM(n == 1);
    CheckM(zeroLenEdges.size() == 1 && zeroLenEdges[0] == newEdge);

    Vec posOut;
    Dir dirOut;
    EntityClass geomCls = PKUtils().GetGeo(newEdge, &curve, 0, &posOut, &dirOut, 0, 0, 0, 0, 0);
    CheckM(geomCls == EntityClass::Line);
    CheckM(curve == line);
    CheckM(ResEqual(posOut, linePos));
    CheckM(dirOut.IsAligned(Dir::XDir()));

    IntArray vertices;
    PKUtils().EdgeGetVertices(newEdge, vertices);
    CheckM(vertices.size() == 2 && vertices[0] != vertices[1]);

    PK_TOPOL_t coiTopol = 0;
    CheckM(PKUtils().EdgeContainsPoint(newEdge, pos, &coiTopol));
    CheckM(coiTopol == vertices[0]);

    Vec midPoint;
    PKUtils().EdgeGetMidPoint(newEdge, midPoint);
    CheckM(ResEqual(midPoint, pos));

    AlignedBox box;
    PKUtils().TopolFindBox(newEdge, box);
    CheckM(ResEqual(box.m_minCoord, PKUtils().VertexGetPoint(vertices[0])));
    IntArray topols;
    topols.push_back(newEdge);
    AlignedBox box2;
    PKUtils().TopolFindBox(topols, box2);
    CheckM(ResEqual(box2.m_minCoord, PKUtils().VertexGetPoint(vertices[0])));

    //Check the associated curve cannot be attached
    CheckM(!PKUtils().EdgeAttachCurve(newEdge, curve));

    //Check another curve cannot be attached to the edge.
    GeometryTag line2 = CreateLine(interaction, lineData, &result);
    CheckTagM(line2);
    CheckM(!PKUtils().EdgeAttachCurve(newEdge, line2));
    CheckM(PKUtils().EdgeGetCurve(newEdge) == curve);

    Vec end1Pos, end2Pos;
    Dir end1Dir, end2Dir;
    CheckM(PKUtils().EdgeFindEndTangents(newEdge, end1Dir, end2Dir, &end1Pos, &end2Pos));
    CheckM(ResParallel(end1Dir, Dir::XDir()));
    CheckM(ResParallel(end2Dir, Dir::XDir()));
    CheckM(ResEqual(end1Pos, pos));
    CheckM(ResEqual(end2Pos, pos));

    CheckM(!PKUtils().EntityIsValidVirtualGeom(curve));
    CheckM(PKUtils().EntityIsValidVirtualGeom(curve, false));

    LineData lineDataOut;
    VTK::EntityGetLineData(interaction, curve, &lineDataOut, &result);
    CheckResultM(result);
    CheckM(lineDataOut == lineData);

    VTK::EntityGetLineData(interaction, newEdge, &lineDataOut, &result);
    CheckResultM(result);
    CheckM(lineDataOut == lineData);

    //Finally test TopolDetachGeom
    CheckM(PKUtils().TopolDetachGeom(newEdge));
    CheckM(PKUtils().EdgeGetCurve(newEdge) == 0);
    //The curve is free.
    CheckM(PKUtils().EntityIsValidVirtualGeom(curve));
    return true;
}

VTK_TEST_FN(TestPKUtils050_ZeroLengthEdge02)
{
    //VertexHasZeroLengthEdge
    //CircularEdgeGetCenter
    //EdgeFindEndTangents
    //IntervalIsEmpty
    //EdgeGetInterval
    //EdgeIsTolerant
    //EdgeOutputVectors
    //BodyGetGeoms
    //CircularEdgeComputeHeight
    //EdgeReplaceCurve
    //TopolsFindMinimalDistance
    //EdgeGetLength
    //EdgeIsPlanar
    //BodyTransform

    ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/LoopObjects/zero_len_arcs_and_line.xml").c_str(), &result);
    CheckResultM(result);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(GetVersion(interaction), bodyArray);
    CheckM(bodyArray.size() == 1);

    EntityTag circZeroLenEdge1 = EdgeOfId(bodyArray[0], 191);
    CheckTagM(circZeroLenEdge1);
    EntityTag circZeroLenEdge2 = EdgeOfId(bodyArray[0], 193);
    CheckTagM(circZeroLenEdge2);
    EntityTag lineZeroLenEdge = EdgeOfId(bodyArray[0], 195);
    CheckTagM(lineZeroLenEdge);

    PK_VERTEX_t vert = VertexOfId(bodyArray[0], 196);
    CheckTagM(vert);
    IntArray zeroLenEdges, nonZeroLenEdges;
    PKUtils().VertexHasZeroLengthEdge(vert, &zeroLenEdges, &nonZeroLenEdges);
    CheckM(zeroLenEdges.size() == 1 && zeroLenEdges[0] == circZeroLenEdge1);
    CheckM(nonZeroLenEdges.size() == 1 && nonZeroLenEdges[0] == EdgeOfId(bodyArray[0], 461));

    Vec cent;
    PKUtils().CircularEdgeGetCenter(circZeroLenEdge1, cent);
    CheckM(ResEqual(cent, Vec(-0.05, 0.04090169943749476, 0)));

    Dir end1Tan, end2Tan;
    Vec end1Pos, end2Pos;
    PKUtils().EdgeFindEndTangents(circZeroLenEdge1, end1Tan, end2Tan, &end1Pos, &end2Pos);
    //Ends of a zero-radius circular edge are always considered at param 0.
    CheckM(end1Tan.IsAligned(Dir::YDir()));
    CheckM(end2Tan.IsAligned(Dir::YDir()));
    CheckM(ResEqual(end1Pos, cent));
    CheckM(ResEqual(end2Pos, cent));

    //Ends of a linear zero-length edge.
    PKUtils().EdgeFindEndTangents(lineZeroLenEdge, end1Tan, end2Tan, &end1Pos, &end2Pos);
    CheckM(end1Tan.IsAligned(Dir::YDir()));
    CheckM(end2Tan.IsAligned(Dir::YDir()));
    CheckM(ResEqual(end1Pos, Vec(-0.06819660112501053, 0.05, 0)));
    CheckM(ResEqual(end2Pos, Vec(-0.06819660112501053, 0.05, 0)));

    double interval[2] = { -1, -1 };
    PK_CURVE_t curve = 0;
    PKUtils().EdgeGetInterval(circZeroLenEdge1, interval, &curve);
    CheckM(ResEqual(interval[0], 0));
    CheckM(ResEqual(interval[1], 0));
    CheckTagM(curve);
    CheckM(PKUtils().CurveIsZeroRadiusCircle(curve));

    PKUtils().EdgeGetInterval(lineZeroLenEdge, interval, &curve);
    CheckM(ResEqual(interval[0], 0));
    CheckM(ResEqual(interval[1], 0));
    CheckTagM(curve);

    CheckM(!PKUtils().EdgeIsTolerant(lineZeroLenEdge));

    IntArray geoms;
    PKUtils().BodyGetGeoms(bodyArray[0].GetPKBodyTag(), geoms);
    CheckM(geoms.size() == 3);

    IntArray topols1, topols2;
    topols1.push_back(circZeroLenEdge1);
    topols1.push_back(circZeroLenEdge2);
    topols2.push_back(lineZeroLenEdge);
    double minDis = 0;
    PK_ENTITY_t endEnts[2] = { 0,0 };
    Vec endPoses[2];
    CheckM(PKUtils().TopolsFindMinimalDistance(topols1, topols2, minDis, endEnts, endPoses));
    CheckM(ResEqual(minDis, 0.020344418537486333));

    Vec planePos;
    Dir planeNorm;
    CheckM(PKUtils().EdgeIsPlanar(circZeroLenEdge1, &planePos, &planeNorm));

    Xform xform;
    xform.SetTranslation(Vec(0.02, 0, 0));
    PK_TRANSF_t transf = PKUtils().GetParasolidTransformTag(xform);
    PKUtils().CircularEdgeGetCenter(circZeroLenEdge1, cent);
    CheckM(ResEqual(cent, Vec(-0.05, 0.04090169943749476, 0)));
    CheckM(bodyArray[0].Transform(transf, Resabs));
    PKUtils().CircularEdgeGetCenter(circZeroLenEdge1, cent);
    CheckM(ResEqual(cent, Vec(-0.03, 0.04090169943749476, 0)));

    //Test TopolDetachGeom, the circle should still be a zero-radius
    PK_CURVE_t circ1 = PKUtils().EdgeGetCurve(circZeroLenEdge1);
    CheckM(PKUtils().CurveIsZeroRadiusCircle(circ1));
    PKUtils().TopolDetachGeom(circZeroLenEdge1);
    CheckM(PKUtils().CurveIsZeroRadiusCircle(circ1));
    CheckM(!PKUtils().EdgeIsZeroLength(circZeroLenEdge1)); //The edge does not have an associated curve so it is not a zero-length edge anymore

    return true;
}

VTK_TEST_FN(TestPKUtils050_ZeroLengthEdge03)
{
    //CreateZeroLengthEdge
    //DefineZeroLengthEdge
    //ConvertZeroLengthEdgeToNormalEdge
    ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/LoopObjects/zero_len_arcs_and_line.xml").c_str(), &result);
    CheckResultM(result);

    PK_LINE_t line = PKUtils().CreateLine(Vec(1, 0, 0), Dir::XDir());
    DoublePair interval(0.1, 0.1);
    PK_EDGE_t lineEdge = PKUtils().CreateZeroLengthEdge(GetVersion(interaction), line, interval);
    CheckTagM(lineEdge);
    PK_CURVE_t curve = 0;
    CheckM(PKUtils().EdgeIsZeroLength(lineEdge, &curve));
    CheckM(curve == line);
    VecArray points;
    PKUtils().EdgeGetVertexPoints(lineEdge, points);
    CheckM(points.size() == 2);
    CheckM(ResEqual(points[0], Vec(1.1, 0, 0)));
    CheckM(ResEqual(points[1], Vec(1.1, 0, 0)));

    //De-associate the edge and line, and test we can redefine the association.
    PKUtils().TopolDetachGeom(lineEdge);
    CheckM(!PKUtils().EdgeIsZeroLength(lineEdge, &curve));
    CheckM(PKUtils().DefineZeroLengthEdge(lineEdge, line));
    CheckM(PKUtils().EdgeIsZeroLength(lineEdge, &curve));
    CheckM(curve == line);

    CheckM(PKUtils().ConvertZeroLengthEdgeToNormalEdge(lineEdge, false));
    CheckM(!PKUtils().EdgeIsZeroLength(lineEdge));
    //Check PK association
    CheckM(PK_ERROR_no_errors == PK_EDGE_ask_curve_nmnl(lineEdge, &curve));
    CheckM(curve == line);

    //Test zero-radius circle
    PK_CIRCLE_t circ = PKUtils().CreateCircle(Vec(1, 0, 0), Dir::ZDir(), 0.0);
    CheckTagM(circ);
    CheckM(PKUtils().CurveIsZeroRadiusCircle(circ));
    interval = DoublePair(PI / 2, PI / 2);
    PK_EDGE_t circEdge = PKUtils().CreateZeroLengthEdge(GetVersion(interaction), circ, interval);
    CheckTagM(circEdge);
    curve = 0;
    CheckM(PKUtils().EdgeIsZeroLength(circEdge, &curve));
    CheckM(curve == circ);
    points.clear();
    PKUtils().EdgeGetVertexPoints(circEdge, points);
    CheckM(points.size() == 2);
    CheckM(ResEqual(points[0], Vec(1, 0, 0)));
    CheckM(ResEqual(points[1], Vec(1, 0, 0)));
    //De-associate the edge and circle, and test we can redefine the association.
    PKUtils().TopolDetachGeom(circEdge);
    CheckM(!PKUtils().EdgeIsZeroLength(circEdge, &curve));
    CheckM(PKUtils().CurveIsZeroRadiusCircle(circ));
    CheckM(PKUtils().DefineZeroLengthEdge(circEdge, circ));
    CheckM(PKUtils().EdgeIsZeroLength(circEdge, &curve));
    CheckM(curve == circ);

    return true;
}

VTK_TEST_FN(TestPKUtils050_ZeroLengthEdge04)
{
    //EdgeDeleteFromWireAutoHeal
    //EdgeDeleteFromWireNoHeal
    //EdgeDeleteFromWireHealWithLinearCap
    return true;
}

VTK_TEST_FN(TestPKUtils051_IdentifySharpTangent)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/ZPlaneProfiles/Sharp_tangent_blend.xml").c_str(), result);
    CheckResultM(result);
    Profile* profile = (Profile*)(interaction->FindTaggedEntityById(149));
    CheckM(profile != NULL);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t sharpLine = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 103);
    PK_EDGE_t blend = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 21);
    PK_EDGE_t smoothLine = bodies[0].FindEntityByIdentifier(EntityClass::Edge, 26);

    PK_VERTEX_t sharpVert = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 15);
    PK_VERTEX_t smoothVert = bodies[0].FindEntityByIdentifier(EntityClass::Vertex, 20);

    double angularTolerance = 1e-11;
    CheckM(PKUtils().VertexIsTangent(smoothVert, smoothLine, blend, angularTolerance, false));
    CheckM(PKUtils().VertexIsTangent(smoothVert, smoothLine, blend, angularTolerance, true));

    CheckM(PKUtils().VertexIsTangent(sharpVert, sharpLine, blend, angularTolerance, false));
    CheckM(!PKUtils().VertexIsTangent(sharpVert, sharpLine, blend, angularTolerance, true));

    return true;
}

VTK_TEST_FN(TestPKUtils052_WirebodyMakeFromCurves)
{
    //Test vertex knitting failure during WirebodyMakeFromCurves
    InteractionTag interaction = VTK::CreateInteraction();
    CheckTagM(interaction);
    VTK::Version ver = VTK::GetVersion(interaction);

    PK_CURVE_t line1 = PKUtils().CreateLine(Vec(0, 0, 0), Dir::XDir());
    PK_CURVE_t line2 = PKUtils().CreateLine(Vec(0, 0, 0), Dir::YDir());
    PK_CURVE_t line3 = PKUtils().CreateLine(Vec(0, 0, 0), Normal(Vec(1, -1, 0)));
    
    IntArray curves;
    DoublePairArray intervals;
    IntIntPairToIntIntPairArrayMap edgeConnectivity;
    IntArray createdEdges;
    curves.push_back(line1);
    curves.push_back(line2);
    curves.push_back(line3);
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(0.1, 0.1));  //Empty interval but defines incorrect vertex positions.
    edgeConnectivity[IntIntPair(line3, 0)].push_back(IntIntPair(line1, 0));
    edgeConnectivity[IntIntPair(line3, 1)].push_back(IntIntPair(line2, 0));
    PK_BODY_t body = PKUtils().WirebodyMakeFromCurves(ver, curves, intervals, true, &edgeConnectivity, &createdEdges);
    CheckM(body == 0);
    return true;
}

VTK_TEST_FN(TestPKUtils053_ComputeNonEmptyInterval)
{
    //Test we need multiple iterations in ComputeNonEmptyIntervalForZeroLengthEdgeCreation
    //to compute the non-empty interval.
    VecArray pts;
    pts.push_back(Vec(0, 0, 0));
    pts.push_back(Vec(Resabs * 10, Resabs * 10, 0));
    pts.push_back(Vec(Resabs * 20, 0, 0));
    ConicDef conicDef(pts, 0.5);
    PK_CURVE_t conic = PKUtils().CreateConic(conicDef);
    CheckTagM(conic);
    DoublePair interval(0, 0);
    DoublePair newInterval = PKUtils().ComputeNonEmptyIntervalForZeroLengthEdgeCreation(interval, conic);
    CheckM(!PKUtils().IntervalIsEmpty(newInterval));
    return true;
}

VTK_TEST_FN(TestPKUtils054_CurveOutputVectors_OutOfRange)
{
    //Test CurveOutputVectors works if the given range is outside the curve's interval
    VTK::ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_ended_bcurve_and_conic.xml").c_str(), result);
    CheckM(interaction != 0);
    Profile* profile = interaction->GetProfile(0);
    CheckM(profile);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t splineEdge = EdgeOfId(bodies[0], 339);
    CheckTagM(splineEdge);
    PK_EDGE_t conicEdge = EdgeOfId(bodies[0], 143);
    CheckTagM(conicEdge);


    PK_CURVE_t splineCurve = PKUtils().EdgeGetCurve(splineEdge);
    PK_CURVE_t conicCurve = PKUtils().EdgeGetCurve(conicEdge);

    IntArray curves;
    curves.push_back(splineCurve);
    curves.push_back(conicCurve);

    for (PK_CURVE_t curve : curves)
    {
        PK_INTERVAL_t interval;
        PKUtils().CurveGetInterval(curve, interval);

        VecArray vecs0, vecs1;
        CheckM(PKUtils().CurveEvaluateAtParam(curve, interval.value[0], 1, vecs0));
        CheckM(PKUtils().CurveEvaluateAtParam(curve, interval.value[1], 1, vecs1));

        double range[] = { -10, 10 };
        VecArray vecs;
        DoubleVector params;
        int n = PKUtils().CurveOutputVectors(curve, 0, 0, PKUtils().CurveGetLength(curve) / 10, range, vecs, &params);
        CheckM(n > 0);
        CheckM(vecs.size() == params.size());
        for (size_t i = 0; i < vecs.size(); i++)
        {
            double p = params[i];
            Vec v = vecs[i];
            if (i == 0)
                CheckM(ResnorEqual(p, range[0]));
            if (i == vecs.size() - 1)
                CheckM(ResnorEqual(p, range[1]));
            if (p < interval.value[0])
            {
                //Check the sampled point is on the linear extension and at the correct position.
                CheckM(Normal(vecs0[0] - v).IsAligned(Normal(vecs0[1])));
                Vec k = vecs0[0] + vecs0[1] * (p - interval.value[0]);
                CheckM(ResEqual(k, v));
            }
            else if (p > interval.value[1])
            {
                //Check the sampled point is on the linear extension and at the correct position.
                CheckM(Normal(v - vecs1[0]).IsAligned(Normal(vecs1[1])));
                Vec k = vecs1[0] + vecs1[1] * (p - interval.value[1]);
                CheckM(ResEqual(k, v));
            }
        }
    }

    //Create an ellipse (periodic) and check all out-of-range samples are still on the ellipse.
    Vec cent(1, 0, 0);
    Dir majorAxis = Dir::XDir();
    Dir minorAxis = Dir::YDir();
    Dir axis = Dir::ZDir();
    PK_CURVE_t ellipseCurve = PKUtils().CreateEllipse(cent, axis, majorAxis, 0.5, 0.2);
    CheckTagM(ellipseCurve);
    double range[] = { -10, 10 };
    VecArray vecs;
    DoubleVector params;
    int n = PKUtils().CurveOutputVectors(ellipseCurve, 0, 0, PKUtils().CurveGetLength(ellipseCurve) / 30, range, vecs, &params);
    CheckM(n > 0);
    CheckM(vecs.size() == params.size());
    for (size_t i = 0; i < vecs.size(); i++)
    {
        double p = params[i];
        Vec v = vecs[i];

        Vec k = Vec(1, 0, 0) + majorAxis * cos(p) * 0.5 + minorAxis * sin(p) * 0.2;

        CheckM(ResEqual(k, v));
    }
    return true;
}

VTK_TEST_FN(TestPKUtils055_OffsetCurveExtension)
{
    //Test offset curve can be created if the required interval is larger than the base
    VTK::ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/open_ended_bcurve_and_conic.xml").c_str(), result);
    CheckM(interaction != 0);
    Profile* profile = interaction->GetProfile(0);
    CheckM(profile);
    VTKBodyArray bodies = profile->GetWireBodies();
    CheckM(bodies.size() == 1);

    PK_EDGE_t splineEdge = EdgeOfId(bodies[0], 339);
    CheckTagM(splineEdge);
    PK_EDGE_t conicEdge = EdgeOfId(bodies[0], 143);
    CheckTagM(conicEdge);

    PKUtils pkUtils;
    OffsetCurveApproximator offsetApproximator(interaction->GetVersion(), &pkUtils);

    //Make a copy of the bcurve 
    BcurveDef bcurveDef;
    pkUtils.BcurveGet(pkUtils.EdgeGetCurve(splineEdge), bcurveDef);
    PK_CURVE_t baseBcurve = pkUtils.BcurveCreate(bcurveDef, &bodies[0]);
    CheckTagM(baseBcurve);

    DoubleVector params;
    params.push_back(-0.3);
    params.push_back(1.3);
    PK_GEOM_t offsetGeom = offsetApproximator.TestCreateOffsetCurveGeometry(baseBcurve, 0.001, OffsetSideUnit::Left, NULL, params, &bodies[0]);
    CheckTagM(offsetGeom);

    //This fails currently
    params.clear();
    params.push_back(-0.4);
    params.push_back(1.3);
    offsetGeom = offsetApproximator.TestCreateOffsetCurveGeometry(baseBcurve, 0.001, OffsetSideUnit::Left, NULL, params, &bodies[0]);
    CheckTagM(offsetGeom);

    VecArray vecs;
    pkUtils.CurveEvaluateAtParam(baseBcurve, 100, 0, vecs);

    ConicDef conicDef;
    pkUtils.ConicData(pkUtils.EdgeGetCurve(conicEdge), conicDef);
    PK_CURVE_t baseConic = pkUtils.CreateConic(conicDef, &bodies[0]);
    CheckTagM(baseConic);
    params.clear();
    params.push_back(-0.3);
    params.push_back(1.3);
    offsetGeom = offsetApproximator.TestCreateOffsetCurveGeometry(baseConic, 0.001, OffsetSideUnit::Left, NULL, params, &bodies[0]);
    CheckTagM(offsetGeom);

    params.clear();
    params.push_back(-4);
    params.push_back(5);
    vecs.clear();
    pkUtils.CurveOutputVectors(baseBcurve, 0, 0, pkUtils.CurveGetLength(baseBcurve) / 20, params.data(), vecs);
    for (Vec v : vecs)
    {
        pkUtils.CreatePoint(v, &bodies[0]);
    }
    //DebugSaveInteraction(interaction->GetTag(), "C:\\ttt.xml");
    return true;
}

VTK_TEST_FN(TestPKUtils056_AttachDetachOffsetCurve)
{
    //Test TopolDetachGeom and EdgeAttachCurve handle offset curve PK_GROUPs correctly
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/offset_curve_on_bcurve.xml").c_str(), result);
    VTKBody vbody = interaction->GetProfile(0)->GetWireBodies()[0];
    CheckVTKBodyM(vbody);

    PK_EDGE_t baseEdge = EdgeOfId(vbody, 222);
    CheckTagM(baseEdge);
    PK_CURVE_t baseCurve = PKUtils().EdgeGetCurve(baseEdge);
    CheckTagM(baseCurve);

    PK_EDGE_t offsetEdge = EdgeOfId(vbody, 216);
    CheckTagM(offsetEdge);
    CheckM(EntityClass::OffsetCurve == PKUtils().EntityGetGeomClass(offsetEdge));
    PK_CURVE_t offsetCurve = PKUtils().EdgeGetCurve(offsetEdge);
    CheckTagM(offsetCurve);
    CheckM(EntityClass::OffsetCurve == PKUtils().EntityGetGeomClass(offsetCurve));
    OffsetCurveGroup group;
    PKUtils().OffsetCurveGetOffsetGroup(offsetCurve, group);
    CheckM(group.m_groupTag != 0);
    CheckM(group.m_offsetCurve = offsetCurve);
    CheckM(group.m_baseCurve = PKUtils().EdgeGetCurve(baseEdge));
    PK_CURVE_t baseCurveOut = 0;
    double distance = 0;
    OffsetSideUnit side = OffsetSideUnit::Left;
    CheckM(PKUtils().OffsetCurveData(offsetCurve, &baseCurveOut, &distance, &side));
    CheckM(baseCurveOut == baseCurve);
    CheckM(side == OffsetSideUnit::Right);
    CheckM(ResEqual(distance, 0.01));

    CheckM(PKUtils().TopolDetachGeom(offsetEdge));

    //Check the PK_GROUP and offset curve attributes are preserved after the detach.
    OffsetCurveGroup group2;
    PKUtils().OffsetCurveGetOffsetGroup(offsetCurve, group2);
    CheckM(group == group2);
    CheckM(PKUtils().OffsetCurveData(offsetCurve, &baseCurveOut, &distance, &side));
    CheckM(baseCurveOut == baseCurve);
    CheckM(side == OffsetSideUnit::Right);
    CheckM(ResEqual(distance, 0.01));

    CheckM(PKUtils().EdgeAttachCurve(offsetEdge, offsetCurve));

    //Check the PK_GROUP and offset curve attributes are preserved after the attach.
    OffsetCurveGroup group3;
    PKUtils().OffsetCurveGetOffsetGroup(offsetCurve, group3);
    CheckM(group == group3);
    CheckM(PKUtils().OffsetCurveData(offsetCurve, &baseCurveOut, &distance, &side));
    CheckM(baseCurveOut == baseCurve);
    CheckM(side == OffsetSideUnit::Right);
    CheckM(ResEqual(distance, 0.01));

    return true;
}

VTK_TEST_FN(TestPKUtils057_EllipseEllipseTangent)
{
    //Check we can identify a tangent position on the underlying ellipses of two edges with a satisfied tangent relation - this
    //requires a resolution based on curvature rather than using Resnor when comparing tangents
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/EllipseEllipseWithSharedEndsAndSatisfiedTangentRelation.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,560);
    int edge2 = EdgeOfId(vbody,664);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Check we can find a tangent position with curves presented in either order (these positions won't be the same as the curves
    //are identical to session tolerance and Parasolid gives different intersecting positions depending on the curve order)
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(!tangentPositions1.empty());

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(!tangentPositions2.empty());

    return true;
}

VTK_TEST_FN(TestPKUtils058_BCurveEllipseNearlyTAETangentPositions)
{
    //Check that we identify only one vertex as a tangent position (D-Cubed sees both relations as satisfied but the tangent
    //at the end vertex has tangents which are only parallel to 1e-5 as the conic param is not quite 1.0)
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Conics/ConicEllipseNearlyTAEAtBothEndsSharpRels.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,477);
    int edge2 = EdgeOfId(vbody,356);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 1);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 1);

    return true;
}

class TestPositionLessThan
{
public:
    bool operator () (const Vec& pos1, const Vec& pos2) const;
};

bool TestPositionLessThan::operator()(const Vec& pos1, const Vec& pos2) const
{
    bool lessThan = false;
    if ( !ResEqual(pos1,pos2) )
    {
        //All components cannot be identical so use lexicographic ordering
        if ( pos1.x < pos2.x )
        {
            lessThan = true;
        }
        else if ( pos2.x < pos1.x )
        {
            lessThan = false;
        }
        else
        {
            if ( pos1.y < pos2.y )
            {
                lessThan = true;
            }
            else if ( pos2.y < pos1.y )
            {
                lessThan = false;
            }
            else
            {
                lessThan = (pos1.z < pos2.z);
            }
        }
    }

    return lessThan;
}

VTK_TEST_FN(TestPKUtils059_BCurveEllipseTAETangentPositions)
{
    //Similar to the previous test but both vertices are now at tangent postions so check 2 vertices are returned
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Conics/ConicEllipseTAEAtBothEndsSharpRels.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,477);
    int edge2 = EdgeOfId(vbody,356);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 2);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 2);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 2);
    for ( size_t i = 0; i < 2; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils060_BCurveBCurveTangentPositions)
{
    //Compute tangent positions between two curves which have non-tangent intersections and a touching non-vertex position. This touching
    //position is on each curve at the middle of the parameter range but Parasolid does not return this parameter as an intersecting
    //position so we miss the tangency. Parasolid PR 9232933 written
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/BcurveBCurveTAEAtBothEndsAndCrossingsAndTouching.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Get the tangents and positions at parameter 0.5 as this is where D-Cubed thinks there is a satisfied tangency - compare tangents
    //and positions to Resnor to check we have a definite tangent intersecting position
    Vec midParamPos1,midParamPos2,midParamTan1,midParamTan2;
    PKUtils().CurveEvaluateAtParam(curve1,0.5,&midParamPos1,&midParamTan1);
    PKUtils().CurveEvaluateAtParam(curve2,0.5,&midParamPos2,&midParamTan2);
    Dir midParamTans [2] = {Normal(midParamTan1),Normal(midParamTan2)};
    CheckM(TolerantEqual(midParamPos1,midParamPos2,Resnor));
    CheckM(midParamTans[0].IsParallel(midParamTans[1]));

    //There should be 7 intersections and the fourth of these corresponds to the one which should be at parameter 0.5 (it's the closest
    //to that position anyway)
    VecArray intersections;
    DoubleVector params1,params2;
    PKUtils().CurveComputeIntersectionsWithCurve(curve1,curve2,0,0,0,0,&intersections,&params1,&params2);
    CheckM(intersections.size() == 7 && params1.size() == 7 && params2.size() == 7);

    Vec pkMidParamPos = intersections[3];
    Vec diffVec = midParamPos1 - pkMidParamPos;
    double diffLen = diffVec.Mag();

    //Three tangent positions are expected - 2 at the shared vertices and one touching position
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 3);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 3);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 3);
    for ( size_t i = 0; i < 3; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils061_BCurveBCurveTangentPositions)
{
    //Compute tangent positions between two curves which have non-tangent intersections - simpler case than the previous test as
    //there's no tangent touching position
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/BcurveBCurveTAEAtBothEndsAndCrossings.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Two tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 2);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 2);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 2);
    for ( size_t i = 0; i < 2; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils062_EllipseEllipseTangentPositions)
{
    //Compute tangent positions between two ellipses where 2 tangencies exist but only one of them is at a shared vertex
    //and there's no tangent touching position
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/EllipseEllipseTangentAndTAE.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,175);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Two tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 2);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 2);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 2);
    for ( size_t i = 0; i < 2; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils063_LineBCurveTangentPositions)
{
    //Compute tangent positions for line/bcurve which are tangent at the shared vertices and have a touching tangent position. This
    //tocuhing position is on each curve but the intersection returned by Parasolid is more than session precision away from
    //this point - see Parasolid PR 9232933
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/LineTAEToBCurveAtBothEnds4.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    Vec linePos;
    Dir lineDir;
    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1,0,&linePos,&lineDir);
    PKUtils().GetGeo(edge2,&curve2);

    //Evaluate the bcurve at 0.5 as that is where D-Cubed thinks the tangency is
    Vec midParamPos,midParamTan;
    PKUtils().CurveEvaluateAtParam(curve2,0.5,&midParamPos,&midParamTan);

    //Check the midParam position is on the line
    CheckM(GeomUtils::IsPointOnLine(midParamPos,linePos,lineDir));

    //3 intersections exist and the second of these is closest to the parameter 0.5 position
    VecArray intersections;
    DoubleVector params1,params2;
    PKUtils().CurveComputeIntersectionsWithCurve(curve1,curve2,0,0,0,0,&intersections,&params1,&params2);
    CheckM(intersections.size() == 3 && params1.size() == 3 && params2.size() == 3);

    Vec pkMidParamPos = intersections[1];
    Vec diffVec = midParamPos - pkMidParamPos;
    double diffLen = diffVec.Mag();

    //Three tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 3);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 3);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 3);
    for ( size_t i = 0; i < 3; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils064_EllipseBCurveTangentPositions)
{
    //Compute tangent positions between an ellipse and bcurve where there are also non-tangent crossing positions
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/EllipseBCurveTAEAtBothEnds3.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Two tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 2);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 2);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 2);
    for ( size_t i = 0; i < 2; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils065_EllipseBCurveTangentPositions)
{
    //Compute tangent positions between an ellipse and bcurve where there is also a tangent touching position
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/EllipseBCurveTAEAtBothEnds4.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Three tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 3);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 3);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 3);
    for ( size_t i = 0; i < 3; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils066_CircleBCurveTangentPositions)
{
    //Compute tangent positions between an circle and bcurve where there are also non-tangent tangent crossing positions
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/CircleTAEToBCurveAtBothEnds2.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Two tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 2);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 2);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 2);
    for ( size_t i = 0; i < 2; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils067_EllipseBCurveTangentPositions)
{
    //Compute tangent positions between a circle and bcurve where there is also a tangent touching position
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/CircleTAEToBCurveAtBothEnds3.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody,6);
    int edge2 = EdgeOfId(vbody,11);

    int curve1 = 0;
    int curve2 = 0;
    PKUtils().GetGeo(edge1,&curve1);
    PKUtils().GetGeo(edge2,&curve2);

    //Three tangent positions are expected
    VecArray tangentPositions1;
    PKUtils().CurvesGetTangentPositions(curve1,curve2,tangentPositions1);
    CheckM(tangentPositions1.size() == 3);

    VecArray tangentPositions2;
    PKUtils().CurvesGetTangentPositions(curve2,curve1,tangentPositions2);
    CheckM(tangentPositions2.size() == 3);

    std::set<Vec,TestPositionLessThan> positions;
    positions.insert(tangentPositions1.begin(),tangentPositions1.end());
    CheckM(positions.size() == 3);
    for ( size_t i = 0; i < 3; ++i )
    {
        CheckM(positions.find(tangentPositions2[i]) != positions.end());
    }

    return true;
}

VTK_TEST_FN(TestPKUtils068_OffsetCurveFindParamForPos)
{
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetOfBCurveLiesOnEdge.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    int edge1 = EdgeOfId(vbody, 14);

    PK_CURVE_t offsetCurve = PKUtils().EdgeGetCurve(edge1);
    CheckTagM(offsetCurve);

    double param1 = 0;
    bool found1 = PKUtils().CurveGetParamForPos(offsetCurve, Vec(7.047318770481856, 1.821859403546089, 0), &param1);

    double param2 = 0;
    bool found2 = PKUtils().CurveGetParamForPos(offsetCurve, Vec(7.047318770481856, 1.8218594035460889, 0), &param2);

    return true;
}

bool TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(PK_CURVE_t offsetCurve)
{
    PKUtils pkUtils;
    PK_CURVE_t baseCurve = 0;
    double offsetDistance = 0;
    OffsetSideUnit offsetSide;
    pkUtils.OffsetCurveData(offsetCurve, &baseCurve, &offsetDistance, &offsetSide);
    CheckTagM(baseCurve);
    CheckM(offsetDistance > 0);

    bool isPeriodic = pkUtils.CurveIsPeriodic(baseCurve);

    PK_INTERVAL_t interval;
    PKUtils().CurveGetInterval(offsetCurve, interval);
    int count = 100;
    double inc = (interval.value[1] - interval.value[0]) / count;
    for (int i = 0; i <= count; i++)
    {
        double param = interval.value[0] + inc * i;

        //Test we can evaluate the position on the offset curve, and 
        //parameterise the position to get the same parameter.
        VecArray vecs;
        CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, param, 1, vecs));
        double param2;
        CheckM(pkUtils.CurveGetParamForPos(offsetCurve, vecs[0], &param2));
        if(isPeriodic && i==count)
            CheckM(ResEqual(param2, interval.value[0]));//Expect the lower bound 
        else
            CheckM(ResEqual(param, param2));

        //Test evaluated tangent and curvature
        Vec pos, tan, basePos, baseTan;
        double offsetCurvature, baseCurvature;
        CheckM(pkUtils.CurveEvaluateAtParam(baseCurve, param, &basePos, &baseTan, &baseCurvature));
        CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, param, &pos, &tan, &offsetCurvature));
        CheckM(ResEqual(pos, vecs[0]));
        CheckM(ResEqual(tan, vecs[1]));
        CheckM(ResEqual(offsetDistance, (pos - basePos).Mag()));

        vecs.clear();
        CheckM(pkUtils.CurveEvaluateAtParam(baseCurve, param, 2, vecs));

        if (baseCurvature == 0)
        {
            CheckM(offsetCurvature == 0);
        }
        else
        {
            bool secondDerPointsToLeft = GeomUtils::AreCloserToParallelThanAntiParallel(Normal(vecs[2]), Normal(Dir::ZDir() * Normal(vecs[1])));
            bool expectHigherCurvature = (secondDerPointsToLeft == (offsetSide == OffsetSideUnit::Left));

            if (expectHigherCurvature)
                CheckM(ResEqual(1 / baseCurvature - 1 / offsetCurvature, offsetDistance));
            else
                CheckM(ResEqual(1 / baseCurvature - 1 / offsetCurvature, -offsetDistance));
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils068_OffsetCurveFindParamForPos02)
{
    //Test parameterisation and evaluation of offset curves on a curly bcurve.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/swissroll_offsetcurves.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    PK_EDGE_t offsetCurveEdge1 = EdgeOfId(vbody, 123);
    CheckTagM(offsetCurveEdge1);
    PK_CURVE_t offsetCurve1 = PKUtils().EdgeGetCurve(offsetCurveEdge1);
    PK_EDGE_t offsetCurveEdge2 = EdgeOfId(vbody, 146);
    CheckTagM(offsetCurveEdge2);
    PK_CURVE_t offsetCurve2 = PKUtils().EdgeGetCurve(offsetCurveEdge2);

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve1));

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve2));

    return true;
}

VTK_TEST_FN(TestPKUtils068_OffsetCurveFindParamForPos03)
{
    //Test parameterisation and evaluation of offset curves on two open bcurves.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/doubly_tan_between_offsetcurves_not_satisfied.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    PK_EDGE_t offsetCurveEdge1 = EdgeOfId(vbody, 1474);
    CheckTagM(offsetCurveEdge1);
    PK_CURVE_t offsetCurve1 = PKUtils().EdgeGetCurve(offsetCurveEdge1);
    PK_EDGE_t offsetCurveEdge2 = EdgeOfId(vbody, 1993);
    CheckTagM(offsetCurveEdge2);
    PK_CURVE_t offsetCurve2 = PKUtils().EdgeGetCurve(offsetCurveEdge2);

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve1));

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve2));

    return true;
}

VTK_TEST_FN(TestPKUtils068_OffsetCurveFindParamForPos04)
{
    //Test parameterisation and evaluation of offset curves on a periodic bcurve.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/offset_curves_on_periodic_bcurve.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    PK_EDGE_t offsetCurveEdge1 = EdgeOfId(vbody, 34);
    CheckTagM(offsetCurveEdge1);
    PK_CURVE_t offsetCurve1 = PKUtils().EdgeGetCurve(offsetCurveEdge1);
    PK_EDGE_t offsetCurveEdge2 = EdgeOfId(vbody, 21);
    CheckTagM(offsetCurveEdge2);
    PK_CURVE_t offsetCurve2 = PKUtils().EdgeGetCurve(offsetCurveEdge2);

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve1));

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve2));

    return true;
}

VTK_TEST_FN(TestPKUtils068_OffsetCurveFindParamForPos05)
{
    //Test parameterisation and evaluation of offset curves on the linear extensions

    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/circ_tan_to_extended_offset_curve.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    PK_EDGE_t offsetCurveEdge1 = EdgeOfId(vbody, 54);
    CheckTagM(offsetCurveEdge1);
    PK_CURVE_t offsetCurve1 = PKUtils().EdgeGetCurve(offsetCurveEdge1);
    PK_EDGE_t offsetCurveEdge2 = EdgeOfId(vbody, 135);
    CheckTagM(offsetCurveEdge2);
    PK_CURVE_t offsetCurve2 = PKUtils().EdgeGetCurve(offsetCurveEdge2);

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve1));

    CheckM(TestPKUtils068_OffsetCurveParameteriseEvaluate_Base(offsetCurve2));

    return true;
}

VTK_TEST_FN(TestPKUtils068_OffsetCurveFindParamForPos06)
{
    //Test to show inaccurate output from PK_GEOM_range_local_vector. PR 9288756 has been created.
    // PS says that the function is behaving per design so we will work around it (Yanong)
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/swissroll_offsetcurves.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    Vec pos(-0.34310378822688836, -1.8181563400731819, 0.0);
    int baseCurve = CurveOfId(vbody, 118);

    double p = 0.56;
    PK_VECTOR_t pkVecs[2];
    PK_CURVE_eval(baseCurve, p, 1, pkVecs);

    ParasolidJournalUtils::StartParasolidJournal("PK_GEOM_range_local_vector", true);
    PK_GEOM_range_local_vector_o_t geomRangeOps;
    PK_GEOM_range_local_vector_o_m(geomRangeOps);
    geomRangeOps.have_tolerance = PK_LOGICAL_true;
    geomRangeOps.tolerance = 1e-8;
    geomRangeOps.opt_level = PK_range_opt_accuracy_c;
    int nResults;
    PK_range_1_r_t * rangeData = NULL;
    PK_ERROR_t pkerror = PK_GEOM_range_local_vector(baseCurve, pos.PkVector(), &geomRangeOps, &nResults, &rangeData);
    CheckM(pkerror == PK_ERROR_no_errors);
    ParasolidJournalUtils::StopParasolidJournal();

    double p2 = rangeData[0].end.parameters[0];

    //Evaluate at the nearest approach parameter.
    //We expect the sample point lies on the normal at this parameter.
    ParasolidJournalUtils::StartParasolidJournal("PK_CURVE_eval", true);
    PK_VECTOR_t pkVecs2[2];
    PK_CURVE_eval(baseCurve, p2, 1, pkVecs2);
    ParasolidJournalUtils::StopParasolidJournal();

    bool b = ResEqual(Vec(pkVecs[0].coord), Vec(pkVecs2[0].coord));

    Dir tanDir = Normal(Vec(pkVecs2[1].coord));
    //Normal direction at the found position
    Dir normDir = Normal(tanDir * Dir::ZDir());
    //Direction from the found position to the given sample position.
    Dir normDir2 = Normal(pos - Vec(pkVecs2[0].coord));
    
    //These directions should be the same, but PS doesn't guarantee it so we will be expecting them not to match here
    CheckM(!normDir.IsParallel(normDir2));
    PK_MEMORY_free(rangeData);

    return true;
}

VTK_TEST_FN(TestPKUtils069_OffsetCurveEdgeContainsPoint)
{
    //Test EdgeContainsPoint for offset curve edges.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/offset_edges.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    IntArray edges;
    edges.push_back(EdgeOfId(vbody, 1395)); //A simple offset curve on a bcurve
    edges.push_back(EdgeOfId(vbody, 518));  //A periodic offset curve on a periodic bcurve
    edges.push_back(EdgeOfId(vbody, 1146)); //An extended offset curve
    edges.push_back(EdgeOfId(vbody, 846));  //A non-periodic offset curve on ellipse, whose interval spans into the second period.

    PKUtils pkUtils;
    for (PK_EDGE_t offsetCurveEdge : edges)
    {
        PK_CURVE_t offsetCurve = pkUtils.EdgeGetCurve(offsetCurveEdge);
        CheckTagM(offsetCurve);

        IntArray vertices;
        pkUtils.EdgeGetVertices(offsetCurveEdge, vertices);

        PK_CURVE_t baseCurve = 0;
        double offsetDistance = 0;
        OffsetSideUnit offsetSide;
        pkUtils.OffsetCurveData(offsetCurve, &baseCurve, &offsetDistance, &offsetSide);
        CheckTagM(baseCurve);
        CheckM(offsetDistance > 0);

        bool isPeriodic = pkUtils.CurveIsPeriodic(baseCurve);

        double interval[2];
        PKUtils().EdgeGetInterval(offsetCurveEdge, interval);
        int count = 100;
        double inc = (interval[1] - interval[0]) / count;
        for (int i = 0; i <= count; i++)
        {
            double param = interval[0] + inc * i;

            VecArray vecs;
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, param, 1, vecs));

            PK_ENTITY_t coiTopol = 0;
            CheckM(pkUtils.EdgeContainsPoint(offsetCurveEdge, vecs[0], &coiTopol));
            if (i == 0 && vertices.size() == 2)
                CheckM(coiTopol == vertices[0]);
            else if (i == count && vertices.size() == 2)
                CheckM(coiTopol == vertices[1]);
            else
                CheckM(coiTopol == offsetCurveEdge);
        }
    }

    return true;
}

VTK_TEST_FN(TestPKUtils070_OffsetCurveEdgeGetTangentDirAtParam)
{
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/offset_edges.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    IntArray edges;
    edges.push_back(EdgeOfId(vbody, 1395)); //A simple offset curve on a bcurve
    edges.push_back(EdgeOfId(vbody, 518));  //A periodic offset curve on a periodic bcurve
    edges.push_back(EdgeOfId(vbody, 1146)); //An extended offset curve
    edges.push_back(EdgeOfId(vbody, 846));  //A non-periodic offset curve on ellipse, whose interval spans into the second period.

    PKUtils pkUtils;
    for (PK_EDGE_t offsetCurveEdge : edges)
    {
        PK_CURVE_t offsetCurve = pkUtils.EdgeGetCurve(offsetCurveEdge);
        CheckTagM(offsetCurve);

        IntArray vertices;
        pkUtils.EdgeGetVertices(offsetCurveEdge, vertices);

        PK_CURVE_t baseCurve = 0;
        double offsetDistance = 0;
        OffsetSideUnit offsetSide;
        pkUtils.OffsetCurveData(offsetCurve, &baseCurve, &offsetDistance, &offsetSide);
        CheckTagM(baseCurve);
        CheckM(offsetDistance > 0);

        bool isPeriodic = pkUtils.CurveIsPeriodic(baseCurve);

        double interval[2];
        PKUtils().EdgeGetInterval(offsetCurveEdge, interval);
        int count = 100;
        double inc = (interval[1] - interval[0]) / count;
        for (int i = 0; i <= count; i++)
        {
            double param = interval[0] + inc * i;

            VecArray vecs;
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, param, 1, vecs));

            //The tangent direction is the same as the first derivative direction.
            Dir tangent;
            Vec pos;
            CheckM(pkUtils.EdgeGetTangentDirectionAtParameter(offsetCurveEdge, param, tangent, &pos));
            CheckM(tangent.IsAligned(Normal(vecs[1])));
            CheckM(ResEqual(pos, vecs[0]));
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils071_OffsetCurveEdgeFindEndTangents)
{
    //Test EdgeFindEndTangents for offset curve edges.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/offset_edges.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    IntArray edges;
    edges.push_back(EdgeOfId(vbody, 1395)); //A simple offset curve on a bcurve
    edges.push_back(EdgeOfId(vbody, 518));  //A periodic offset curve on a periodic bcurve
    edges.push_back(EdgeOfId(vbody, 1146)); //An extended offset curve
    edges.push_back(EdgeOfId(vbody, 846));  //A non-periodic offset curve on ellipse, whose interval spans into the second period.

    PKUtils pkUtils;
    for (PK_EDGE_t offsetCurveEdge : edges)
    {
        PK_CURVE_t offsetCurve = pkUtils.EdgeGetCurve(offsetCurveEdge);
        CheckTagM(offsetCurve);

        IntArray vertices;
        pkUtils.EdgeGetVertices(offsetCurveEdge, vertices);

        bool isPeriodic = vertices.empty();

        Dir startDir, endDir;
        Vec startPos, endPos;
        if (isPeriodic)
        {
            CheckM(!pkUtils.EdgeFindEndTangents(offsetCurveEdge, startDir, endDir, &startPos, &endPos));
        }
        else
        {
            CheckM(pkUtils.EdgeFindEndTangents(offsetCurveEdge, startDir, endDir, &startPos, &endPos));
            double interval[2];
            PKUtils().EdgeGetInterval(offsetCurveEdge, interval);
            VecArray vecs;
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, interval[0], 1, vecs));
            //The tangent direction is the same as the first derivative direction.
            CheckM(startDir.IsAligned(Normal(vecs[1])));
            CheckM(ResEqual(startPos, vecs[0]));
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, interval[1], 1, vecs));
            //The tangent direction is the same as the first derivative direction.
            CheckM(endDir.IsAligned(Normal(vecs[1])));
            CheckM(ResEqual(endPos, vecs[0]));

            //test again after inverting the edge sense.
            pkUtils.TopolDetachGeom(offsetCurveEdge);
            bool sense = false;
            pkUtils.EdgeAttachCurve(offsetCurveEdge, offsetCurve, &sense);
            CheckM(pkUtils.TopolGetSense(offsetCurveEdge) == false);

            CheckM(pkUtils.EdgeFindEndTangents(offsetCurveEdge, startDir, endDir, &startPos, &endPos));
            PKUtils().EdgeGetInterval(offsetCurveEdge, interval);
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, interval[1], 1, vecs));
            //The tangent direction is the anti-aligned with the first derivative direction at the upper bound.
            CheckM(startDir.IsAligned(-Normal(vecs[1])));
            CheckM(ResEqual(startPos, vecs[0]));
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, interval[0], 1, vecs));
            //The tangent direction is the anti-aligned with  the first derivative direction at the lower bound.
            CheckM(endDir.IsAligned(-Normal(vecs[1])));
            CheckM(ResEqual(endPos, vecs[0]));
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils072_OffsetCurveGetClosestParamsForPos)
{
    //Test CurveGetClosestParamsForPos for offset curves.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/OffsetCurves/offset_edges.xml").c_str());
    CheckM(interaction != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction), vbodies);
    VTKBody vbody = vbodies[0];

    IntArray edges;
    //edges.push_back(EdgeOfId(vbody, 1395)); //A simple offset curve on a bcurve
    edges.push_back(EdgeOfId(vbody, 518));  //A periodic offset curve on a periodic bcurve
    edges.push_back(EdgeOfId(vbody, 1146)); //An extended offset curve
    edges.push_back(EdgeOfId(vbody, 846));  //A non-periodic offset curve on ellipse, whose interval spans into the second period.

    PKUtils pkUtils;
    for (PK_EDGE_t offsetCurveEdge : edges)
    {
        PK_CURVE_t offsetCurve = pkUtils.EdgeGetCurve(offsetCurveEdge);
        CheckTagM(offsetCurve);

        PK_CURVE_t baseCurve = 0;
        double offsetDistance = 0;
        OffsetSideUnit offsetSide;
        pkUtils.OffsetCurveData(offsetCurve, &baseCurve, &offsetDistance, &offsetSide);
        CheckTagM(baseCurve);
        CheckM(offsetDistance > 0);

        bool baseIsPeriodic = pkUtils.CurveIsPeriodic(baseCurve);

        double interval[2];
        PKUtils().EdgeGetInterval(offsetCurveEdge, interval);
        int count = 100;
        double inc = (interval[1] - interval[0]) / count;
        for (int i = 0; i <= count; i++)
        {
            double param = interval[0] + inc * i;

            VecArray vecs;
            CheckM(pkUtils.CurveEvaluateAtParam(offsetCurve, param, 1, vecs));
            //move the position along the normal a bit (so we should find the same or equivalent param)
            Vec pos = vecs[0] + (Normal(vecs[1] * Dir::ZDir())) * 0.01;
            double foundParam = 0;
            CheckM(pkUtils.CurveGetClosestParamsForPos(offsetCurve, pos, &foundParam));
            if (baseIsPeriodic)
            {
                if (i == 0 || i == count)
                {
                    //For the first/last point, if the offset curve is also periodic, then we will get the 
                    //start or end paramter of the interval; otherwise, the parameter will always be in the
                    //first period where the offset curve's interval may span into the second.
                    CheckM(ResEqual(foundParam, interval[0]) || ResEqual(foundParam, interval[1]) || ResEqual(foundParam + TWO_PI, interval[1]));
                }
                else
                {
                    //The parameter will always be in the first period where the curve's interval may span into the second
                    CheckM(ResEqual(foundParam, param) || ResEqual(foundParam + TWO_PI, param));
                }
            }
            else
            {
                CheckM(ResEqual(foundParam, param));
            }
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils073_CompareFacetBodies)
{
    //Compare facet bodies
    InteractionTag interaction1 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/FacetsBodies/cube.xml").c_str());
    InteractionTag interaction2 = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/FacetsBodies/cube_with_blends.xml").c_str());

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction1),vbodies);
    CheckM(vbodies.size() == 2);

    CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1),vbodies[0],vbodies[0]));
    CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1),vbodies[0],vbodies[1]) == false);

    PK_DEBUG_BODY_compare_o_t opts;
    PK_DEBUG_BODY_compare_o_m(opts);
    PK_DEBUG_BODY_compare_r_t results;
    ResultType result = ResultTypeOk;
    CheckM(VTKBody::DebugBodyCompare(GetVersion(interaction1),vbodies[0],vbodies[0],&opts,&results,&result) == true && result == ResultTypeOk);
    PK_DEBUG_BODY_compare_r_f(&results);
    CheckM(VTKBody::DebugBodyCompare(GetVersion(interaction1),vbodies[0],vbodies[1],&opts,&results,&result) == false && result == ResultTypeOk);
    PK_DEBUG_BODY_compare_r_f(&results);

    return true;
}

VTK_TEST_FN(TestPKUtils074_ChamferEdges)
{
    // This code uses PKUtils().ChamferEdges to put a 4mm chamfer (quite large) on the rims of the central hole in the stand model.
    // This shows that the temporary memory is freed properly by the util code.

    VTKBody vbody = TestUtilLoadSingleBody("vtk_part_data:Misc/New Stand_v20.x_t");
    CheckVTKBodyM(vbody);

    IntSet edges;
    edges.insert(EdgeOfId(vbody, 915));
    edges.insert(EdgeOfId(vbody, 913));

    bool ok = PKUtils().ChamferEdges(TestGetCurrentVersion(), edges, 0.004);
    CheckM(ok);

    bool different = TestUtilAreBodiesDifferent(TestGetCurrentVersion(), vbody, "vtk_part_data:Misc/New Stand_v20_chamfered_hole.x_t");
    CheckM(!different);

    return true;
}

VTK_TEST_FN(TestPKUtils075_CurveMakeWirebody_AvoidTolerantVertices)
{
    // This test shows that we don't allow tolerant vertices when creating a wirebody from curves via CurveMakeWirebody.

    IntArray        curves;
    DoublePairArray intervals;
    PK_BODY_t       wirebody;
    int             numberOfTolerantVertices;
    IntArray        tolerantVertices;
    VTK::Version    version;
    IntArray        vertices;

    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::XDir()));
    curves.push_back(PKUtils().CreateLine(Vec::Origin(), Dir::YDir()));

    // Parasolid sets a default tolerance of 10^-6, and the distance between the vertices at the ends of these two lines is less than that, but more than resabs = 10^-8
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(1e-7, 1));

    version = GetCurrentVersion();
    wirebody = PKUtils().CurveMakeWirebody(version, curves, intervals);
    VTKBody vbody(wirebody);

    numberOfTolerantVertices = vbody.GetTolerantVertices(version, &tolerantVertices);
    CheckM(numberOfTolerantVertices == 0);

    vbody.GetVertices(GetCurrentVersion(), &vertices); 
    CheckM(vertices.size() == 4);

    return true;
}

VTK_TEST_FN(TestPKUtils076_MergeNonMergeableVertexAttributes)
{
    //Test the Parasolid behaviour of merging two non-mergeable vertices to check we don't get duplicated
    //attributes
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/WireBodyYJunction2.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);

    //Add attributes to the vertices at one of the Y-junctions
    int vs [3] = {VertexOfId(vbodies[0],2),VertexOfId(vbodies[0],20),VertexOfId(vbodies[0],36)};
    for ( int i = 0; i < 3; ++i )
    {
        VertexSetIsMergeable(interaction,vs[i],false);
    }

    //Merge the vertices
    IntIntPairArray mergePairs;
    mergePairs.push_back(IntIntPair(vs[0],vs[1]));
    mergePairs.push_back(IntIntPair(vs[0],vs[2]));
    CheckM(PKUtils().BodyKnitAtTopols(interactionPtr->GetVersion(),vbodies[0],mergePairs));

    //Check that there is one non-mergeable attribute on the body
    std::vector<int> attribs;
    CheckM(vbodies[0].AskAllAttribs(interactionPtr->GetVersion(),interactionPtr->GetNonMergeableVertexAttDefTag(),attribs) == 1);
    CheckM(PKUtils().AttribGetOwner(attribs[0]) == vs[0]);

    return true;
}

bool TestPKUtils077_model( int index )
{
    //Test the Parasolid behaviour of merging a vertex having a non-mergeable attribute with a vertex not having this attribute
    //to check that the attribute does not survive
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/WireBodyYJunction2.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);

    //Add attribute to one of the vertices at one of the Y-junctions
    int vs [3] = {VertexOfId(vbodies[0],2),VertexOfId(vbodies[0],20),VertexOfId(vbodies[0],36)};
    VertexSetIsMergeable(interaction,vs[index],false,&result);
    CheckM(result == ResultTypeOk);

    //Merge the vertices
    IntIntPairArray mergePairs;
    mergePairs.push_back(IntIntPair(vs[0],vs[1]));
    mergePairs.push_back(IntIntPair(vs[0],vs[2]));
    CheckM(PKUtils().BodyKnitAtTopols(interactionPtr->GetVersion(), vbodies[0],mergePairs));

    //Check that there are no non-mergeable attributes on the body
    std::vector<int> attribs;
    CheckM(vbodies[0].AskAllAttribs(interactionPtr->GetVersion(),interactionPtr->GetNonMergeableVertexAttDefTag(),attribs) == 0);

    return true;
}

VTK_TEST_FN(TestPKUtils077a_CheckMergingAttributeOnNonMergeableVertex)
{
    //Put the attribute on the first vertex
    CheckM(TestPKUtils077_model(0));

    return true;
}

VTK_TEST_FN(TestPKUtils077b_CheckMergingAttributeOnNonMergeableVertex)
{
    //Put the attribute on the second vertex
    CheckM(TestPKUtils077_model(1));

    return true;
}

VTK_TEST_FN(TestPKUtils077c_CheckMergingAttributeOnNonMergeableVertex)
{
    //Put the attribute on the third vertex
    CheckM(TestPKUtils077_model(2));

    return true;
}

VTK_TEST_FN(TestPKUtils078_TestPKPerformanceHammer)
{
    //Test case for tracking performance of PK model query functions which have increased slightly
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/ThousandEdgesAndVertices.xml").c_str());
    CheckM(interaction != 0);

    double linePos [3] = {0.0,0.0,0.0};
    double lineDir [3] = {0.0,1.0,0.0};
    OperationSetDragLinear(interaction,linePos,lineDir,linePos);

    CheckM(OperationConfigure(interaction));

    OperationStart(interaction);

    double dragPos [3] = {0.0,0.01,0.0};

    OperationStepDragLinear(interaction,dragPos);

    CheckM(OperationUpdate(interaction) == UpdateApplied);

    VTKBodyArray vbodies;
    VTKBody::SessionGetBodies(GetVersion(interaction),vbodies);
    CheckM(vbodies.size() == 1);

    double times [3] = {0,0,0};
    double nums [3] = {0,0,0};
    IntArray edges;
    IntArray firstVertex;
    IntArray secondVertex;

    vbodies[0].GetEdges(GetVersion(interaction),&edges);
    int numEdges = (int)edges.size();
    for ( int l = 0; l < numEdges; ++l )
    {
        IntArray vertices;
        PKUtils().EdgeGetVertices(edges[l],vertices);
        int numVertices = (int)vertices.size();
        if ( numVertices == 0 )
        {
            firstVertex.push_back(0);
            secondVertex.push_back(0);
        }
        else if ( numVertices == 1 )
        {
            firstVertex.push_back(vertices[0]);
            secondVertex.push_back(0);
        }
        else
        {
            firstVertex.push_back(vertices[0]);
            secondVertex.push_back(vertices[1]);
        }
    }

    PK_LOGICAL_t is = PK_LOGICAL_false;
    PK_POINT_t point = 0;
    int numIterations = 100000;

    UtilTimer timer;
    for ( int i = 0; i < numIterations; ++i )
    {
        for ( int k = 0; k < numEdges; ++k )
        {
            int edge = edges[k];

            PK_ENTITY_is(edge,&is);

            PK_CLASS_t pkClass = PK_CLASS_point;
            PK_ENTITY_ask_class(edge,&pkClass);

            int v1 = firstVertex[k];
            if ( v1 != 0 )
            {
                PK_ENTITY_is(v1,&is);
                PK_VERTEX_ask_point(firstVertex[k],&point);
            }

            int v2 = secondVertex[k];
            if ( v2 != 0 )
            {
                PK_ENTITY_is(v2,&is);
                PK_VERTEX_ask_point(secondVertex[k],&point);
            }
        }
    }

    //Average increased from 4e-7s with Parasolid 31_1_232
    //printf("\n%g",timer.Time()/(numEdges*numIterations));

    return true;
}

VTK_TEST_FN(TestPKUtils079_TestPVMPerformanceHammer)
{
    //Test case for slight performance slow-downs of PVM queries - this really needs to be run with 
    //30000 iterations to really see the issue but left at 3000 to avoid a 30 minute test
    VTKBody vbody= TestUtilLoadSingleBody("vtk_part_data:Ricoh Variations/Ricoh Case 7.x_t");

    IntArray faces;
    vbody.GetFaces(GetCurrentVersion(),&faces);
    int numFaces = (int)faces.size();
    int numIterations = 3000;
    UtilTimer timer;
    for ( int j = 0; j < numIterations; ++j )
    {
        for ( size_t i = 0; i < numFaces; ++i )
        {
            int face = faces[i];
            PK_LOGICAL_t isPKTopol = PK_LOGICAL_false;
            PK_ENTITY_is_topol(face, &isPKTopol);
            int nFeatures = 0;
            PK_TOPOL_t* pFeatures = NULL;
            PVM_TOPOL_ask_feats(face, &nFeatures, &pFeatures);
        }
    }

    double time = timer.Time();
    //printf("\n***** %g (%g)\n",time,time/(numFaces*numIterations));

    return true;
}

VTK_TEST_FN(TestPKUtils080a_CheckGetContinuityChecking001){
    //Test get continuity checking function by loading an interaction with a known check-continuity setting
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurve_closed.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    PK_LOGICAL_t contChecking;
    PK_ERROR_code_t pkError = PK_SESSION_ask_check_continuity(&contChecking);
    CheckM(pkError == PK_ERROR_no_errors);
    CheckM(contChecking == PK_LOGICAL_true);

    return true;
}

VTK_TEST_FN(TestPKUtils080b_CheckGetContinuityChecking002)
{
    //Test get continuity checking function by loading an interaction with a known check-continuity setting
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/g0_bcurve_with_teardrop2.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    PK_LOGICAL_t contChecking;
    PK_ERROR_code_t pkError = PK_SESSION_ask_check_continuity(&contChecking);
    CheckM(pkError == PK_ERROR_no_errors);
    CheckM(contChecking == PK_LOGICAL_false);

    return true;
}

VTK_TEST_FN(TestPKUtils080c_CheckSetContinuityChecking001)
{
    //Test set continuity checking function and check result using get function
    VTK::ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteraction();
    CheckM(interaction != 0);

    //Initial default value should be true
    PK_LOGICAL_t contChecking;
    PK_ERROR_code_t pkError = PK_SESSION_ask_check_continuity(&contChecking);
    CheckM(pkError == PK_ERROR_no_errors);
    CheckM(contChecking == PK_LOGICAL_true);

    //Set to false and check
    pkError = PK_SESSION_set_check_continuity(PK_LOGICAL_false);
    CheckM(pkError == PK_ERROR_no_errors);
    pkError = PK_SESSION_ask_check_continuity(&contChecking);
    CheckM(pkError == PK_ERROR_no_errors);
    CheckM(contChecking == PK_LOGICAL_false);

    //Set back to true and check
    pkError = PK_SESSION_set_check_continuity(PK_LOGICAL_true);
    CheckM(pkError == PK_ERROR_no_errors);
    pkError = PK_SESSION_ask_check_continuity(&contChecking);
    CheckM(pkError == PK_ERROR_no_errors);
    CheckM(contChecking == PK_LOGICAL_true);

    return true;
}

VTK_TEST_FN(TestPKUtils081_CurveIsClosed001)
{
    //Test checking whether different curves are closed
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    VTKBody body = bodyArray[0];
    IntArray geoms;
    body.GetGeoms(interactionPtr->GetVersion(), geoms);
    CheckM(geoms.size() == 8);

    for (int g : geoms)
    {
        switch (PKUtils().EntityGetClass(g))
        {
        case EntityClass::Point :
            CheckM(PKUtils().CurveIsClosed(g) == false);
            break;
        case EntityClass::Line :
            CheckM(PKUtils().CurveIsClosed(g) == false);
            break;
        case EntityClass::Bcurve:
            CheckM(PKUtils().CurveIsClosed(g) == false);
            break;
        case EntityClass::Circle:
            CheckM(PKUtils().CurveIsClosed(g) == true);
            break;
        case EntityClass::Ellipse:
            CheckM(PKUtils().CurveIsClosed(g) == true);
            break;
        case EntityClass::Conic:
            CheckM(PKUtils().CurveIsClosed(g) == false);
            break;
        default:
            //Should not be here
            CheckM(false);
            break;
        }
    }

    return true;
}

VTK_TEST_FN(TestPKUtils082_RepairSplitEdgesForWirebodyCreation001)
{
    //Test the function RepairSplitEdgesForWirebodyCreation for various invalid inputs
    //and make sure it fails properly

    //Create a valid wirebody from an interaction
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interactionTag = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/virtual_bcurve_teardrop.xml").c_str());
    CheckM(interactionTag != 0);
    Interaction* interaction = TagManager::FindInteractionFromTag(interactionTag);
    CheckM(interaction != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    IntArray geoms;
    bodyArray[0].GetGeoms(interaction->GetVersion(), geoms);
    CheckM(geoms.size() == 1);

    //Create the wirebody
    PK_CURVE_t curves[1];
    curves[0] = geoms[0];
    PK_INTERVAL_t bounds[1];
    bounds[0].value[0] = 0.0; bounds[0].value[1] = 1.0;
    PK_CURVE_make_wire_body_o_t makeWireBodyOptions;
    PK_CURVE_make_wire_body_o_m(makeWireBodyOptions);
    makeWireBodyOptions.want_edges = PK_LOGICAL_true;
    makeWireBodyOptions.want_indices = PK_LOGICAL_true;
    PK_BODY_t wireBody;
    int newEdgeCountPS = 0;
    PK_EDGE_t* newEdgesPS;
    int* edgeIndicesPS = NULL;
    PK_ERROR_code_t pkError = PK_CURVE_make_wire_body_2(1, curves, bounds, &makeWireBodyOptions, &wireBody, &newEdgeCountPS, &newEdgesPS, &edgeIndicesPS);
    CheckM(pkError == PK_ERROR_no_errors);

    //Supply bad geoms
    IntArray badGeoms;
    bool mergeEdgesSuccess = interaction->GetPKUtils().TestRepairSplitEdgesForWirebodyCreation(interaction->GetVersion(), badGeoms, wireBody, newEdgeCountPS, newEdgesPS, edgeIndicesPS);
    CheckM(mergeEdgesSuccess == false);

    //No wirebody
    mergeEdgesSuccess = interaction->GetPKUtils().TestRepairSplitEdgesForWirebodyCreation(interaction->GetVersion(), geoms, 0, newEdgeCountPS, newEdgesPS, edgeIndicesPS);
    CheckM(mergeEdgesSuccess == false);

    if (newEdgesPS)
        PK_MEMORY_free(newEdgesPS);
    if (edgeIndicesPS)
        PK_MEMORY_free(edgeIndicesPS);

    SetLiveInteraction(interactionTag);

    return true;
}

VTK_TEST_FN(TestPKUtils082_RepairSplitEdgesForWirebodyCreation002)
{
    //Test merging split edges produced when using PK functions to create a wirebody
    //from a closed non-periodic curve which is G0-continuous at the start/end point.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interactionTag = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/virtual_bcurve_teardrop.xml").c_str());
    CheckM(interactionTag != 0);
    Interaction* interaction = TagManager::FindInteractionFromTag(interactionTag);
    CheckM(interaction != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    IntArray geoms;
    bodyArray[0].GetGeoms(interaction->GetVersion(), geoms);
    CheckM(geoms.size() == 1);

    //Create the wirebody
    PK_CURVE_t curves[1];
    curves[0] = geoms[0];
    PK_INTERVAL_t bounds[1];
    bounds[0].value[0] = 0.0; bounds[0].value[1] = 1.0;
    PK_CURVE_make_wire_body_o_t makeWireBodyOptions;
    PK_CURVE_make_wire_body_o_m(makeWireBodyOptions);
    makeWireBodyOptions.want_edges = PK_LOGICAL_true;
    makeWireBodyOptions.want_indices = PK_LOGICAL_true;
    PK_BODY_t wireBody;
    int newEdgeCountPS = 0;
    PK_EDGE_t* newEdgesPS;
    int* edgeIndicesPS = NULL;
    PK_ERROR_code_t pkError = PK_CURVE_make_wire_body_2(1, curves, bounds, &makeWireBodyOptions, &wireBody, &newEdgeCountPS, &newEdgesPS, &edgeIndicesPS);
    CheckM(pkError == PK_ERROR_no_errors);

    //Check that 2 edges have been produced
    CheckM(newEdgeCountPS == 2);

    //Merge the edges
    IntArray mergedEdges;
    bool mergeSuccess = interaction->GetPKUtils().TestRepairSplitEdgesForWirebodyCreation(interaction->GetVersion(), geoms, wireBody, newEdgeCountPS, newEdgesPS, edgeIndicesPS, &mergedEdges);
    CheckM(mergeSuccess);

    //Check there is now a single edge
    CheckM((int)mergedEdges.size() == 1);

    if (newEdgesPS)
        PK_MEMORY_free(newEdgesPS);
    if (edgeIndicesPS)
        PK_MEMORY_free(edgeIndicesPS);

    SetLiveInteraction(interactionTag);

    return true;
}

VTK_TEST_FN(TestPKUtils082_RepairSplitEdgesForWirebodyCreation003)
{
    //Test merging split edges produced when using PK functions to create a wirebody
    //from multiple closed non-periodic curves which are G0-continuous at their start/end points.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interactionTag = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/virtual_bcurve_2_teardrops.xml").c_str());
    CheckM(interactionTag != 0);
    Interaction* interaction = TagManager::FindInteractionFromTag(interactionTag);
    CheckM(interaction != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    IntArray geoms;
    bodyArray[0].GetGeoms(interaction->GetVersion(), geoms);
    CheckM(geoms.size() == 2);

    //Create the wirebody
    PK_CURVE_t curves[2];
    curves[0] = geoms[0]; curves[1] = geoms[1];
    PK_INTERVAL_t bounds[2];
    bounds[0].value[0] = 0.0; bounds[0].value[1] = 1.0;
    bounds[1].value[0] = 0.0; bounds[1].value[1] = 1.0;
    PK_CURVE_make_wire_body_o_t makeWireBodyOptions;
    PK_CURVE_make_wire_body_o_m(makeWireBodyOptions);
    makeWireBodyOptions.want_edges = PK_LOGICAL_true;
    makeWireBodyOptions.want_indices = PK_LOGICAL_true;
    PK_BODY_t wireBody;
    int newEdgeCountPS = 0;
    PK_EDGE_t* newEdgesPS;
    int* edgeIndicesPS = NULL;
    PK_ERROR_code_t pkError = PK_CURVE_make_wire_body_2(2, curves, bounds, &makeWireBodyOptions, &wireBody, &newEdgeCountPS, &newEdgesPS, &edgeIndicesPS);
    CheckM(pkError == PK_ERROR_no_errors);

    //Check that 4 edges have been produced, 2 per input curve
    CheckM(newEdgeCountPS == 4);

    //Merge the edges
    IntArray mergedEdges;
    bool mergeSuccess = interaction->GetPKUtils().TestRepairSplitEdgesForWirebodyCreation(interaction->GetVersion(), geoms, wireBody, newEdgeCountPS, newEdgesPS, edgeIndicesPS, &mergedEdges);
    CheckM(mergeSuccess);

    //Check there is now a single edge per curve (2 in total)
    CheckM((int)mergedEdges.size() == 2);

    //Check that the returned edges are in the same order as their corresponding input curves
    for (int i = 0; i < (int)mergedEdges.size(); ++i)
    {
        CheckM(interaction->GetPKUtils().EdgeGetCurve(mergedEdges[i]) == geoms[i]);
    }

    if (newEdgesPS)
        PK_MEMORY_free(newEdgesPS);
    if (edgeIndicesPS)
        PK_MEMORY_free(edgeIndicesPS);

    SetLiveInteraction(interactionTag);

    return true;
}

VTK_TEST_FN(TestPKUtils082_RepairSplitEdgesForWirebodyCreation004)
{
    //Test merging split edges produced when using PK functions to create a wirebody from a closed non-periodic
    //b-curve which is G0-continuous at the start/end point. The vertex created when splitting edges will be
    //shared with other lines and should be detached from the b-curve instead of just deleted.
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interactionTag = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/virtual_bcurve_teardrop_and_line3.xml").c_str());
    CheckM(interactionTag != 0);
    Interaction* interaction = TagManager::FindInteractionFromTag(interactionTag);
    CheckM(interaction != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interaction->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    IntArray geoms;
    bodyArray[0].GetGeoms(interaction->GetVersion(), geoms);
    CheckM(geoms.size() == 4);

    //Check all curves coincide at the same position (at param 0.5 of the b-curve)
    //This position is where a vertex will be created when the bcurve edges are split by PK_CURVE_make_wire_body_2
    for (GeometryTag gTag : geoms)
    {
        double paramToCheck;
        if (EntityGetGeometryType(interactionTag, gTag, &result) == GeometryTypeSplineCurve)
        {
            paramToCheck = 0.5;
        }
        else
        {
            CheckM(EntityGetGeometryType(interactionTag, gTag, &result) == GeometryTypeLine);
            paramToCheck = 0.0;
        }
        EvaluateData evalData;
        CurveEvaluateAtParameter(interactionTag, gTag, paramToCheck, TagNull, evalData, &result);
        Position posToCheck = evalData.GetPosition();
        Position posExpected = Position(0.0, 0.1, 0.0);
        CheckM(posToCheck == posExpected);
    }

    //Create the wirebody
    PK_CURVE_t curves[4];
    PK_INTERVAL_t bounds[4];
    for (int i = 0; i < 4; ++i)
    {
        curves[i] = geoms[i];
        bounds[i].value[0] = 0.0;
        bounds[i].value[1] = 1.0;
    }

    PK_CURVE_make_wire_body_o_t makeWireBodyOptions;
    PK_CURVE_make_wire_body_o_m(makeWireBodyOptions);
    makeWireBodyOptions.want_edges = PK_LOGICAL_true;
    makeWireBodyOptions.want_indices = PK_LOGICAL_true;
    makeWireBodyOptions.check = PK_LOGICAL_false;
    PK_BODY_t wireBody;
    int newEdgeCountPS = 0;
    PK_EDGE_t* newEdgesPS;
    int* edgeIndicesPS = NULL;

    makeWireBodyOptions.tolerance = Resabs;
    makeWireBodyOptions.allow_disjoint = PK_LOGICAL_true;
    makeWireBodyOptions.allow_general = PK_LOGICAL_true;

    PK_SESSION_set_general_topology(PK_LOGICAL_true);

    PK_ERROR_code_t pkError = PK_CURVE_make_wire_body_2(4, curves, bounds, &makeWireBodyOptions, &wireBody, &newEdgeCountPS, &newEdgesPS, &edgeIndicesPS);
    CheckM(pkError == PK_ERROR_no_errors);

    //Check that 5 edges have been produced, 2 for the b-curve and 1 for each line
    CheckM(newEdgeCountPS == 5);

    //Check edges associated with the vertices
    int nVertices = 0;
    PK_VERTEX_t* vertices;
    pkError = PK_BODY_ask_vertices(wireBody, &nVertices, &vertices);
    CheckM(pkError == PK_ERROR_no_errors);
    //Should have 6 vertices
    CheckM(nVertices == 6);
    IntArray nEdgesPerVertex;
    EntityTag vertexToRemove; //This is the vertex created by splitting the bcurve into two edges
    EntityTag linesVertex; //This is the vertex connecting the three lines
    for (int v = 0; v < nVertices; ++v)
    {
        IntArray vertEdges;
        int nVertEdges = interaction->GetPKUtils().VertexGetEdges(vertices[v], vertEdges);
        nEdgesPerVertex.push_back(nVertEdges);
        if (nVertEdges == 3)
            linesVertex = vertices[v];
        else if (nVertEdges == 2)
        {
            PointData pointData;
            EntityGetPointData(interactionTag, vertices[v], &pointData, &result);
            CheckResultM(result);
            if (pointData.GetPosition() == Position(0.0, 0.1, 0.0))
                vertexToRemove = vertices[v];
        }
    }
    if (vertices)
        PK_MEMORY_free(vertices);
    CheckM(vertexToRemove && linesVertex);
    //Check we have the expected number of edges per vertex
    IntArray nEdgesPerVertexExpected;
    nEdgesPerVertexExpected.push_back(2);
    nEdgesPerVertexExpected.push_back(2);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(3);
    CheckM(nEdgesPerVertex == nEdgesPerVertexExpected);

    //Try merging the coi vertices so the bcurve and lines are connected
    TagArray vertsToMerge;
    vertsToMerge.Add(vertexToRemove);
    vertsToMerge.Add(linesVertex);
    VertexMerge(interactionTag, vertsToMerge, TagNull, &result);
    CheckResultM(result);

    //Now check the vertices again
    pkError = PK_BODY_ask_vertices(wireBody, &nVertices, &vertices);
    CheckM(pkError == PK_ERROR_no_errors);
    //Should now have 5 vertices
    CheckM(nVertices == 5);
    nEdgesPerVertex.clear();
    for (int v = 0; v < nVertices; ++v)
    {
        IntArray vertEdges;
        int nVertEdges = interaction->GetPKUtils().VertexGetEdges(vertices[v], vertEdges);
        nEdgesPerVertex.push_back(nVertEdges);
    }
    if (vertices)
        PK_MEMORY_free(vertices);
    nEdgesPerVertexExpected.clear();
    nEdgesPerVertexExpected.push_back(5); //This vertex is connected to all 5 edges
    nEdgesPerVertexExpected.push_back(2);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(1);
    CheckM(nEdgesPerVertex == nEdgesPerVertexExpected);

    //Merge the edges
    IntArray mergedEdges;
    bool mergeSuccess = interaction->GetPKUtils().TestRepairSplitEdgesForWirebodyCreation(interaction->GetVersion(), geoms, wireBody, newEdgeCountPS, newEdgesPS, edgeIndicesPS, &mergedEdges);
    CheckM(mergeSuccess);

    //Check there is now a single edge for the b-curve (4 in total)
    CheckM((int)mergedEdges.size() == 4);

    //Check vertices now
    pkError = PK_BODY_ask_vertices(wireBody, &nVertices, &vertices);
    CheckM(pkError == PK_ERROR_no_errors);
    //Should still have 5 vertices
    CheckM(nVertices == 5);
    nEdgesPerVertex.clear();
    for (int v = 0; v < nVertices; ++v)
    {
        IntArray vertEdges;
        int nVertEdges = interaction->GetPKUtils().VertexGetEdges(vertices[v], vertEdges);
        nEdgesPerVertex.push_back(nVertEdges);
    }
    if (vertices)
        PK_MEMORY_free(vertices);
    nEdgesPerVertexExpected.clear();
    nEdgesPerVertexExpected.push_back(3); //This vertex has been detached from the bcurve
    nEdgesPerVertexExpected.push_back(2);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(1);
    nEdgesPerVertexExpected.push_back(1);
    CheckM(nEdgesPerVertex == nEdgesPerVertexExpected);

    if (newEdgesPS)
        PK_MEMORY_free(newEdgesPS);
    if (edgeIndicesPS)
        PK_MEMORY_free(edgeIndicesPS);

    SetLiveInteraction(interactionTag);

    return true;
}

VTK_TEST_FN(TestPKUtils083_EdgeIsPeriodic001)
{
    // Test checking whether different edges are periodic
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/AllCurveTypes.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    VTKBody body = bodyArray[0];
    IntArray edges;
    body.GetEdges(interactionPtr->GetVersion(), &edges);
    CheckM(edges.size() == 7);

    for (int e : edges)
    {
        switch (EntityGetGeometryType(interaction, e, &result))
        {
        case GeometryTypeLine :
            CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(e) == false);
            break;
        case GeometryTypeCircle:
            if (interactionPtr->GetPKUtils().EntityGetIdentifier(e) == 120)
                CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(e) == true);
            else
                CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(e) == false);
            break;
        case GeometryTypeEllipse:
            CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(e) == false);
            break;
        case GeometryTypeSplineCurve:
            CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(e) == false);
            break;
        case GeometryTypeConic:
            CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(e) == false);
            break;
        default:
            //Should not be here
            CheckM(false);
            break;
        }
    }

    return true;
}

VTK_TEST_FN(TestPKUtils083_EdgeIsPeriodic002)
{
    // Test periodicity checking on a closed non-periodic edge
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurve_edge_with_teardrop2.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    VTKBody body = bodyArray[0];
    IntArray edges;
    body.GetEdges(interactionPtr->GetVersion(), &edges);
    CheckM(edges.size() == 1);

    //Confirm edge is closed
    CheckM(interactionPtr->GetPKUtils().EdgeIsClosed(edges[0]));

    //Check edge is not periodic
    CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(edges[0]) == false);

    return true;
}

VTK_TEST_FN(TestPKUtils084_EdgeIsClosedNonPeriodic001)
{
    // Test EdgeIsClosedNonPeriodic for various edges
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/AllCurveTypes.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    VTKBody body = bodyArray[0];
    IntArray edges;
    body.GetEdges(interactionPtr->GetVersion(), &edges);
    CheckM(edges.size() == 7);

    for (int e : edges)
    {
        switch (EntityGetGeometryType(interaction, e, &result))
        {
        case GeometryTypeLine:
            CheckM(interactionPtr->GetPKUtils().EdgeIsClosedNonPeriodic(e) == false);
            break;
        case GeometryTypeCircle:
            CheckM(interactionPtr->GetPKUtils().EdgeIsClosedNonPeriodic(e) == false);
            break;
        case GeometryTypeEllipse:
            CheckM(interactionPtr->GetPKUtils().EdgeIsClosedNonPeriodic(e) == false);
            break;
        case GeometryTypeSplineCurve:
            CheckM(interactionPtr->GetPKUtils().EdgeIsClosedNonPeriodic(e) == false);
            break;
        case GeometryTypeConic:
            CheckM(interactionPtr->GetPKUtils().EdgeIsClosedNonPeriodic(e) == false);
            break;
        default:
            //Should not be here
            CheckM(false);
            break;
        }
    }

    return true;
}

VTK_TEST_FN(TestPKUtils084_EdgeIsClosedNonPeriodic002)
{
    // Test EdgeIsClosedNonPeriodic for a closed non-periodic edge
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Splines/bcurve_edge_with_teardrop2.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 1);
    VTKBody body = bodyArray[0];
    IntArray edges;
    body.GetEdges(interactionPtr->GetVersion(), &edges);
    CheckM(edges.size() == 1);

    //Check edge is closed
    CheckM(interactionPtr->GetPKUtils().EdgeIsClosed(edges[0]));

    //Check edge is not periodic
    CheckM(interactionPtr->GetPKUtils().EdgeIsPeriodic(edges[0]) == false);

    //Check edge is closed and non periodic
    CheckM(interactionPtr->GetPKUtils().EdgeIsClosedNonPeriodic(edges[0]));

    return true;
}

VTK_TEST_FN(TestPKUtils085_PR_9497136_compare)
{
    // This PR has produced a body which we cannot produce a valid journal checkpoint for.
    // We should be able to load this body twice and compare with no differences (else we need to understand)

    VTKBody vbody1 = TestUtilLoadSingleBody("vtk_part_data:Misc/9497136_compare.x_t");
    VTKBody vbody2 = TestUtilLoadSingleBody("vtk_part_data:Misc/9497136_compare.x_t");

    InteractionTag interaction1 = CreateInteraction();

    // Compare body 1 with another loaded version of body 1 (body 2)
    CheckM(PKUtils().DebugBodyCompare(GetVersion(interaction1), vbody1, vbody2));
    return true;
}

VTK_TEST_FN(TestPKUtils086_CurveEvaluateAtParameter_EllipseParameterConversion01)
{
    //Test that evaluating elliptic curve at given parameters works for input parameters at the edge of the first period.
    //This can cause problems during parameter conversion unless we properly use tolerances.
    //Corresponding API test is TestVTKApiGeometry_CurveEvaluateAtParameter_EllipseParameterConversion.

    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/ellipse_inside_circle_with_coi_point.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 2);
    VTKBody body = bodyArray[1];

    PKUtils& pkUtils = interactionPtr->GetPKUtils();

    //Get ellipse curve tag
    int edgeTag = EdgeOfId(body, 34);
    CheckTagM(edgeTag);

    int ellipseTag = pkUtils.EdgeGetCurve(edgeTag);
    CheckTagM(ellipseTag);

    //Check using a param value that we know works
    double param = 1.5;
    VecArray posAndDerivs;
    pkUtils.CurveEvaluateAtParam(ellipseTag, param, 0, posAndDerivs);
    CheckM(posAndDerivs.size() == 1);
    CheckM(posAndDerivs[0].IsEqual(Vec(-0.24033400821103568, 0.13529773305275208, 0.0), Resabs));

    //Now try a value just under pi/2, which would originally cause a hang
    //due to being just outside the first period after subtracting pi/2 (to convert to PK parameter).
    param = 1.5707963267948961;
    posAndDerivs.clear();
    pkUtils.CurveEvaluateAtParam(ellipseTag, param, 0, posAndDerivs);
    CheckM(posAndDerivs.size() == 1);
    CheckM(posAndDerivs[0].IsEqual(Vec(-0.24168301948580498, 0.13536035838765073, 0.0), Resabs));

    //Also check params corresponding to 2pi and just over 2pi after subtracting pi/2.
    param = 7.8539816339744830;
    posAndDerivs.clear();
    pkUtils.CurveEvaluateAtParam(ellipseTag, param, 0, posAndDerivs);
    CheckM(posAndDerivs.size() == 1);
    CheckM(posAndDerivs[0].IsEqual(Vec(-0.24168301948580498, 0.13536035838765073, 0.0), Resabs));

    param = 7.8539816339744835;
    posAndDerivs.clear();
    pkUtils.CurveEvaluateAtParam(ellipseTag, param, 0, posAndDerivs);
    CheckM(posAndDerivs.size() == 1);
    CheckM(posAndDerivs[0].IsEqual(Vec(-0.24168301948580501, 0.13536035838765073, 0.0), Resabs));

    return true;
}

VTK_TEST_FN(TestPKUtils086_CurveEvaluateAtParameter_EllipseParameterConversion02)
{
    //Test that evaluating elliptic curve at given parameters works for input parameters at the edge of the first period.
    //This can cause problems during parameter conversion unless we properly use tolerances.
    //Check value 6.2831853024138979 which originally caused PR test case
    //FAIL_nx_next_sketch_ellipse_normalise_parameter_compute_changeable_entities_hang.xml to hang.

    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/Ellipses/connected_circles_and_ellipses.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);

    VTKBodyArray bodyArray;
    VTKBody::SessionGetBodies(interactionPtr->GetVersion(), bodyArray);
    CheckM(bodyArray.size() == 2);
    VTKBody body = bodyArray[1];

    PKUtils& pkUtils = interactionPtr->GetPKUtils();

    //Get ellipse curve tag
    int edgeTag = EdgeOfId(body, 6);
    CheckTagM(edgeTag);

    int ellipseTag = pkUtils.EdgeGetCurve(edgeTag);
    CheckTagM(ellipseTag);

    double param = 6.2831853024138979 + (PI / 2);
    VecArray posAndDerivs;
    pkUtils.CurveEvaluateAtParam(ellipseTag, param, 0, posAndDerivs);
    CheckM(posAndDerivs.size() == 1);
    CheckM(posAndDerivs[0].IsEqual(Vec(0.0, 0.028, 0.0), Resabs));

    return true;
}

VTK_TEST_FN(TestPKUtils087_Parameter_Can_Be_Found_On_Linear_Extension_Of_OffsetCurve_Outside_Curve_Interval)
{
    PKUtils pkUtils;
    VecArray pts;
    pts.push_back(Vec(-0.3533806304780734, 0.105746476544575, 0));
    pts.push_back(Vec(-0.1472419293658639, 0.1351948624177478, 0));
    pts.push_back(Vec(-0.2400489636328327, 0.03257169952638807, 0));
    ConicDef conicDef(pts, 0.4);

    PK_CURVE_t conic = pkUtils.CreateConic(conicDef);
    CheckTagM(conic);


    OffsetCurveApproximator offsetApproximator(GetCurrentVersion(), &pkUtils);
    PK_CURVE_t offset = offsetApproximator.CreateCurve(conic, 0.045, OffsetSideUnit::Left, NULL, NULL, true);
    CheckTagM(offset);

    Vec pos;
    pkUtils.CurveEvaluateAtParam(offset, -0.2, &pos);
    double outParam = 0;
    CheckM(pkUtils.CurveGetParamForPos(offset, pos, &outParam));
    CheckM(ResparamEqual(outParam, -0.2));

    pkUtils.CurveEvaluateAtParam(offset, 1.2, &pos);
    CheckM(pkUtils.CurveGetParamForPos(offset, pos, &outParam));
    CheckM(ResparamEqual(outParam, 1.2));

    return true;
}

VTK_TEST_FN(TestPKUtils_EntityGetClass_SingleSegmentPLine)
{
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/MixedModelling/MixedBody.xml").c_str(), result);
    CheckResultM(result);

    CheckM(interaction->GetInstance(0));
    CheckM(interaction->GetInstance(0)->GetVTKPart().GetParentBodies().size() == 1);
    VTKBody vbody = interaction->GetInstance(0)->GetVTKPart().GetParentBodies()[0];
    CheckM(vbody.IsNotNull());

    int pLineTag = EdgeOfId(vbody, 387);
    CheckM(pLineTag);

    CheckM(PKUtils().EntityGetClass(pLineTag) == EntityClass::Edge);
    CheckM(PKUtils().EntityGetGeomClass(pLineTag) == EntityClass::Pline);
    CheckM(PKUtils().EntityGetClass(PKUtils().EntityGetGeom(pLineTag)) == EntityClass::Pline);

    int singleSegmentPLineTag = EdgeOfId(vbody, 472);
    CheckM(singleSegmentPLineTag);

    CheckM(PKUtils().EntityGetClass(singleSegmentPLineTag) == EntityClass::Edge);
    CheckM(PKUtils().EntityGetGeomClass(singleSegmentPLineTag) == EntityClass::SingleSegmentPline);
    CheckM(PKUtils().EntityGetClass(PKUtils().EntityGetGeom(singleSegmentPLineTag)) == EntityClass::SingleSegmentPline);

    return true;
}

VTK_TEST_FN(TestPKUtils088_SplitEdgesOfG0Curve_With_Missing_Vertex_Can_Be_Merged)
{
    //In this case PK_CURVE_make_wire_body_2 produces two split edges for 
    //a G0 spline, but there is no vertex between the edges. Workaround code
    //has been added in RepairSplitEdgesForCurve to merge the edges.
    
    VTK::ResultType result = ResultTypeOk;
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/ZPlaneProfiles/g0_curves_to_make_invalid_wirebody.xml").c_str());
    CheckM(interaction != 0);
    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);

    IntArray curves(interactionPtr->GetCreatedGeometries().begin(), interactionPtr->GetCreatedGeometries().end());
    CheckM(curves.size() == 5);

    DoublePairArray intervals;
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(0, 1));
    intervals.push_back(DoublePair(0, 0.003530976879877608));
    intervals.push_back(DoublePair(0, 0.003530976879877702));
    intervals.push_back(DoublePair(0, 0.0009333340799999699));

    IntArray newEdges;
    PK_BODY_t body = PKUtils().CurveMakeWirebody(interactionPtr->GetVersion(), curves, intervals, true, &newEdges);
    CheckTagM(body);
    CheckM(VTKBody(body).GetEdges(interactionPtr->GetVersion()) == 5);
    for (size_t i = 0; i < 5; i++)
    {
        CheckM(PKUtils().EdgeGetCurve(newEdges[i]) == curves[i]);
        double interval[2];
        PKUtils().EdgeGetInterval(newEdges[i], interval);
        CheckM(ResparamEqual(interval[0], intervals[i].first));
        CheckM(ResparamEqual(interval[1], intervals[i].second));
    }
    return true;
}

Vec MidPoint(Vec start, Vec end, Vec circlePoint, double circleRadius)
{
    Vec startPos = start;
    Vec endPos = end;
    Vec centrePos = circlePoint;
    double radius = circleRadius;

    Dir axisDir = Dir::ZDir();
    Dir xDir = !axisDir;
    Dir yDir = CommonNormal(axisDir, xDir);

    Vec centreToStart = startPos - centrePos;
    Vec centreToEnd = endPos - centrePos;
    double xToStart = GeomUtils::ComputeDirectedAngleBetween(Dir::XDir(), Normal(centreToStart), Dir::ZDir());
    double xToEnd = GeomUtils::ComputeDirectedAngleBetween(Dir::XDir(), Normal(centreToEnd), Dir::ZDir());
    if ( xToEnd < xToStart )
    {
        xToEnd += TWO_PI;
    }
    double range = xToEnd - xToStart;


    double midPointParam = xToStart + (range / 2.0);

    double cosA = radius * cos(midPointParam);
    double sinA = radius * sin(midPointParam);
    double x = centrePos.x + (cosA);
    double y = centrePos.y + (sinA);
    double z = centrePos.z;
    Vec midPoint(x, y, z);
    return midPoint;
}

VTK_TEST_FN(TestPKUtils088_CircleEdgeMidpoints)
{
    //Test for PKUtils().ComputeConeRadiusAtAxisPos
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ExportDCM/Misc/MegalCirclesProfile.xml").c_str());

    VTKBodyArray bodies;
    VTKBody::SessionGetAllPKBodies(bodies);
    CheckM(bodies.size() == 1);

    IntSet edges;
    bodies[0].GetEdges(VTK::GetVersion(interaction), edges);

// Check edge 10
// Check edge 6
// Check edge 17
// Check edge 21
// Check edge 40

    //edges.clear();
    //edges.insert(EdgeOfId(bodies[0], 40));


    for ( int edge : edges )
    {
        PK_EDGE_t circleEdgeId107 = edge;
        int edgeID = PKUtils().EntityGetIdentifier(edge);
        CheckM(circleEdgeId107);

        //printf("Check edge %u\n", edgeID);

        if ( PKUtils().EdgeIsCircular(circleEdgeId107) && PKUtils().EdgeIsCircularArc(circleEdgeId107) )
        {
            Vec circlePoint;
            double circleRadius;
            PKUtils().GetGeo(circleEdgeId107, 0, 0, &circlePoint, 0, &circleRadius);

            Vec start, end, middle;
            PKUtils().EdgeGetEndsAndMidPoints(circleEdgeId107, start, middle, end);

            Vec midPoint = MidPoint(start, end, circlePoint, circleRadius);
            
            CheckM(ResEqual(middle, midPoint));
        }
        else
        {
            // not circle or full circle
            int badEdge = edge;
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils089_CircleEdgeMidpoints)
{
    //Test for PKUtils().ComputeConeRadiusAtAxisPos
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/ExportDCM/Misc/LinesCirclesEdited.xml").c_str());

    VTKBodyArray bodies;
    VTKBody::SessionGetAllPKBodies(bodies);
    CheckM(bodies.size() == 1);

    IntSet edges;
    bodies[0].GetEdges(VTK::GetVersion(interaction), edges);


    // Find a circle edge
    edges.clear();
    edges.insert(EdgeOfId(bodies[0], 174));


    for ( int edge : edges )
    {
        PK_EDGE_t circleEdgeId107 = edge;
        int edgeID = PKUtils().EntityGetIdentifier(edge);
        CheckM(circleEdgeId107);

        //printf("Check edge %u\n", edgeID);

        if ( PKUtils().EdgeIsCircular(circleEdgeId107) && PKUtils().EdgeIsCircularArc(circleEdgeId107) )
        {
            Vec circlePoint;
            double circleRadius;
            PKUtils().GetGeo(circleEdgeId107, 0, 0, &circlePoint, 0, &circleRadius);

            Vec start, end, middle;
            PKUtils().EdgeGetEndsAndMidPoints(circleEdgeId107, start, middle, end);

            Vec midPoint = MidPoint(start, end, circlePoint, circleRadius);

            CheckM(ResEqual(middle, midPoint));
        }
        else
        {
            // not circle or full circle
            int badEdge = edge;
        }
    }
    return true;
}

VTK_TEST_FN(TestPKUtils090_ZeroRadiusArc_UpdateCurveData)
{
    //Test to check that the radius is updated in the curve data attribute for a zero-radius arc
    Interaction* interaction = TestCreateInteraction();
    PKUtils& pkUtils = interaction->GetPKUtils();

    //Create zero length/radius arc
    double rad = 1.0e-15;
    int circle = pkUtils.CreateCircle(Vec::Origin(), Dir::ZDir(), rad);
    CheckM(pkUtils.CurveIsZeroRadiusCircle(circle));

    DoublePair interval;
    interval.first = 0.0;
    interval.second = 0.0;
    int edge = pkUtils.CreateZeroLengthEdge(interaction->GetVersion(), circle, interval);
    CheckM(edge != 0);

    //Check radius stored in curve attribute data
    double storedRadius = 0.0;
    pkUtils.GetCircleData(circle, NULL, NULL, NULL, &storedRadius, NULL, NULL);
    CheckM(storedRadius == rad);

    double deltaRadius = -rad;
    CheckM(pkUtils.UpdateZeroRadiusCircleData(circle, deltaRadius));

    //Now check the stored radius again, it should have changed
    pkUtils.GetCircleData(circle, NULL, NULL, NULL, &storedRadius, NULL, NULL);
    CheckM(storedRadius != rad);
    CheckM(storedRadius == 0.0);

    //Now create a non-zero radius circle and check its radius is not changed
    rad = 1.0e-5;
    circle = pkUtils.CreateCircle(Vec::Origin(), Dir::ZDir(), rad);
    CheckM(pkUtils.CurveIsZeroRadiusCircle(circle) == false);

    edge = pkUtils.CreateZeroLengthEdge(interaction->GetVersion(), circle, interval);
    CheckM(edge != 0);

    pkUtils.GetCircleData(circle, NULL, NULL, NULL, &storedRadius, NULL, NULL);
    CheckM(storedRadius == rad);

    deltaRadius = -rad;
    CheckM(pkUtils.UpdateZeroRadiusCircleData(circle, deltaRadius));

    pkUtils.GetCircleData(circle, NULL, NULL, NULL, &storedRadius, NULL, NULL);
    CheckM(storedRadius != 0.0);
    CheckM(storedRadius == rad);

    return true;
}

VTK_TEST_FN(TestPKUtils091_CheckEdgeEdgeIntersectionFaultFiltering)
{
    //Check that an edge/edge intersection fault is filtered in the body and session context
    InteractionTag interaction = DebugLoadInteraction(TestUtilPrependFilename("vtk_test_data:Input/Profiles/perpendicular_intersection_multiple_bodies.xml").c_str());
    CheckM(interaction != 0);

    Interaction* interactionPtr = TagManager::FindInteractionFromTag(interaction);
    CheckM(interactionPtr != 0);
    interactionPtr->TestMergeCoiVerticesOfProfileBodies(true);

    int ref1 = ObjectFindByIdentifier(interaction,977);
    CheckM(ref1 != 0);
    int ref2 = ObjectFindByIdentifier(interaction,980);
    CheckM(ref2 != 0);

    int ent1 = ReferenceGetEntity(interaction,ref1);
    int ent2 = ReferenceGetEntity(interaction,ref2);
    IntArray ents;
    ents.push_back(ent1);
    ents.push_back(ent2);

    //This fault is ignored
    CheckM(!VTKBody::ShouldKeepCheckFaultForBodyEntity(PK_BODY_type_wire_c,PK_EDGE_state_bad_wire_ed_ed_c,ent1,ent2));
    CheckM(!PKUtils().DebugShouldKeepSessionCheckFault(PK_EDGE_state_bad_wire_ed_ed_c,ents));

    //This fault is not ignored
    IntArray ent;
    ent.push_back(ent1);
    CheckM(VTKBody::ShouldKeepCheckFaultForBodyEntity(PK_BODY_type_wire_c,PK_EDGE_state_bad_vertex_c,ent1,0));
    CheckM(PKUtils().DebugShouldKeepSessionCheckFault(PK_EDGE_state_bad_vertex_c,ent));

    return true;
}

bool TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(String model, int curveId, int edgeId, const Vec& pos, double distance, Dir planeNormal, const VecArray& expectedPositions, const DoubleVector expectedParams)
{
    // Check that the method CurveFindPositionsExactDistanceFromGivenPositionInPlane returns the expected output

    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename(model).c_str(), result);
    CheckResultM(result);

    SetLiveInteraction(interaction->GetTag());

    VTKBody vbody = interaction->GetProfile(0)->GetWireBodies()[0];

    int curve = 0;
    if (curveId != 0)
    {
        curve = CurveOfId(vbody, curveId);
    }
    if (edgeId != 0)
    {
        int edge = EdgeOfId(vbody, edgeId);
        curve = PKUtils().EdgeGetCurve(edge);
    }
    CheckM(curve != 0);

    VecArray returnedPositions;
    DoubleVector returnedParams;
    CheckM(PKUtils().CurveFindPositionsExactDistanceFromGivenPositionInPlane(curve, pos, distance, planeNormal, &returnedPositions, &returnedParams));

    CheckM(returnedParams.size() == returnedPositions.size());
    CheckM(returnedParams.size() == expectedParams.size());

    for (int i = 0; i < expectedParams.size(); i++)
    {
        bool foundExpected = false;
        double expectedParam = expectedParams.at(i);
        Vec expectedPosition = expectedPositions.at(i);
        for (int j = 0; j < returnedParams.size(); j++)
        {
            double returnedParam = returnedParams.at(j);
            Vec returnedPosition = returnedPositions.at(j);

            if (ResEqual(returnedParam, expectedParam) && ResEqual(returnedPosition, expectedPosition))
            {
                foundExpected = true;
                break;
            }
        }
        CheckM(foundExpected);
    }

    return true;
}

VTK_TEST_FN(TestPKUtils092a_CurveFindPositionsExactDistanceFromGivenPositionInPlane_ZeroIntersections)
{
    // Check that if the point and the curve are further away than the given distance, then 
    // CurveFindPositionsExactDistanceFromGivenPositionInPlane will return nothing

    String model = "vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml";

    int curveId = 144;
    int edgeId = 0;
    Vec pos = Vec(0.06931972649297335, 0.04239332504047234, 0.0);
    double distance = 0.02468027350685413;
    Dir planeNormal = Dir::ZDir();
    VecArray expectedPositions;
    DoubleVector expectedParams;
    CheckM(TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(model, curveId, edgeId, pos, distance, planeNormal, expectedPositions, expectedParams));


    return true;
}

VTK_TEST_FN(TestPKUtils092b_CurveFindPositionsExactDistanceFromGivenPositionInPlane_OneIntersection)
{
    // Check that if there is exactly one point on the curve at the given distance from the position
    // then we will return that as the intersection

    String model = "vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml";

    int curveId = 132;
    int edgeId = 0;
    Vec pos = Vec(0.02124231896155113, 0.0206615752568449, 0.0);
    double distance = 0.052;
    Dir planeNormal = Dir::ZDir();
    VecArray expectedPositions;
    expectedPositions.push_back(Vec(0.02124231896155113, -0.031338424743155097, 0.0));
    DoubleVector expectedParams;
    expectedParams.push_back(4.7123889803846897);
    CheckM(TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(model, curveId, edgeId, pos, distance, planeNormal, expectedPositions, expectedParams));

    return true;
}

VTK_TEST_FN(TestPKUtils092c_CurveFindPositionsExactDistanceFromGivenPositionInPlane_MultipleIntersections)
{
    // Check that the method CurveFindPositionsExactDistanceFromGivenPositionInPlane returns the correct
    // two points on an offset curve 

    String model = "vtk_test_data:Input/Profiles/OffsetCurves/doubly_tan_between_bcurve_and_offsetcurve.xml";

    int curveId = 0;
    int edgeId = 1474;
    Vec pos = Vec(0.1812912500795387, 0.07629492003750971, 0.0);
    double distance = 0.06;
    Dir planeNormal = Dir::ZDir();
    VecArray expectedPositions;
    expectedPositions.push_back(Vec(0.19060895131363986, 0.017022831802725617, 0.0));
    expectedPositions.push_back(Vec(0.20256855596430978, 0.020194329538960706, 0.0));
    DoubleVector expectedParams;
    expectedParams.push_back(0.89595942787826666);
    expectedParams.push_back(0.95432567952391345);
    CheckM(TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(model, curveId, edgeId, pos, distance, planeNormal, expectedPositions, expectedParams));

    return true;
}

VTK_TEST_FN(TestPKUtils092d_CurveFindPositionsExactDistanceFromGivenPositionInPlane_InfiniteIntersections)
{
    // Check the output when the method CurveFindPositionsExactDistanceFromGivenPositionInPlane 
    // is called on a point at the centre of a circle with a given distance equal to the radius of the circle
    // I'm not sure what the most desirable result of this case is, so this test is mostly checking that this
    // edge case doesn't cause a crash, and seeing if the behaviour changes

    String model = "vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml";

    int curveId = 151;
    int edgeId = 0;
    Vec pos = Vec(0.06931972649297335, 0.04239332504047234, 0.0);
    double distance = 0.02468027350685413;
    Dir planeNormal = Dir::ZDir();
    VecArray expectedPositions;
    expectedPositions.push_back(Vec(0.093999999999827485, 0.042393325040472336, 0.0));
    expectedPositions.push_back(Vec(0.093999999999827485, 0.042393325040472336, 0.0));
    DoubleVector expectedParams;
    expectedParams.push_back(0.0);
    expectedParams.push_back(0.0);
    CheckM(TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(model, curveId, edgeId, pos, distance, planeNormal, expectedPositions, expectedParams));

    return true;
}

VTK_TEST_FN(TestPKUtils092e_CurveFindPositionsExactDistanceFromGivenPositionInPlane_DistanceResZero)
{
    // Check that CurveFindPositionsExactDistanceFromGivenPositionInPlane still works with a res zero distance
    // The point is slightly further than 10^(-8) from the curve, but the distance from the curve puts us within resabs

    String model = "vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml";

    int curveId = 132;
    int edgeId = 0;
    Vec pos = Vec(0.02124231896155113, -0.03133840674, 0.0);
    double distance = 0.000000009;
    Dir planeNormal = Dir::ZDir();
    VecArray expectedPositions;
    expectedPositions.push_back(Vec(0.02124231896155113, -0.031338424743155097, 0.0));
    DoubleVector expectedParams;
    expectedParams.push_back(4.7123889803846897);
    CheckM(TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(model, curveId, edgeId, pos, distance, planeNormal, expectedPositions, expectedParams));

    return true;
}

VTK_TEST_FN(TestPKUtils092f_CurveFindPositionsExactDistanceFromGivenPositionInPlane_CurveNotInSamePlaneAsPoint)
{
    // Check that CurveFindPositionsExactDistanceFromGivenPositionInPlane only finds intersection points in the 
    // plane through the position passed in, normal to the given direction

    String model = "vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml";

    int curveId = 132;
    int edgeId = 0;
    Vec pos = Vec(0.02124231896155113, 0.0206615752568449, 0.0);
    double distance = 0.062;
    Dir planeNormal = Dir::XDir();
    VecArray expectedPositions;
    DoubleVector expectedParams;
    CheckM(TestPKUtils092_CurveFindPositionsExactDistanceFromGivenPositionInPlane_Base(model, curveId, edgeId, pos, distance, planeNormal, expectedPositions, expectedParams));

    return true;
}

VTK_TEST_FN(EdgeIsSingleSegmentPline_Works)
{
    // Identifies only edges with SingleSegmentPline geoms as edges with SingleSegmentPline geoms 
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Cage/MeshFaces/ThreeMeshFacesOnTripleChamferedCube.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int sspEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 12);
    int linearEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 168);
    int vertex = vBody.FindEntityByIdentifier(EntityClass::Vertex, 5);
    int mesh = vBody.FindEntityByIdentifier(EntityClass::Mesh, 26);
    int plane = vBody.FindEntityByIdentifier(EntityClass::Plane, 67);

    CheckM(pkUtils.EdgeIsSingleSegmentPline(sspEdge));
    CheckFalseM(pkUtils.EdgeIsSingleSegmentPline(linearEdge));
    CheckFalseM(pkUtils.EdgeIsSingleSegmentPline(vertex));
    CheckFalseM(pkUtils.EdgeIsSingleSegmentPline(mesh));
    CheckFalseM(pkUtils.EdgeIsSingleSegmentPline(plane));

    return true;
}

VTK_TEST_FN(EntityIsBsurf_Works)
{
    // Identifies only entities that are Bsurf geoms as Bsurf geoms
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Cage/OneMeshBsurfPlanarFace.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int bsurfFace = vBody.FindEntityByIdentifier(EntityClass::Bsurf, 391);
    int linearEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 64);
    int vertex = vBody.FindEntityByIdentifier(EntityClass::Vertex, 5);
    int mesh = vBody.FindEntityByIdentifier(EntityClass::Mesh, 69);
    int plane = vBody.FindEntityByIdentifier(EntityClass::Plane, 35);

    CheckM(pkUtils.EntityIsBsurf(bsurfFace));
    CheckFalseM(pkUtils.EntityIsBsurf(linearEdge));
    CheckFalseM(pkUtils.EntityIsBsurf(vertex));
    CheckFalseM(pkUtils.EntityIsBsurf(mesh));
    CheckFalseM(pkUtils.EntityIsBsurf(plane));

    return true;
}

VTK_TEST_FN(CageEdgeGetDir_WorksForLineAndSingleSegmentPlineGeoms)
{
    // Can get directions for edges with line and SingleSegmentPline geoms
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Cage/MeshFaces/ThreeMeshFacesOnTripleChamferedCube.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int sspEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 12);
    int linearEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 63);
    CheckM(pkUtils.EdgeIsSingleSegmentPline(sspEdge));
    CheckM(pkUtils.EdgeIsLinear(linearEdge));


    Dir lineDir = pkUtils.LinearEdgeGetDir(linearEdge);
    Dir sspDir = pkUtils.LinearEdgeGetDir(sspEdge);

    CheckDirAlignedM(lineDir, Dir::YDir().Reverse());
    CheckDirAlignedM(sspDir, Dir::ZDir().Reverse());

    return true;
}

VTK_TEST_FN(EntityIsCurve_WorksWith2DModel)
{
    // Identifies only entites that are curves as curves 
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Profiles/ZPlaneProfiles/AllCurveTypes_virtuals.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int bcurve = vBody.FindEntityByIdentifier(EntityClass::Bcurve, 144);
    int line = vBody.FindEntityByIdentifier(EntityClass::Line, 141);
    int circle = vBody.FindEntityByIdentifier(EntityClass::Circle, 156);
    int ellipse = vBody.FindEntityByIdentifier(EntityClass::Ellipse, 132);
    int point = vBody.FindEntityByIdentifier(EntityClass::Point, 314);
    int edge = vBody.FindEntityByIdentifier(EntityClass::Edge, 120);

    CheckM(pkUtils.EntityIsCurve(bcurve));
    CheckM(pkUtils.EntityIsCurve(line));
    CheckM(pkUtils.EntityIsCurve(circle));
    CheckM(pkUtils.EntityIsCurve(ellipse));
    CheckFalseM(pkUtils.EntityIsCurve(point));
    CheckFalseM(pkUtils.EntityIsCurve(edge));

    return true;
}

VTK_TEST_FN(EntityIsCurve_WorksWith3DModel)
{
    // Identifies only entites that are curves as curves 
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/SurfaceExt/SweptTopAndSideWithBlend.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int bcurve = vBody.FindEntityByIdentifier(EntityClass::Bcurve, 241);
    int line = vBody.FindEntityByIdentifier(EntityClass::Line, 55);
    int icurve = vBody.FindEntityByIdentifier(EntityClass::Icurve, 314);
    int vertex = vBody.FindEntityByIdentifier(EntityClass::Vertex, 180);
    int point = vBody.FindEntityByIdentifier(EntityClass::Vertex, 252);
    int edge = vBody.FindEntityByIdentifier(EntityClass::Edge, 145);
    int face = vBody.FindEntityByIdentifier(EntityClass::Face, 201);

    CheckM(pkUtils.EntityIsCurve(bcurve));
    CheckM(pkUtils.EntityIsCurve(line));
    CheckM(pkUtils.EntityIsCurve(icurve));
    CheckFalseM(pkUtils.EntityIsCurve(point));
    CheckFalseM(pkUtils.EntityIsCurve(edge));
    CheckFalseM(pkUtils.EntityIsCurve(face));
    CheckFalseM(pkUtils.EntityIsCurve(vertex));

    return true;
}

VTK_TEST_FN(EdgeGetFaces_SetVariantWorksWithLinearEdge)
{
    // Can get 2 attached faces of a give edge. One is a mesh, one is a plane. The Edge is a line
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Cage/MeshFaces/ThreeMeshFacesOnTripleChamferedCube.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int linearEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 43);
    IntSet expectedFaceTagsSet;
    expectedFaceTagsSet.insert(vBody.FindEntityByIdentifier(EntityClass::Face, 35));  // Plane
    expectedFaceTagsSet.insert(vBody.FindEntityByIdentifier(EntityClass::Face, 71));  // Mesh
    IntSet returnedFaces;

    pkUtils.EdgeGetFaces(linearEdge, returnedFaces);

    CheckSizeM(returnedFaces, 2);
    CheckM(returnedFaces == expectedFaceTagsSet);

    return true;
}

VTK_TEST_FN(EdgeGetFaces_SetVariantWorksWithSingleSegmentPlineEdge)
{
    // Can get 2 attached faces of a give edge. Both are meshes. The Edge is a singleSegmentPline
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Cage/MeshFaces/ThreeMeshFacesOnTripleChamferedCube.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int sspEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 64);
    IntSet expectedFaceTagsSet;
    expectedFaceTagsSet.insert(vBody.FindEntityByIdentifier(EntityClass::Face, 69));
    expectedFaceTagsSet.insert(vBody.FindEntityByIdentifier(EntityClass::Face, 71));
    IntSet returnedFaces;

    pkUtils.EdgeGetFaces(sspEdge, returnedFaces);

    CheckSizeM(returnedFaces, 2);
    CheckM(returnedFaces == expectedFaceTagsSet);

    return true;
}

VTK_TEST_FN(EdgeGetFaces_SetVariantWorksWithCircularEdge)
{
    // Can get 2 attached faces of a give edge. One is a plane, one is a cylinder. The Edge is an arc
    ResultType result = ResultTypeOk;
    Interaction* interaction = TestCreateInteractionFromFile(TestUtilPrependFilename("vtk_test_data:Input/Compare/CompareConfigResult.xml").c_str(), result);
    CheckResultM(result);

    PKUtils pkUtils; VTKBodyArray bodies;
    VTKBody::SessionGetBodies(TestGetCurrentVersion(), bodies);
    CheckM(bodies.size() == 1);
    VTKBody vBody = bodies[0];
    int arcEdge = vBody.FindEntityByIdentifier(EntityClass::Edge, 134);
    IntSet expectedFaceTagsSet;
    expectedFaceTagsSet.insert(vBody.FindEntityByIdentifier(EntityClass::Face, 35));  // Plane
    expectedFaceTagsSet.insert(vBody.FindEntityByIdentifier(EntityClass::Face, 161));  // Cylinder
    IntSet returnedFaces;

    pkUtils.EdgeGetFaces(arcEdge, returnedFaces);

    CheckSizeM(returnedFaces, 2);
    CheckM(returnedFaces == expectedFaceTagsSet);

    return true;
}

VTK_TEST_FN(TestPKUtils093a_MinimisePKEntityIsCallsDuringConfigure_ConfigureCase)
{
    // TODO What tests should I have and where should I put them (in one place or in their respective places)
    // TODO these technically aren't pkutils tests so maybe they should go in separate areas or even a new file)
    // TODO overall configure
    // TODO geometric finding test
    // TODO chamfer finding maybe? probably not
    // TODO solver? although the solver code is fractured so maybe overall configure would work
    // TODO Do I do this at a unit or api level? could do it at an api level for simplicity, but then we can only call configure
    // TODO but I worry that if we call anything more specific then we will miss loads of areas, so we need broad tests

    return true;
}

VTK_TEST_FN(TestPKUtils094_CreateWirebody_Finds_Accurate_Intersection_Between_Line_And_OffsetCurve)
{
    //Test for PR10001811, to show we generate accurate enough intersection between a line and an offset curve, by
    //using a tighter tolerance in PKUtils::CurveDataUtils::OffsetCurveCheckPositionOfApproximateCurveAtParameter when
    //checking if the approximation is accurate.
    InteractionTag interaction = CreateInteraction();
    VTK::OptionSetProfileSolverType(interaction, ProfileSolverType2D);

    EllipseData ellipseData(Position(0.1303239481960395, 0.02597705572526405, 0), Direction(-1.110223024625157e-16, -1.554312234475219e-15, 1), Direction(0.8787896032528926, 0.4772094228057778, 9.992007221626409e-16), 0.1054748554063213, 0.08434999999999736);
    int ellipse = VTK::CreateEllipse(interaction, ellipseData);
    OffsetCurveData offsetData(ellipse, 0, OffsetCurveSideRight);
    int offset = VTK::CreateOffsetCurve(interaction, offsetData);
    LineData lineData(Position(0.04572150514083245, -0.04152285914851424, 0), Direction(0.9403573935646512, 0.3401881426039065, 0));
    int line = VTK::CreateLine(interaction, lineData);

    TagArray curves;
    curves.Add(ellipse);
    curves.Add(offset);
    curves.Add(line);
    IntervalArray intervals;
    intervals.Add(Interval(1.7573081331041, 4.525877174075486));
    intervals.Add(Interval(1.954852349668871, 3.3754226458485861)); //3.3754226458485861 is the true accurate parameter position, which should give us the most accurate result. NX used 3.375422646128886 in their journal.
    intervals.Add(Interval(0, 0.003999999999999955));

    TagArray createdBodies, createdEdges;
    VTK::CreateWireBodies(interaction, curves, intervals, TagNull, createdBodies, createdEdges);
    CheckM(createdEdges.GetSize() == 3);

    //Expect the length to be much more accurate than Resabs.
    double lineLength = 0;
    PKUtils().EdgeGetLength(createdEdges[2], lineLength);
    CheckM(ResnorEqual(lineLength, 0.004));

    return true;
}

VTK_TEST_FN(TestPKUtils095_GeomCheck_Ignores_SelfIntersectingState_If_CheckSelfIntSessionOptionIsOff)
{
    //Check PKUtils::GeomCheck ignores self-intersecting state if the session option "check_self_int" option is off.
    ResultType result = ResultTypeOk;
    InteractionTag interaction = CreateInteraction();

    PositionArray controlPoints;
    DoubleArray knots;

    controlPoints.Add(Position(0, 0, 0));
    controlPoints.Add(Position(5, 1, 0));
    controlPoints.Add(Position(4, 2, 0));
    controlPoints.Add(Position(3, 3, 0));
    controlPoints.Add(Position(2, 2, 0));
    controlPoints.Add(Position(4, 1, 0));
    controlPoints.Add(Position(6, 0, 0));
    knots.Add(0);
    knots.Add(0);
    knots.Add(0);
    knots.Add(0);
    knots.Add(0.25);
    knots.Add(0.5);
    knots.Add(0.75);
    knots.Add(1);
    knots.Add(1);
    knots.Add(1);
    knots.Add(1);

    GeometryTag spline1 = VTK::CreateSplineCurve(interaction, SplineCurveData(3, false, knots, controlPoints, DoubleArray()), &result);
    CheckResultM(result);
    CheckTagM(spline1);

    PK_LOGICAL_t checkSelfInt = PK_LOGICAL_true;
    PK_SESSION_ask_check_self_int(&checkSelfInt);
    CheckM(checkSelfInt == PK_LOGICAL_true);
    //The spline is self-intersecting so it fails the check when the session option is ON.
    bool checkPassed = PKUtils().GeomCheck(spline1);
    CheckM(!checkPassed);
    //The spline passes the check if the session option is switched off.
    PK_SESSION_set_check_self_int(PK_LOGICAL_false);
    checkPassed = PKUtils().GeomCheck(spline1);
    CheckM(checkPassed);

    //Restore the original setting.
    PK_SESSION_set_check_self_int(PK_LOGICAL_true);
    return true;
}


VTK_TEST_FN(TestPKUtils999_SortingHatPointCloudToSolid)
{
    //IntArray measurements = { 292,286,284,277,275,268,265,259,257,254,249,247,244,243,240,240,242,241,242,243,248,248,253,257,262,265,268,273,276,281,285,290,296,297,302,303,308,307,310,311,313,310,312,311,311,309,307,304,304,299,295,293,286,285,278,276,267,267,260,256,252,251,247,247,243,241,242,241,241,245,245,249,249,255,257,261,264,270,275,277,283,288,290,294,298,303,304,307,308,311,311,312,313,313,312,310,310,307,304,302,301,297,291,288,283,279,274,271,266,262,257,254,249,248,244,245,243,242,241,243,243,245,248,252,255,257,261,267,269,275,277,284,287,291,295,299,301,304,307,310,310,312,311,313,311,313,312,310,306,305,303,299,294,292,287,283,277,275,268,266,261,258,254,249,248,245,244,243,240,243,242,244,245,249,250,255,259,263,267,270,275,279,283,288,291,296,297,302,304,307,308,311,312,313,312,313,312,311,309,308,305,301,299,295,291,286,282,279,275,269,264,262,256,252,250,248,246,244,242,242,241,242,244,247,248,252,254,259,261,267,271,276,278,284,287,291,293,299,303,305,306,308,311,310,311,313,313,312,310,308,307,304,302,300,295 };
    IntArray measurements = { 206,203,201,270,202,182,153,237,259,445,179,183,191,201,201,203,204,205,210,212,218,220,223,223,223,223,225,223,225,223,223,220,222,220,222,213,224,280,224,226,226,225,226,226,223,218,217,215,213,208,352,355,346,345,348,346,310,182,233,347,346,345,348,349,349,352,356,354,354,360,365,221,223,226,227,227,226,229,227,228,229,226,226,224,380,385,385,227,228,333,335,338,344,355,365,351,259,335,348,359,350,348,350,347,346,343,341,184,173,248,461,183,346,344,347,349,352,351,355,215,359,358,223,230,225,228,375,375,374,376,378,379,378,381,382,383,381,224,227,362,352,352,355,358,363,370,370,353,347,351,352,212,352,350,346,344,346,344,344,346,332,345,343,344,345,347,345,348,351,355,355,217,221,224,224,224,372,374,372,375,377,378,377,378,379,379,380,383,229,371,372,363,364,366,357,348,327,379,325,343,350,355,219,364,352,347,342,343,341,343,345,338,342,342,342,344,345,349,350,354,351,354,358,362,221,370,374,373,372,372,375,376,378,379,380,381,380,381,387,229,318,330,331,379,379,383,380,381,380,378,376,371,367,362,358,356,354,352,349,343,342,345,341,345,344,344,343,345,348,350,351,356,355,362,364,366,369,371,374,376,377,382,381,383,382,383,384,385,386,384,382,384,382,383,383,235,236,234,233,378,375,377,372,367,362,356,354,351,349,348,349,347,346,343,343,344,345,347,349,351,352,353,357,360,364,366,373,375,376,378,381,381,382,383,383,382,380,382,381,384,384,385,383,382,379,379,378,381,380,381,374,372,370,367,364,358,357,355,350,348,348,347,348,348,349,350,350,351,354,350,351,354,355,358,368,370,373,375,377,380,381,380,381,383,380,381,380,382,380,380,381,385,382,383,383,384,383,382,383,374,376,377,376,373,370,367,362,357,357,353,352,350,350,353,354,351,351,352,354,354,356,359,361,365,368,371,373,374,375,376,378,381,381,381,379,378,377,376,377,379,381,383,382,379,380,380,382,383,379,372,372,374,381,383,386,383,379,364,359,357,352,354,355,358,358,357,358,359,359,358,357,358,359,360,362,363,365,368,369,368,367,369,372,373,373,375,376,376,377,377,382,385,388,388,388,380,383,383,377,374,379,389,392,391,389,387,383,381,363,361,359,355,356,359,361,363,359,364,364,361,358,356,355,357,358,365,363,359,360,364,368,373,381,381,381,381,386,387,388,383,384,386,389,390,390,390,387,386,387,380,387,391,392,391,391,389,384,376,371,362,360,357,358,360,364,365,366,366,367,367,364,361,359,359,360,361,369,367,366,365,368,374,383,382,382,382,383,383,388,392,393,385,388,390,393,388,389,391,388,390,387,391,390,390,388,387,383,380,376,374,360,359,359,360,364,368,370,370,370,369,369,366,363,365,366,368,369,368,369,381,384,385,384,382,382,383,387,386,392,392,393,393,396,395,395,394,392,392,393,391,392,391,393,395,399,398,396,394,392,388,385,361,361,360,362,366,369,372,373,372,372,370,366,365,367,368,371,371,378,382,383,386,386,386,384,385,385,385,383,387,390,392,393,392,391,389,387,389,391,391,393,397,405,403,403,403,401,397,394,391,387,383,363,362,361,363,367,370,372,372,372,370,367,368,367,368,370,373,378,381,384,386,387,387,387,386,383,383,383,382,383,389,393,393,389,388,388,390,391,398,404,407,406,406,402,402,401,398,394,393,387,376,368,360,360,360,363,365,372,373,373,369,367,368,368,369,370,376,376,380,384,386,388,388,388,388,386,385,384,383,384,382,387,391,397,398,401,399,404,402,405,405,405,403,399,398,399,398,397,396,392,385,374,368,363,362,358,358,361,366,369,367,365,366,367,369,370,371,374,380,380,384,386,388,389,392,390,388,387,386,383,381,381,385,389,394,396,398,399,399,401,403,403,406,396,398,393,396,395,395,393,389,385,370,367,362,360,356,357,361,363,365,366,365,367,371,371,375,375,379,386,386,391,391,394,395,394,394,392,391,391,388,388,384,384,389,390,392,393,395,394,395,395,394,392,393,395,396,396,397,395,395,392,387,381,375,370,370,369,357,359,361,363,364,365,368,372,373,375,377,380,383,385,388,389,391,394,395,395,394,392,390,388,387,384,387,390,388,387,387,388,390,391,391,392,395,397,398,399,399,398,397,396,392,388,380,377,374,374,370,366,362,364,365,367,370,374,379,380,381,382,382,385,388,390,390,392,395,396,395,395,394,392,390,390,388,389,391,391,387,384,382,383,389,390,394,399,400,400,401,401,399,399,398,394,391,385,379,376,375,371,369,369,371,372,372,377,380,382,384,386,387,389,390,391,392,394,397,398,399,399,401,396,396,395,395,394,395,393,396,397,385,382,383,387,391,396,399,401,402,403,402,399,399,398,395,384,379,374,375,371,370,369,372,377,377,379,383,385,388,390,393,393,395,397,398,399,400,402,403,404,404,404,405,402,400,402,399,398,393,395,399,405,382,382,387,391,395,399,401,401,400,398,398,397,395,392,388,383,381,382,383,380,378,378,378,380,381,384,385,386,389,392,395,396,396,398,398,399,402,405,407,408,410,408,401,402,401,399,398,395,400,400,403,385,386,390,394,399,402,401,404,404,400,402,399,397,393,387,385,385,383,384,381,380,381,382,384,386,387,389,391,394,395,399,400,401,402,402,405,407,412,416,416,412,410,405,404,405,406,406,407,403,407,406,422,390,392,396,399,402,405,411,411,412,402,403,405,400,387,386,387,389,388,388,387,387,387,387,386,385,388,389,392,395,397,399,399,400,402,405,409,414,414,412,412,410,407,406,407,409,410,412,406,413,416,426,393,395,402,402,410,412,412,413,412,411,405,405,408,386,386,387,391,390,391,392,394,395,396,402,387,380,388,392,394,397,398,400,402,404,409,415,421,420,417,415,413,411,411,415,413,421,421,422,424,425,425,425,402,404,408,410,411,412,412,412,413,409,399,390,388,386,390,392,393,394,394,397,400,400,405,412,411,392,396,402,408,423,425,426,428,428,428,426,425,422,422,420,422,415,419,421,422,426,429,430,429,429,433,404,400,404,407,409,410,410,408,404,396,393,391,387,387,390,393,394,395,397,400,401,407,409,412,414,419,417,419,421,424,424,426,427,427,428,428,427,426,426,425,423,422,419,421,425,428,431,430,430,431,394,393,392,395,403,404,404,403,398,394,393,392,391,390,390,394,395,395,396,398,401,404,408,411,414,418,425,420,424,420,427,426,426,427,427,430,429,429,427,428,426,427,423,421,423,426,428,428,429,431,395,393,395,392,387,385,386,389,389,390,393,393,394,395,394,398,404,408,416,400,400,401,406,408,413,418,421,425,432,435,432,433,433,432,431,432,434,434,437,438,437,435,434,490,425,425,426,426,427,430,435,393,397,395,392,389,383,381,382,387,390,394,396,398,400,399,403,407,414,419,423,426,432,408,412,415,420,424,428,431,438,442,447,456,473,475,478,480,483,487,490,490,490,490,490,490,490,427,426,427,432,395,397,399,395,391,389,390,394,400,405,408,407,401,402,403,404,407,413,417,420,425,428,431,434,438,441,444,445,449,449,456,462,466,469,467,474,476,479,486,490,490,490,490,490,490,490,490,490,430,429,398,413,407,403,394,390,389,391,394,399,405,408,411,412,411,410,411,412,415,418,423,426,430,434,438,442,445,448,452,455,461,465,449,452,455,452,454,456,457,458,461,467,490,490,490,490,490,425,427,429,422,418,413,410,403,395,393,393,396,400,405,410,412,414,415,416,416,419,421,424,421,424,427,431,435,438,441,445,449,452,454,457,461,463,466,469,472,476,485,490,490,490,490,490,490,490,490,490,490,490,490,434,439,420,413,406,393,394,397,396,401,408,410,412,413,414,416,417,422,426,429,429,428,432,433,437,441,444,448,451,455,457,460,463,466,468,471,474,479,487,490,490,490,490,490,490,490,490,490,490,490,490,490,435,432,427,412,395,396,403,406,407,409,411,412,414,415,415,419,421,425,430,435,438,438,437,440,445,447,451,454,457,460,463,466,468,471,474,477,490,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,424,416,413,400,400,400,397,396,401,404,406,408,410,414,418,419,427,429,436,441,441,444,448,452,457,460,464,467,471,474,464,467,469,472,478,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,421,423,408,402,401,396,393,394,396,400,403,406,408,412,415,419,423,429,432,436,441,447,451,455,459,464,467,472,473,477,480,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,423,408,406,399,392,392,393,397,401,406,409,412,415,418,422,425,429,434,438,443,448,452,456,461,465,469,473,475,480,485,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,413,401,400,400,400,397,394,396,399,404,407,412,416,421,425,430,435,440,445,452,456,461,466,471,474,476,481,485,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,415,407,405,401,399,399,396,400,401,403,408,413,419,423,428,431,438,443,450,456,461,468,472,473,474,471,475,479,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,426,423,416,416,411,410,408,411,413,412,417,419,424,428,432,436,439,443,443,446,449,454,457,458,463,467,473,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,425,420,414,404,400,403,404,408,409,414,417,422,441,444,452,456,463,469,474,480,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,430,420,409,397,397,398,402,404,410,416,421,426,434,440,446,454,463,468,474,479,482,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,399,384,384,387,392,397,403,414,423,433,440,448,456,462,469,477,481,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,395,392,388,389,396,398,406,414,425,435,442,451,459,466,474,482,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,400,401,403,403,407,412,417,426,433,442,448,455,462,468,477,483,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,394,394,399,405,416,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,484,382,386,394,403,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,384,386,391,397,410,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,471,370,372,378,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,457,362,366,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,459,359,365,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,349,367,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490 };
    int stepstoCOR = 478;
    int stepsPerRev51 = 51;
    double revolutionHeight = 10.0/6.0; // 0.8...mm
    double distanceToCOR = 87.3; //# mm
    double factor = 6.17669;
    
    double xHome = 0;
    double yHome = 0;
    double stepAngleDegrees = 360.0 / stepsPerRev51;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    int rev_i = 0;

    int nLayers = 32;

    VTKBodyArray bodySlices;

    for (int rev_i = 0; rev_i < nLayers; rev_i++)
    {
        VecArray layer;
        for (int step_i = 0; step_i < stepsPerRev51; step_i++)
        {
            int indexOffsetForLayer = rev_i * stepsPerRev51;
            int step = measurements[indexOffsetForLayer + step_i];
            step = stepstoCOR - step;
            double measurementInMM = step / factor;
            double measuredRadius = measurementInMM;
            Vec pos;
            pos.x = xHome + (measuredRadius * cos(stepAngleRads * step_i));
            pos.y = yHome + (measuredRadius * sin(stepAngleRads * step_i));
            pos.z = rev_i * revolutionHeight;
            layer.push_back(pos);
        }

        int nPos = (int)layer.size();
        PK_VECTOR_t* pkVectors = new PK_VECTOR_t[nPos];
        for (int t = 0; t < nPos; t++)
        {
            pkVectors[t] = layer[t].PkVector();
        }

        PK_BODY_create_sheet_planar_o_t pkSheetOpt;
        PK_BODY_create_sheet_planar_o_m(pkSheetOpt);
        pkSheetOpt.plane.location = pkVectors[0];
        pkSheetOpt.plane.axis = Dir::ZDir().GetVec().PkVector();
        PK_BODY_t       fenceBody = 0;
        PK_ERROR_code_t pkError = PK_BODY_create_sheet_planar(nPos, pkVectors, &pkSheetOpt, &fenceBody);
        CheckM(PK_ERROR_no_errors == pkError);

        delete[] pkVectors;

        VTKBody vb(fenceBody);
        IntIntPairArray ignore;
        double layerHeight = revolutionHeight * 1.1;
        bool sweptOk = vb.Sweep(Dir::ZDir(), layerHeight, ignore);
        CheckM(sweptOk);

        bodySlices.push_back(vb);
        
    }

    //VTKBody::TransmitBodies(VTK::Version_25_00, bodySlices, "D:\\temp.x_t");
   

    PK_BODY_boolean_o_t boolOptions = { 0 };
    PK_BODY_boolean_o_m(boolOptions);
    boolOptions.function = PK_boolean_unite_c;
    boolOptions.merge_imprinted = PK_LOGICAL_false; // PK_LOGICAL_true; we don't want to merge this leaves our tag to disappear
    PKTopolTrackResults tracking;
    PK_boolean_r_t boolRes = { 0 };
    PK_ERROR_t pkError = 0;
    for (int b = 1; b < bodySlices.size(); b++)
    {
        pkError = bodySlices[0].Boolean(bodySlices[b], &boolOptions, &tracking, &boolRes);
        CheckM(PK_ERROR_no_errors == pkError);
        tracking.Clear();
        PK_boolean_r_f(&boolRes);
    }

    bodySlices[0].Transmit(VTK::Version_25_00, "D:\\temp.x_t");


    return true;
}

void SetCell(BYTE* sliceGrid,int xMax, int x, int y, bool val)
{
    sliceGrid[y * xMax + x] = (val) ? 1 : 0;
}

bool GetCell(BYTE* sliceGrid, int xMax, int x, int y)
{
    return (sliceGrid[y * xMax + x]) ==1;
}

void SetCellVal(BYTE* sliceGrid, int xMax, int x, int y, int value)
{
    sliceGrid[y * xMax + x] = value;
}

int GetCellVal(BYTE* sliceGrid, int xMax, int x, int y)
{
    return (int)(sliceGrid[y * xMax + x]);
}

bool isValidXY(int x, int y, int xMax, int yMax)
{
    if (x < 0)
        return false;
    if (y < 0)
        return false;
    if (x >= xMax)
        return false;
    if (y >= yMax)
        return false;
    return true;
}

double DistanceFromCor(int xMax, int yMax, int x, int y)
{
    double midX = xMax / 2.0;
    double midY = yMax / 2.0;
    double x2 = (x - midX) * (x - midX);
    double y2 = (y - midY) * (y - midY);
    double d = sqrt(x2 + y2);
    return d;
}


void printSlice(BYTE* sliceGrid, int xMax, int yMax)
{
    if (true)
    {


        std::cout << std::endl;
        for (int y = 0; y < yMax; y++)
        {
            String row = "";
            for (int x = 0; x < xMax; x++)
            {
                row.append(StringUtils::Format("%u", GetCellVal(sliceGrid, xMax, x, y)).c_str());
            }
            std::cout << row << std::endl;
        }
        std::cout << std::endl;
    }
}

IntArray HasCellWithValue(BYTE* sliceGrid, int xMax, int yMax, int value)
{
    for (int y = 0; y < yMax; y++)
    {
        for (int x = 0; x < xMax; x++)
        {
            if (GetCellVal(sliceGrid, xMax, x, y) == value)
            {
                return IntArray{ x,y };
            }
        }
    }
    return IntArray{};
}


void ContiguousExpandFromStartPointBy8(BYTE* sliceGrid, int xMax, int yMax, int xStart, int yStart, int regionKey)
{
    IntIntPairSet expanded;
    IntIntPairSet toExpand;

    toExpand.insert(IntIntPair(xStart, yStart));

    while (!toExpand.empty())
    {
        IntIntPairSet nextToExpand;

        for (auto p : toExpand)
        {
            int xBase = p.first;
            int yBase = p.second;
            // This cell becomes a member of this region
            DEB_assert( GetCell(sliceGrid, xMax, xBase, yBase) );
            // NB this sets the value to a number > 1
            SetCellVal(sliceGrid, xMax, xBase, yBase, regionKey);

            // For every cell in the toExpand set, get the 8 surrounding cells
            
            for (int y = yBase - 1; y <= yBase + 1; y++)
            {
                for (int x = xBase - 1; x <= xBase + 1; x++)
                {
                    if (y == yBase && x == xBase)
                        continue;
                    if (isValidXY(x, y, xMax, yMax) && GetCell(sliceGrid, xMax, x, y))
                    {
                        IntIntPair candidate(x, y);
                        if (expanded.find(candidate) == expanded.end() && toExpand.find(candidate) == toExpand.end())
                        {
                            // if the cells have value KEY and are not already in the expanded or the toExpand set
                            // add them to the nextToExpand set 
                            nextToExpand.insert(candidate);
                        }
                    }
                }
            }
            // We just expanded p
            expanded.insert(p);
        }
        toExpand.swap(nextToExpand);
        nextToExpand.clear();
        //printSlice(sliceGrid, xMax, yMax);
    }
}

int ContiguousExpandFromStartPointBy4(BYTE* sliceGrid, int xMax, int yMax, int xStart, int yStart, int regionKey)
{
    IntIntPairSet expanded;
    IntIntPairSet toExpand;

    toExpand.insert(IntIntPair(xStart, yStart));

    while (!toExpand.empty())
    {
        IntIntPairSet nextToExpand;

        for (auto p : toExpand)
        {
            int xBase = p.first;
            int yBase = p.second;
            // This cell becomes a member of this region
            DEB_assert(GetCell(sliceGrid, xMax, xBase, yBase));
            // NB this sets the value to a number > 1
            SetCellVal(sliceGrid, xMax, xBase, yBase, regionKey);

            // For every cell in the toExpand set, get the 8 surrounding cells

            IntIntPairArray valid;
            valid.push_back(IntIntPair(-1, 0)); // W
            valid.push_back(IntIntPair(1,  0)); // E
            valid.push_back(IntIntPair(0, -1)); // N
            valid.push_back(IntIntPair(0,  1)); // S

            
            for (IntIntPair offset : valid)
            {
                int y = yBase + offset.first;
                int x = xBase + offset.second;

                if (isValidXY(x, y, xMax, yMax) && GetCell(sliceGrid, xMax, x, y))
                {
                    IntIntPair candidate(x, y);
                    if (expanded.find(candidate) == expanded.end() && toExpand.find(candidate) == toExpand.end())
                    {
                        // if the cells have value KEY and are not already in the expanded or the toExpand set
                        // add them to the nextToExpand set 
                        nextToExpand.insert(candidate);
                    }
                }
            }
            // We just expanded p
            expanded.insert(p);
        }
        toExpand.swap(nextToExpand);
        nextToExpand.clear();
        //printSlice(sliceGrid, xMax, yMax);
    }
    return (int)expanded.size();
}


IntArray FindAndNumberDisjointRegionsInSlice(BYTE* sliceGrid, int xMax, int yMax)
{
    // This will change values in sliceGrid
    // All zero values will remain 0
    // All 1 values will be replaced with a higher number based on which region their are contiguous members of
    // Diagonal connections will not be allowed

    int regionKey = 2;
    IntArray regionKeys;

    while (true)
    {
        IntArray startFrom = HasCellWithValue(sliceGrid, xMax, yMax, 1);
        if (startFrom.empty())
            break;
        // Found a starting point
        // Mark all contiguous as part of this region
        int regionSize = ContiguousExpandFromStartPointBy4(sliceGrid, xMax, yMax, startFrom[0], startFrom[1], regionKey);
        // Repeat whilst there are more starting points
        if (regionSize > 2)
        {
            regionKeys.push_back(regionKey);
        }
        regionKey += 1;
    }

    return regionKeys;
}

static IntIntPair pNORTH(0, -1);
static IntIntPair pEAST(1, 0);
static IntIntPair pSOUTH(0, 1);
static IntIntPair pWEST(-1, 0);

void GoLeft(IntIntPair& p) {
    // Change current hearing p, 90 degrees anti-clockwise
    if (p == pNORTH) // N
    {
        p.first = pWEST.first;
        p.second = pWEST.second;
    }
    else if (p == pEAST) // E
    {
        p.first = pNORTH.first;
        p.second = pNORTH.second; // N
    }
    else if (p == pSOUTH) // S
    {
        p.first = pEAST.first;
        p.second = pEAST.second; // E
    }
    else if (p == pWEST) // W
    {
        p.first = pSOUTH.first;
        p.second = pSOUTH.second; // S
    }
}
void GoRight(IntIntPair& p) { 
    // Change current hearing p, 90 degrees clockwise
    if (p == pNORTH) // N
    {
        p.first = pEAST.first;
        p.second = pEAST.second; // E
    }
    else if (p == pEAST) // E
    {
        p.first = pSOUTH.first;
        p.second = pSOUTH.second; // S
    }
    else if (p == pSOUTH) // S
    {
        p.first = pWEST.first;
        p.second = pWEST.second; // W
    }
    else if (p == pWEST) // W
    {
        p.first = pNORTH.first;
        p.second = pNORTH.second; // N
    }
}

void TraceOutlinesOfNumberedDisjointRegionInSlice(BYTE* sliceGrid, int xMax, int yMax, int regionKey, VecArray& layerPerim, double zValue)
{
    // https://en.wikipedia.org/wiki/Boundary_tracing
    // From all contiguous cells labelled with regionKey, find the Vec positions of the perimeter

    // There are n-contiguous cells in sliceGrid with value 'regionKey' - find the perimeter
    // First, scan from the upper left to right and row by row to find first cell of region key
    //Upon entering your first white cell, the core of the algorithm starts.It consists mainly of two rules :
    //
    // If you are in a white cell, go left.
    // If you are in a black cell, go right.
    // Keep in mind that it matters how you entered the current cell, so that left and right can be defined.
    IntArray startFrom = HasCellWithValue(sliceGrid, xMax, yMax, regionKey);
    if (startFrom.empty())
        return;
    //DEB_assert(startFrom.size() > 0);
    IntIntPair start(startFrom[0], startFrom[1]);

    IntIntPairSet boundaryPointsSet; //prevent double occurrences
    IntIntPairArray boundaryPointsOrderedWalk;

     // We found at least one pixel
    boundaryPointsSet.insert(start);
    boundaryPointsOrderedWalk.push_back(start);

    // The first pixel you encounter is a white one by definition, so we go left. 
    // Our initial direction was going from left to right, hence (1, 0)
    IntIntPair nextStep;
    nextStep.first = pEAST.first;
    nextStep.second = pEAST.second;
    GoLeft(nextStep);
    IntIntPair next;
    next.first = start.first + nextStep.first;
    next.second = start.second + nextStep.second;

    while (next != start)
    {
        // We found a black cell, so we go right and don't add this cell to our HashSet
        int cellVal = GetCellVal(sliceGrid, xMax, next.first, next.second);
        if (cellVal != regionKey)
        {
            //next = next - nextStep;
            next.first -= nextStep.first;
            next.second -= nextStep.second;
            //next = next + nextStep;
            GoRight(nextStep);
            next.first += nextStep.first;
            next.second += nextStep.second;
        }
        // Alternatively we found a white cell, we do add this to our HashSet
        else
        {
            // Only store unique
            if (boundaryPointsSet.find(next) == boundaryPointsSet.end())
            {
                boundaryPointsSet.insert(next);
                boundaryPointsOrderedWalk.push_back(next);
            }
            GoLeft(nextStep);
            //next = next + nextStep;
            next.first += nextStep.first;
            next.second += nextStep.second;
        }
    }


    if (!boundaryPointsOrderedWalk.empty())
    {
        bool resetInteriorTo0 = true;
        if (resetInteriorTo0)
        {
            for (int y = 0; y < yMax; y++)
            {
                for (int x = 0; x < xMax; x++)
                {
                    int cellVal = GetCellVal(sliceGrid, xMax, next.first, next.second);
                    if (cellVal == regionKey)
                    {
                        if (boundaryPointsSet.find(IntIntPair(x, y)) == boundaryPointsSet.end())
                        {
                            SetCell(sliceGrid, xMax, x, y, false);
                        }
                    }
                }
            }
        }
        printSlice(sliceGrid, xMax, yMax);

        bool skipFirst = true;
        double scale = 1.0;
        for (auto p : boundaryPointsOrderedWalk)
        {
            if (skipFirst)
            {
                skipFirst = false;
                continue;
            }
            Vec forPoint(scale * p.first, scale * p.second, zValue);
            layerPerim.push_back(forPoint);
            //SetCellVal(sliceGrid, xMax, p.first, p.second, 9);
        }
        
    }
}




bool CarveSlice(int layerIndex, IntArray& allMeasurements, VecArrayArray& layerPerims, double zValue)
{
    // The voxel region for a slice is 180 x 180mm
    int xMax = 180;
    int yMax = 180;
    int area = xMax * yMax;
    BYTE* sliceGrid = new BYTE[area];
    // The center of rotation is in the middle of the voxel grid

    int processLayer = 0;

    // Turn off all cells...
    for (int y = 0; y < yMax; y++) {
        for (int x = 0; x < xMax; x++) {
            SetCell(sliceGrid, xMax, x, y, false);
        }
    }

    double tableCenterX = xMax / 2.0;
    double tableCenterY = yMax / 2.0;


    int stepsPerRev103 = 103;
    double stepAngleDegrees = 360.0 / stepsPerRev103;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    {
        double stepstoCOR = 416.7;
        double revolutionHeight = 10.0 / 6.0; // 0.8...mm
        double distanceToCOR = 67.21; // mm
        double factor = -6.202;
        double probeWidth = 5.0 / 2.0; // m
        int rev_i = layerIndex;
        double maxRadius = 80.0;
            
        if (true)
        {
            maxRadius = 1.0; // mm
            // Pre-scan for smallest measurment 
            for (int step_i = 0; step_i < stepsPerRev103; step_i++)
            {
                int indexOffsetForLayer = rev_i * stepsPerRev103;
                int step = allMeasurements[indexOffsetForLayer + step_i];
                double measurementInMM = (step - stepstoCOR) / factor;
                double measuredRadius = measurementInMM;
                if (measuredRadius > maxRadius)
                {
                    maxRadius = measuredRadius;
                }
            }
            printSlice(sliceGrid, xMax, yMax);
        }

        if (true)
        {
            // Turn on all cells within radius of X table...        
            for (int y = 0; y < yMax; y++) {
                for (int x = 0; x < xMax; x++) {
                    double distanceFromCenter = DistanceFromCor(xMax, yMax, x, y);
                    if (distanceFromCenter <= 80)
                    {
                        SetCellVal(sliceGrid, xMax, x, y, 7);
                    }
                }
            }
            // At the point we should have a complete disk.
            printSlice(sliceGrid, xMax, yMax);
        }


        // Turn on all cells within radius of X table...
        for (int y = 0; y < yMax; y++) {
            for (int x = 0; x < xMax; x++) {
                double distanceFromCenter = DistanceFromCor(xMax, yMax, x, y);
                if (distanceFromCenter <= maxRadius)
                {
                    SetCell(sliceGrid, xMax, x, y, true);
                }
            }
        }
        // At the point we should have a complete disk.
        printSlice(sliceGrid, xMax, yMax);

        // 

        for (int step_i = 0; step_i < stepsPerRev103; step_i++)
        {
            int indexOffsetForLayer = rev_i * stepsPerRev103;
            int step = allMeasurements[indexOffsetForLayer + step_i];
            double measurementInMM = (step - stepstoCOR) / factor;
            double measuredRadiusFromCor = measurementInMM;
            //Vec pos;
            //pos.x = xHome + (measuredRadius * cos(stepAngleRads * step_i));
            //pos.y = yHome + (measuredRadius * sin(stepAngleRads * step_i));
            //pos.z = rev_i * revolutionHeight;

            // Recall that the COR is in the middle of the output array...
            double offsetDistance = -2.5;
            double angle = (stepAngleRads * step_i);
            double angleMinus90 = angle - (PI/2.0);
            
            //Vec linePos(tableCenterX + (offsetDistance * cos(angleMinus90)), tableCenterY - (offsetDistance * sin(angleMinus90)),0.0);
            //Vec outsidePos(tableCenterX+(distanceToCOR * cos(stepAngleRads * step_i)), tableCenterY-(distanceToCOR* sin(stepAngleRads * step_i)), 0.0) ;
            //outsidePos = linePos + outsidePos;

            Vec linePos(tableCenterX, tableCenterY,0.0);
            // To generate a position on the boundary of the circle, it must be relative to the COR
            Vec outsidePos(tableCenterX+(distanceToCOR * cos(stepAngleRads * step_i)), tableCenterY-(distanceToCOR* sin(stepAngleRads * step_i)), 0.0) ;
            
            Dir lineDir = Normal(outsidePos-linePos);
            // Iterate over output array

            for (int y = 0; y < yMax; y++)
            {
                for (int x = 0; x < xMax; x++)
                {
                    Vec v(1.0 * x, 1.0 * y, 0.0);
                    Vec projection = GeomUtils::ProjectPointOnLine(v, linePos, lineDir);
                    double cellDistanceFromTraceLine = (v - projection).Mag();

                    
                    //double distanceToCOR = 67.21; // mm
                    double probeTravelDistance = distanceToCOR - measuredRadiusFromCor;

                    double cellDistanceFromOutsidePos = (outsidePos - v).Mag();
                    
                    // At the center of rotation I want the test to be distance < (1.0*probeWidth)
                    // At distanceFromM, I want the test to be distance < (2.25*probeWidth)
                    double factor = 1.0;// 1 + (1.6 * ((distanceToCOR - distanceOfXYFromCor) / distanceToCOR));
                    double testDistance = factor * probeWidth;

                    if (ResEqual(cellDistanceFromTraceLine, testDistance) || cellDistanceFromTraceLine < testDistance)
                    {
                        // This is in the right angle, does it need turning off?
                        
                        if (ResEqual(cellDistanceFromOutsidePos, probeTravelDistance) || cellDistanceFromOutsidePos < probeTravelDistance)
                        {
                            // Turn this cell off
                            SetCell(sliceGrid, xMax, x, y, false);
                        }
                    }
                }
            }
            printSlice(sliceGrid, xMax, yMax);
           int bob = 1;
            //for a given X/Y
                //1a) project onto vector
                //1b) measure distance to projection
                //2) measure distance from COR


        }
    }



   // printSlice(sliceGrid, xMax, yMax);
    double zCoordinateOfSlice = zValue;
    IntArray regionKeys = FindAndNumberDisjointRegionsInSlice(sliceGrid, xMax, yMax);

    printSlice(sliceGrid, xMax, yMax);

    // Obtain the outlines of all bounded regions within this slide as VecArrayArray
    for (auto key : regionKeys)
    {
        VecArray perim;
        TraceOutlinesOfNumberedDisjointRegionInSlice(sliceGrid, xMax, yMax, key, perim, zCoordinateOfSlice);
        layerPerims.push_back(perim);
    }
    printSlice(sliceGrid, xMax, yMax);


    // Turn 
    delete[] sliceGrid;

    return true;
}



VTK_TEST_FN(TestPKUtils999_SpaceCarveSpriteCan001)
{

    IntArray measurements = {290,289,287,289,289,289,290,289,288,288,289,285,285,284,284,286,283,281,279,278,278,277,277,263,263,266,269,266,265,263,262,259,258,257,253,250,248,248,245,245,244,242,230,227,228,229,231,233,235,235,237,236,231,234,234,236,221,218,218,223,228,235,241,244,227,229,247,248,248,248,250,254,255,258,253,255,255,257,264,268,271,273,275,276,278,280,282,283,286,286,287,287,288,287,292,292,294,296,296,293,296,295,297,297,287,284,286,290,297,298,297,297,295,294,293,292,291,292,290,289,290,289,286,283,282,267,265,264,267,274,274,272,270,267,265,263,262,257,253,249,249,248,249,249,242,233,232,231,234,234,239,239,239,241,237,237,238,238,239,220,218,216,226,233,237,243,226,227,224,248,248,246,247,252,252,252,252,255,258,257,260,265,271,274,275,278,280,284,284,286,287,288,289,292,290,292,290,294,296,296,297,297,297,299,296,299,301,291,290,290,292,300,300,296,298,298,296,284,284,279,283,285,280,264,271,267,267,270,271,266,266,267,273,274,275,272,271,268,263,264,258,256,254,252,250,250,249,245,235,233,234,233,236,239,239,238,239,238,239,237,237,238,222,220,223,230,232,237,243,229,231,226,235,237,236,239,252,250,245,253,257,251,250,252,265,271,275,278,278,282,281,282,283,287,290,289,291,289,292,290,295,297,298,298,298,299,299,298,301,302,291,290,293,293,299,302,300,300,300,297,265,263,264,291,300,295,276,265,256,255,254,276,259,255,271,276,278,277,276,274,271,270,265,260,258,257,254,254,253,253,249,237,236,235,236,238,240,241,240,240,193,239,240,240,241,224,223,224,229,233,241,204,206,206,209,203,210,210,214,214,218,220,221,220,225,227,228,229,232,232,234,231,238,236,241,241,243,245,248,242,249,247,251,252,253,252,255,253,254,254,254,255,255,255,255,255,255,254,253,254,253,247,250,248,250,245,246,247,243,242,241,239,237,234,233,230,228,224,222,222,219,218,216,213,211,208,207,205,206,201,200,202,197,193,195,193,192,191,191,193,192,189,189,189,189,189,188,188,188,191,189,190,192,194,192,196,196,199,201,202,205,206,207,210,210,210,216,217,222,221,224,224,226,228,226,231,232,235,232,237,237,240,240,242,243,245,246,246,250,248,248,251,249,252,250,250,250,250,251,250,250,249,249,247,248,246,248,244,244,240,240,239,238,236,234,232,229,227,227,224,221,219,219,216,215,211,207,210,206,204,202,201,196,198,198,194,194,191,190,192,188,186,187,184,188,186,185,184,185,186,185,186,187,189,189,190,192,192,194,195,196,199,200,202,203,205,207,210,212,213,214,215,218,220,221,225,226,230,229,230,232,233,235,237,239,240,239,243,243,244,244,247,247,247,246,248,248,249};
    //double stepstoCOR = 416.87;
    //double factor = -6.202;
    //double distanceToCOR = 67.21; //# mm
    //double xHome = 0;
    //double yHome = 0;
    
    int stepsPerRev103 = 103;
    double revolutionHeight = 10.0 / 6.0; // 0.8...mm
    double stepAngleDegrees = 360.0 / stepsPerRev103;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    int maxLayers = (int)measurements.size() / stepsPerRev103;

    int rev_i = 0;
    int nLayers = maxLayers;
    int layerOffset = 0;

    VTKBodyArray bodySlices;

    for (int rev_i = 0; rev_i < nLayers; rev_i++)
    {
        VecArrayArray layerPerims;
        CarveSlice(rev_i + layerOffset, measurements, layerPerims, revolutionHeight * (rev_i + layerOffset));


        // Make sheet from each perim and sweep into body
        for (VecArray layer : layerPerims)
        {
            int nPos = (int)layer.size();
            PK_VECTOR_t* pkVectors = new PK_VECTOR_t[nPos];
            for (int t = 0; t < nPos; t++)
            {
                pkVectors[t] = layer[t].PkVector();
            }

            PK_BODY_create_sheet_planar_o_t pkSheetOpt;
            PK_BODY_create_sheet_planar_o_m(pkSheetOpt);
            pkSheetOpt.plane.location = pkVectors[0];
            pkSheetOpt.plane.axis = Dir::ZDir().GetVec().PkVector();
            PK_BODY_t       fenceBody = 0;
            PK_ERROR_code_t pkError = PK_BODY_create_sheet_planar(nPos, pkVectors, &pkSheetOpt, &fenceBody);
            //CheckM(PK_ERROR_no_errors == pkError);
            if (PK_ERROR_no_errors == pkError)
            {


                VTKBody vb(fenceBody);
                IntIntPairArray ignore;
                double layerHeight = revolutionHeight * 1.1;
                bool sweptOk = vb.Sweep(Dir::ZDir(), layerHeight, ignore);
                //CheckM(sweptOk);
                if (sweptOk)
                {
                    bodySlices.push_back(vb);
                }

            }
            delete[] pkVectors;
        }

    }

    VTKBody::TransmitBodies(VTK::Version_25_00, bodySlices, "D:\\slices.x_t");


    PK_BODY_boolean_o_t boolOptions = { 0 };
    PK_BODY_boolean_o_m(boolOptions);
    boolOptions.function = PK_boolean_unite_c;
    boolOptions.merge_imprinted = PK_LOGICAL_false; // PK_LOGICAL_true; we don't want to merge this leaves our tag to disappear
    PKTopolTrackResults tracking;
    PK_boolean_r_t boolRes = { 0 };
    PK_ERROR_t pkError = 0;
    for (int b = 1; b < bodySlices.size(); b++)
    {
        pkError = bodySlices[0].Boolean(bodySlices[b], &boolOptions, &tracking, &boolRes);
        CheckM(PK_ERROR_no_errors == pkError);
        tracking.Clear();
        PK_boolean_r_f(&boolRes);
    }

    bodySlices[0].Transmit(VTK::Version_25_00, "D:\\temp.x_t");


    return true;
}

void DoCreateCylinder(VTKBody vbody)
{

    PK_CYL_sf_t theCylSF = { 0 };

    theCylSF.radius = 32;
    theCylSF.basis_set.location.coord[0] = 80.0;
    theCylSF.basis_set.location.coord[1] = 90.3;
    theCylSF.basis_set.location.coord[2] = 0;
    theCylSF.basis_set.axis.coord[0] = 0.0;
    theCylSF.basis_set.axis.coord[1] = 0.0;
    theCylSF.basis_set.axis.coord[2] = 1.0;

   
    theCylSF.basis_set.ref_direction.coord[0] = 1.0;
    theCylSF.basis_set.ref_direction.coord[1] = 0.0;
    theCylSF.basis_set.ref_direction.coord[2] = 0.0;
    PK_CYL_t cyl = PK_ENTITY_null;
    PK_ERROR_t error = PK_CYL_create(&theCylSF, &cyl);
    vbody.AddGeoms(cyl);

}


VTK_TEST_FN(TestPKUtils999_SpaceCarveSpriteCan002)
{
    // Full scan

    IntArray measurements = { 287,286,288,288,289,288,288,288,287,286,286,284,283,283,282,280,279,280,278,276,277,262,263,265,270,265,264,262,261,260,257,256,252,249,247,247,244,243,243,243,230,226,227,228,231,233,235,236,235,234,233,234,234,234,219,216,217,222,228,232,239,245,226,228,245,247,247,248,246,254,253,257,255,253,255,257,264,267,271,272,273,272,279,278,281,282,283,285,286,287,288,286,292,292,293,294,296,294,297,298,298,298,288,286,285,289,295,298,298,297,295,293,291,291,288,293,291,289,285,285,282,283,277,270,264,266,265,271,273,270,272,266,265,261,258,255,253,250,248,249,248,247,247,234,225,230,232,233,235,236,237,238,236,235,235,235,236,220,218,215,224,228,234,241,244,226,225,245,247,248,248,249,253,253,252,255,255,254,257,263,267,270,272,274,276,279,281,282,285,286,288,289,289,289,287,293,294,294,295,295,297,297,298,298,299,292,288,287,290,295,300,298,298,299,296,279,273,273,291,291,291,279,261,263,261,256,267,259,258,267,266,272,270,268,267,266,262,262,257,255,253,250,249,248,248,245,235,232,232,233,234,236,237,237,236,236,235,236,235,236,221,221,219,228,231,235,240,228,226,224,237,246,247,243,244,247,247,249,256,249,248,248,252,264,271,272,277,277,279,280,280,283,286,285,288,280,291,289,293,292,295,296,297,297,297,298,297,299,292,291,290,291,296,300,300,301,298,299,264,260,260,294,295,293,291,270,258,249,252,269,253,246,256,262,264,274,271,268,265,264,263,259,257,254,251,251,200,249,249,236,197,233,235,200,197,196,196,194,192,193,193,194,194,195,195,198,197,197,199,199,203,204,204,202,207,209,211,212,214,217,218,222,220,224,221,227,228,231,230,235,232,238,237,241,242,245,242,246,246,248,247,250,251,251,251,249,252,251,253,254,253,254,255,253,253,253,252,251,253,248,250,248,246,245,244,243,241,240,238,236,235,233,231,230,228,224,223,221,220,218,215,213,210,208,207,206,203,201,202,199,197,193,195,193,192,191,190,189,189,186,190,188,189,187,188,188,188,188,189,190,191,192,194,195,196,198,200,201,204,203,207,206,210,210,216,212,218,217,223,223,226,224,228,229,233,233,234,236,238,238,242,242,244,244,245,246,247,247,249,249,248,249,249,250,249,249,248,250,249,248,246,247,246,246,245,244,244,241,240,238,237,236,235,230,230,227,227,224,222,219,220,216,215,211,209,209,207,205,202,200,199,199,197,194,191,191,190,189,187,187,183,185,185,185,183,184,184,184,185,184,185,188,189,191,191,190,191,193,197,198,197,203,200,203,204,209,210,212,211,214,215,219,218,222,223,226,227,229,229,232,233,237,238,239,239,241,241,243,241,247,245,247,245,246,246,247,247,248,248,248,247,246,247,247,246,245,244,244,242,240,241,238,237,236,235,233,231,228,228,226,224,220,219,217,214,211,210,206,206,204,202,201,199,197,195,195,193,190,189,188,188,185,184,188,185,185,183,184,182,184,181,184,185,185,186,186,188,192,189,191,192,195,195,198,199,202,201,205,209,208,212,211,212,214,216,218,222,223,223,228,227,231,231,233,235,235,238,238,238,241,243,243,246,245,247,245,247,247,247,248,247,247,247,247,246,246,247,247,244,244,242,241,240,240,238,236,236,234,231,229,229,227,225,224,221,219,217,216,212,211,208,209,205,201,198,198,197,195,193,194,190,191,189,190,185,186,183,183,184,183,182,186,185,183,185,182,186,186,188,188,190,188,192,191,195,194,199,198,201,201,204,206,211,209,213,212,216,217,220,219,223,224,227,232,231,233,233,233,238,238,238,239,240,241,242,244,245,246,246,244,246,246,246,250,247,247,247,246,246,245,245,243,241,241,241,239,239,237,236,234,233,231,232,228,226,224,223,219,220,218,215,212,210,208,208,204,201,200,198,197,197,191,189,192,190,189,187,186,184,186,185,184,184,186,182,182,183,184,184,183,185,186,189,189,188,192,192,193,195,196,199,200,202,205,205,208,211,213,214,215,217,221,222,223,224,226,230,230,231,232,235,236,237,242,239,240,241,243,244,247,244,246,247,246,246,248,247,248,247,246,247,244,245,244,245,242,242,242,240,240,238,238,236,235,234,229,229,227,225,222,219,217,216,213,212,210,207,205,205,202,199,198,198,196,194,191,189,187,187,186,185,185,184,184,185,186,184,183,184,183,183,185,185,185,186,185,188,189,193,194,194,197,197,200,200,203,205,207,208,210,211,214,215,219,220,221,224,226,228,228,232,232,234,234,237,235,238,239,242,243,243,244,244,248,246,247,247,247,248,247,250,247,247,247,247,246,246,245,242,242,242,242,241,238,236,235,233,232,228,229,227,225,223,222,221,216,214,214,210,206,208,205,201,199,197,196,195,194,193,192,191,189,187,185,185,184,184,184,182,183,183,184,184,184,184,185,184,188,187,188,189,190,191,194,196,199,200,201,203,205,205,210,210,212,216,215,217,218,221,225,224,227,228,229,232,232,235,238,237,239,240,242,242,243,244,246,245,246,245,246,246,246,248,246,249,245,245,244,245,244,243,242,241,240,239,240,237,236,234,233,232,230,227,226,225,221,220,218,216,214,213,211,207,206,204,202,202,198,196,196,192,191,190,188,187,187,187,185,187,186,184,184,184,182,183,183,182,185,185,185,186,188,189,189,192,194,194,195,196,201,201,202,204,206,209,212,211,216,214,218,220,221,222,226,226,228,229,231,233,236,236,237,239,239,243,241,243,244,244,245,247,245,244,245,246,249,247,247,247,245,247,246,247,244,242,241,241,239,237,236,235,235,234,232,230,230,226,224,223,221,219,216,214,210,208,208,206,203,201,201,197,199,194,193,193,189,190,187,184,185,184,184,183,184,184,183,185,183,185,184,183,184,187,185,187,189,190,192,194,195,198,198,201,203,204,205,209,209,211,212,215,214,217,219,221,224,224,227,228,231,231,233,235,236,238,239,240,243,245,244,247,246,246,247,246,247,246,248,247,246,246,248,246,246,245,244,242,241,241,240,239,240,238,234,234,233,230,228,228,227,225,224,220,218,215,213,212,209,207,206,204,201,199,199,197,195,194,191,189,189,188,186,184,185,183,183,184,182,184,183,182,183,183,182,183,186,189,189,190,191,191,193,194,197,197,200,202,203,208,207,209,210,214,213,215,216,220,222,224,226,227,230,230,233,233,234,235,239,237,242,242,242,244,246,244,245,246,245,246,247,247,248,246,246,247,247,246,245,244,243,242,241,241,239,238,236,236,234,233,230,229,227,224,224,222,218,219,216,214,212,212,208,207,205,203,201,198,197,195,193,190,189,188,187,186,188,188,186,185,184,181,181,183,181,182,183,183,186,186,185,189,188,190,193,193,195,195,199,201,202,203,207,210,210,211,214,216,219,219,220,222,223,225,228,228,231,233,234,235,237,236,239,241,243,243,243,243,247,245,246,247,247,245,245,248,246,247,247,246,245,245,245,244,242,241,241,238,237,238,235,234,232,232,229,227,225,224,221,219,217,218,213,212,209,207,205,204,202,200,199,196,194,193,193,190,187,187,186,185,185,183,184,182,184,184,184,183,183,183,185,184,185,185,188,190,192,190,192,194,197,197,200,201,203,204,209,211,211,215,213,218,218,218,221,224,227,227,229,232,231,233,236,236,238,238,240,241,243,243,246,245,246,246,249,247,247,247,246,247,248,246,246,245,245,244,245,242,242,241,239,238,235,235,235,232,232,228,227,227,224,221,220,219,217,216,212,210,209,205,204,202,198,197,197,194,192,192,189,188,188,185,185,185,186,182,182,182,182,184,181,182,183,185,184,184,187,189,189,191,191,192,196,198,197,198,202,202,207,208,209,213,212,215,216,219,220,221,222,227,227,230,230,232,234,236,237,237,238,241,241,242,243,245,244,246,246,246,248,248,244,248,247,247,247,245,246,245,244,242,241,240,240,239,239,237,237,235,232,231,229,227,225,223,221,218,218,217,214,212,211,207,207,203,202,200,197,197,193,192,190,188,189,188,186,185,184,185,185,182,184,183,182,182,182,184,184,184,186,188,189,189,191,192,194,193,196,199,200,202,205,207,210,210,211,214,215,217,219,221,223,224,227,227,230,231,233,235,235,237,239,240,240,242,244,245,246,245,247,247,246,247,248,246,246,246,247,246,246,246,244,243,243,242,241,240,239,238,235,235,234,233,231,228,229,227,222,222,220,216,216,214,211,208,206,206,205,202,198,197,196,194,193,191,189,188,188,186,185,184,185,184,185,184,185,183,185,183,183,184,184,186,186,189,188,191,191,193,194,198,198,201,202,202,206,209,210,214,213,215,217,218,219,221,224,225,228,229,232,231,235,235,237,238,241,240,241,242,246,245,246,248,249,248,246,246,248,246,247,247,246,246,246,245,245,244,241,243,240,240,239,237,235,234,233,230,229,229,226,223,224,220,217,214,215,211,211,207,207,205,202,198,198,196,194,192,192,189,190,187,187,186,184,184,186,184,183,184,182,185,182,184,184,186,186,187,188,190,192,191,193,195,197,199,200,204,204,210,209,210,213,216,215,217,219,223,223,226,226,228,231,231,232,233,237,239,238,242,241,242,244,244,244,245,246,247,246,248,247,246,245,247,246,246,245,248,247,245,244,242,242,241,239,239,239,236,233,233,230,230,227,227,223,220,216,216,216,213,211,210,210,206,203,202,199,197,195,194,192,190,190,189,188,186,186,185,185,187,182,182,181,182,184,183,184,183,187,186,187,191,189,190,191,193,196,196,200,200,203,204,207,208,210,211,212,215,219,218,222,223,224,227,229,229,232,233,234,235,238,238,240,242,242,243,245,247,246,246,247,247,246,247,249,246,249,247,246,246,248,245,245,245,244,243,241,240,237,236,236,236,234,232,230,228,227,225,223,221,216,217,214,210,211,208,206,204,204,199,198,197,195,193,191,191,189,188,185,184,185,184,185,183,185,182,183,184,183,184,184,186,186,187,189,188,190,193,194,195,199,198,200,203,205,206,211,211,213,215,217,217,218,219,222,225,226,228,231,232,232,235,236,238,240,240,240,242,244,246,245,246,249,248,248,247,247,248,247,248,247,248,247,246,246,245,245,244,242,241,240,238,236,235,234,231,230,229,226,225,223,223,219,217,217,214,212,210,208,205,204,200,198,197,198,193,192,190,190,188,188,186,185,184,184,185,182,184,182,184,183,184,185,183,186,186,188,189,189,191,191,193,196,198,198,198,202,205,207,208,211,214,214,214,217,219,221,221,224,226,228,232,230,233,233,238,237,241,240,241,244,244,245,244,245,247,248,248,249,249,247,247,248,248,247,247,246,245,243,244,243,243,241,241,240,237,236,235,234,232,229,229,225,223,221,219,218,215,214,213,210,209,206,206,202,202,196,194,193,191,189,189,190,189,186,186,185,185,185,184,183,182,182,184,183,184,185,185,185,187,189,189,191,191,195,195,195,199,200,203,205,208,210,212,212,212,215,219,217,223,223,223,227,228,232,232,233,234,237,238,238,240,240,244,243,246,245,247,246,247,246,247,247,247,248,248,246,248,246,247,247,245,244,244,241,240,239,237,236,235,234,231,232,228,227,226,223,220,218,217,214,213,211,209,209,207,205,201,199,197,197,196,193,191,189,189,187,187,186,186,184,184,183,185,182,183,183,183,183,184,187,186,188,188,189,190,191,193,196,197,200,201,204,204,206,209,211,212,214,216,217,219,221,223,224,226,230,231,232,233,235,236,238,239,242,241,242,243,244,244,246,247,247,247,247,249,247,247,247,247,247,246,246,246,245,243,242,243,241,238,238,236,234,234,230,229,228,228,227,223,223,218,217,215,214,211,209,207,207,202,201,198,196,197,194,192,192,190,191,187,185,185,186,184,183,183,181,183,184,184,183,185,186,188,188,188,188,190,191,194,194,196,198,200,201,203,206,207,208,209,214,214,214,217,219,222,222,224,227,230,229,232,233,234,237,239,238,241,242,243,243,244,245,246,246,247,246,247,247,247,247,248,247,246,249,246,246,244,243,243,241,240,239,240,236,237,233,232,229,230,226,224,222,221,218,218,215,216,211,209,209,206,204,201,201,199,196,192,191,189,189,188,188,187,187,185,183,183,183,182,182,181,182,183,183,184,184,186,188,188,190,191,195,195,197,197,201,201,202,205,207,208,210,212,215,216,216,221,222,221,226,227,227,230,232,233,235,236,237,239,241,241,243,243,244,244,245,248,245,247,248,247,247,247,247,248,246,246,248,246,245,244,243,241,240,239,237,237,235,235,232,230,228,228,223,223,222,218,216,215,214,211,209,206,204,202,201,201,197,196,193,191,190,189,190,186,186,184,185,185,183,183,185,183,183,183,183,184,184,185,188,186,188,188,190,190,192,196,198,199,202,203,205,207,208,210,211,215,216,220,219,221,225,224,225,227,229,231,235,234,236,240,240,243,241,243,243,247,244,246,246,247,247,247,245,245,247,247,246,247,246,247,244,244,243,241,241,241,239,237,236,235,234,231,228,230,227,225,225,220,220,217,214,212,210,209,207,205,203,202,199,197,196,193,192,193,189,189,186,185,184,184,183,183,183,182,184,182,182,184,183,184,185,188,186,189,189,190,192,194,195,198,199,201,202,205,209,209,212,212,213,217,217,220,222,223,225,225,229,230,231,233,234,237,237,240,242,242,243,245,246,245,246,246,247,248,248,247,248,247,248,247,247,246,246,245,244,243,242,242,241,240,240,236,236,235,234,230,228,226,226,221,220,219,217,215,214,212,209,207,206,204,201,200,197,196,193,192,190,189,188,187,187,186,185,186,186,185,183,183,182,183,183,184,184,186,188,187,189,190,190,192,195,198,200,200,202,202,206,206,207,209,213,216,216,220,220,223,223,225,228,229,231,233,234,236,237,239,240,242,243,245,246,245,246,246,249,248,247,248,247,248,248,247,247,248,248,247,247,245,244,243,242,241,239,240,238,236,234,233,230,227,226,226,224,220,219,216,215,212,210,208,208,204,203,201,199,198,196,194,193,190,188,188,186,185,186,185,184,184,182,183,184,184,185,184,186,185,186,186,187,189,189,190,193,196,196,197,201,201,203,206,208,211,211,212,214,216,219,221,221,224,226,226,229,230,231,234,235,236,238,240,242,242,244,245,247,247,246,247,247,248,248,248,249,249,247,248,247,248,247,246,246,243,242,241,241,239,237,237,235,231,233,227,227,226,223,221,220,218,217,214,213,210,207,207,205,202,199,197,196,194,192,193,190,189,188,186,186,184,185,184,183,182,182,182,183,183,184,183,184,185,188,189,189,187,190,192,194,196,199,199,203,204,207,210,211,213,214,216,218,220,219,223,224,227,228,230,231,234,235,237,239,239,241,240,244,244,246,246,249,247,248,248,248,247,246,248,248,248,248,247,247,247,246,244,244,243,242,242,241,237,236,234,233,233,230,227,226,225,222,220,219,218,215,213,211,209,206,206,204,200,196,198,194,191,190,189,188,187,186,186,184,184,185,183,182,184,182,181,184,182,186,185,187,188,189,188,190,192,193,194,196,198,199,201,203,206,207,211,212,213,214,217,220,220,222,223,228,229,229,233,235,234,235,237,239,243,241,242,243,245,247,246,248,248,248,249,249,247,249,247,247,247,246,246,246,245,244,244,242,241,239,240,236,236,234,233,231,231,227,224,224,220,219,216,214,212,209,210,206,204,205,201,202,197,196,193,192,190,188,187,186,185,186,185,184,185,185,185,182,184,185,185,183,185,184,186,187,187,190,191,190,192,193,198,197,200,203,205,206,207,209,212,213,215,216,220,221,221,223,225,228,230,232,232,234,237,237,238,241,241,242,243,244,246,246,249,246,247,246,248,247,247,247,247,247,246,246,247,245,246,243,242,241,240,239,237,234,234,232,230,230,228,226,223,221,220,217,218,214,212,210,208,206,201,203,200,197,197,194,191,190,190,189,187,186,186,185,184,184,185,182,182,182,184,185,185,185,187,188,187,187,188,192,193,193,195,196,198,200,203,203,209,209,210,212,214,216,219,221,219,224,224,227,229,230,231,233,235,235,237,238,240,241,244,245,246,245,248,247,248,248,248,248,248,248,248,248,248,249,247,247,246,245,245,242,242,240,239,238,235,234,235,230,228,227,225,223,221,217,218,215,213,212,210,210,206,205,204,200,200,195,194,191,190,188,189,188,188,187,187,186,186,186,184,183,182,182,184,184,185,186,186,187,188,188,190,192,196,195,197,199,202,203,205,207,208,212,214,213,216,218,220,220,224,225,225,228,230,232,232,235,236,238,239,241,243,244,245,244,247,248,247,247,248,248,249,248,248,249,249,248,248,246,247,245,243,243,244,242,241,240,238,238,235,233,231,230,226,225,224,220,218,218,215,214,210,209,208,206,205,202,199,198,199,194,191,192,189,188,186,187,186,184,184,184,183,185,182,184,184,184,184,185,184,187,186,190,190,191,193,193,195,195,198,200,202,202,207,208,209,212,214,216,216,218,222,222,223,226,228,228,230,232,235,238,237,240,241,242,243,244,246,246,246,246,249,248,247,248,246,247,248,247,247,248,247,245,248,246,245,244,243,243,240,238,237,234,232,229,228,227,225,223,223,219,217,215,213,211,209,207,205,203,201,200,198,196,195,191,190,190,188,187,187,186,186,185,185,183,183,182,183,183,183,184,186,185,185,188,188,188,190,190,193,195,199,198,201,204,206,206,208,211,211,215,215,217,219,222,225,224,226,227,230,230,233,235,236,239,239,240,243,243,243,246,247,246,248,248,248,249,249,249,249,248,248,247,247,246,246,246,245,244,242,242,240,239,237,237,236,232,231,229,228,225,223,220,219,216,214,214,212,209,208,207,205,203,199,198,197,193,191,190,191,188,189,187,187,187,186,184,185,182,182,182,182,185,183,185,185,185,189,188,189,193,192,195,197,198,200,201,203,205,207,208,210,212,214,217,218,220,221,222,226,226,230,232,234,232,236,237,238,240,241,243,242,243,246,248,247,249,248,248,248,249,248,248,249,248,248,248,246,247,246,246,244,243,242,242,239,238,236,235,234,231,230,227,225,224,220,220,217,217,212,209,207,206,205,206,201,201,199,197,195,193,192,189,187,189,188,186,186,186,184,184,183,184,184,184,185,182,185,187,187,188,188,191,189,191,193,195,197,198,201,203,206,206,208,211,214,216,216,217,219,221,221,224,227,229,229,230,232,235,236,239,240,239,243,243,243,245,245,247,248,247,247,247,248,248,248,249,247,247,246,247,248,246,244,242,242,244,241,239,236,235,233,234,230,229,226,225,222,221,220,217,214,213,210,209,208,205,204,201,199,197,196,195,193,190,188,190,189,188,187,187,185,185,184,183,183,183,184,184,184,185,186,187,187,188,190,190,191,195,197,197,197,200,202,206,208,207,210,213,213,215,219,218,222,222,223,226,227,230,230,234,233,236,237,238,242,244,243,244,245,248,246,246,246,248,247,246,246,247,247,248,247,246,247,247,246,245,244,243,242,242,239,236,235,234,234,231,229,226,224,225,219,218,218,215,214,214,208,208,206,205,202,201,198,195,194,190,190,188,186,187,187,184,187,184,184,182,181,182,182,182,184,185,184,185,185,186,187,188,189,191,193,194,195,197,200,203,204,206,208,210,213,214,215,219,220,224,223,224,228,227,229,231,232,233,234,240,239,242,243,244,245,246,247,248,248,248,248,249,248,249,248,249,248,248,246,246,246,246,245,244,243,244,240,239,237,236,234,232,230,228,229,227,222,220,221,216,215,213,210,209,208,205,204,202,200,197,195,194,190,190,188,187,187,186,185,186,186,184,184,185,185,184,185,183,183,186,185,186,189,187,190,190,193,194,196,198,199,203,203,204,207,211,213,213,218,217,217,219,221,223,224,226,228,229,233,233,234,236,238,239,241,242,243,244,245,245,246,246,249,246,247,247,247,247,246,246,246,246,246,245,245,246,244,241,242,242,238,237,236,234,232,230,230,226,224,221,220,218,215,216,215,210,209,209,206,203,200,197,198,194,193,191,189,188,187,186,188,184,184,185,184,182,185,184,183,183,182,183,186,186,186,186,189,189,192,191,194,196,197,200,200,203,205,207,208,213,214,215,219,217,219,223,222,225,226,227,231,231,232,234,237,238,239,243,241,243,244,245,245,249,248,247,247,250,247,246,247,246,247,246,246,246,244,244,245,243,244,240,239,237,235,234,234,230,228,229,226,222,220,220,217,215,213,214,210,208,205,203,201,201,196,194,193,190,192,191,188,187,187,186,185,185,184,183,182,182,182,182,185,182,185,185,185,187,189,189,189,193,193,194,195,198,199,201,206,207,208,211,212,214,218,218,220,220,222,223,226,228,228,230,233,233,235,239,239,243,244,242,244,246,246,247,247,250,248,250,248,248,249,249,248,248,247,247,246,244,244,243,241,241,241,239,238,236,234,234,231,228,227,225,223,223,220,216,214,214,211,210,207,205,203,202,200,197,196,192,191,191,190,189,188,187,185,185,184,184,183,183,183,183,183,183,183,184,185,186,187,189,190,191,192,193,195,196,198,200,202,202,206,208,211,211,215,216,218,219,221,223,225,226,229,229,232,233,235,236,238,239,240,241,243,244,246,246,247,247,247,248,248,248,248,248,247,247,247,247,246,246,246,245,244,243,242,241,240,238,236,234,232,231,228,227,225,223,221,219,217,216,214,212,210,208,206,203,201,199,196,195,194,191,191,189,189,188,187,186,185,184,184,183,182,182,182,183,183,184,185,186,186,187,188,189,190,191,193,196,197,198,200,202,204,206,207,210,213,214,216,218,220,221,223,225,227,228,230,231,232,234,236,238,238,241,240,243,244,245,246,247,247,247,248,248,247,248,248,248,248,247,247,246,246,245,245,243,243,242,241,239,237,236,234,232,230,228,226,223,222,221,219,217,215,213,212,209,208,205,203,201,199,196,195,192,190,190,188,189,187,186,185,184,184,183,183,183,183,182,183,183,184,184,185,186,186,188,188,191,192,193,195,197,199,199,203,206,208,208,210,213,215,217,218,220,222,223,225,225,228,231,232,234,235,236,238,240,241,241,243,245,245,246,247,247,247,247,247,248,248,247,248,247,247,247,247,247,245,245,244,242,242,240,238,237,236,234,231,230,228,227,225,222,220,219,217,214,212,210,209,206,205,203,201,199,197,195,193,192,190,188,188,187,185,184,185,184,184,182,183,182,183,183,184,183,184,185,186,188,189,190,190,193,194,196,197,198,200,203,205,207,210,211,213,215,217,218,220,222,222,225,226,229,230,231,233,234,237,240,240,242,243,243,245,246,247,247,247,247,247,248,247,247,248,248,247,247,247,246,246,246,244,243,243,242,240,239,238,235,233,230,230,228,226,225,223,221,218,217,215,213,211,208,207,205,203,201,198,196,195,193,192,190,189,188,187,187,186,185,185,184,183,183,182,182,183,183,184,184,185,186,188,188,188,191,192,194,196,195,199,201,202,204,205,211,211,212,215,216,219,220,222,223,224,227,228,230,232,233,235,236,238,240,243,243,244,245,248,247,247,248,248,247,250,248,247,248,248,247,248,247,246,246,245,244,244,241,242,240,238,236,235,233,232,229,227,225,223,221,219,218,217,214,213,211,210,206,205,203,200,198,196,194,192,190,190,189,188,186,187,185,185,184,184,183,183,184,181,185,183,184,185,187,187,188,189,191,190,191,194,196,198,200,200,205,208,208,209,212,214,216,218,220,222,222,224,226,228,229,231,232,235,237,237,239,241,242,243,245,245,246,247,247,247,248,248,248,248,248,247,247,248,247,247,247,246,245,245,243,243,240,239,240,237,236,233,233,229,228,227,225,222,220,218,216,214,212,210,206,207,205,204,200,198,196,194,192,191,190,189,187,187,185,185,184,184,184,183,182,182,183,183,184,184,185,185,187,187,189,190,191,193,195,198,198,200,202,204,206,208,210,212,214,216,218,219,221,223,225,225,227,229,230,233,234,236,238,240,241,242,243,244,246,246,246,247,247,248,248,248,248,248,247,247,247,247,247,246,245,245,244,243,242,241,239,238,236,233,232,230,228,227,225,223,221,220,218,216,214,213,210,207,205,203,201,199,197,195,194,192,190,190,188,187,187,186,186,184,184,183,183,184,183,183,183,183,184,185,186,187,188,189,190,192,194,195,197,198,201,203,205,207,209,210,212,214,217,219,220,221,224,225,228,228,230,231,233,234,236,238,239,241,242,245,245,246,247,248,248,248,248,248,248,248,248,248,247,247,247,247,246,246,245,244,243,242,241,240,237,236,235,233,230,229,227,224,223,221,218,217,218,213,212,210,208,206,203,201,200,197,194,193,191,190,189,188,188,187,186,185,184,184,184,183,182,183,182,184,183,185,186,187,187,189,189,190,191,193,195,197,199,201,202,205,207,209,211,213,215,217,218,220,221,224,225,227,227,230,232,233,235,236,238,240,241,242,244,245,246,246,247,247,248,248,248,248,248,248,248,247,247,247,247,246,246,245,244,243,242,241,239,237,235,234,233,230,229,227,225,222,221,220,217,216,213,211,209,207,206,204,202,200,197,195,193,192,191,189,188,187,185,185,185,183,184,182,182,183,183,183,183,183,184,185,186,188,188,190,190,192,194,195,197,199,200,203,204,207,209,211,213,216,217,218,220,222,224,224,227,229,230,232,233,235,237,237,240,242,243,244,245,247,247,246,247,248,248,248,248,247,248,247,248,247,248,247,246,246,245,244,243,242,241,240,238,236,233,233,230,228,226,225,223,221,218,217,215,214,212,209,207,207,205,200,198,197,197,193,191,192,189,189,187,187,185,185,184,184,183,183,182,183,183,184,184,185,186,186,188,189,190,192,193,195,196,198,202,202,205,207,208,210,212,214,216,217,220,221,223,225,226,227,229,230,232,235,235,237,238,240,242,243,244,245,246,247,247,247,248,247,248,248,248,247,247,247,247,247,246,245,245,245,242,241,241,240,237,237,234,235,231,229,227,225,222,223,219,216,216,216,213,210,208,206,204,202,199,199,195,193,192,190,189,188,187,187,188,184,184,184,184,183,183,184,183,183,184,184,186,186,188,188,189,189,192,193,196,196,199,201,203,205,207,209,210,213,214,216,217,220,221,223,224,227,228,228,232,235,234,236,238,239,240,242,243,245,248,246,247,247,249,248,248,248,250,248,248,247,247,249,247,246,246,245,243,244,242,240,238,236,235,234,233,231,229,226,225,223,220,219,217,215,213,211,209,207,206,205,201,199,197,195,192,191,190,189,188,188,186,186,184,184,186,183,182,181,182,182,183,184,185,188,186,188,189,190,192,194,194,194,196,201,199,202,204,206,209,211,212,215,217,218,220,222,224,225,228,228,229,232,233,235,237,238,240,242,242,243,244,247,246,247,246,249,247,247,248,248,248,248,249,247,246,248,246,246,247,245,243,241,241,240,238,235,232,230,231,228,226,226,222,221,220,217,216,214,212,211,208,204,205,200,197,195,193,195,191,190,189,188,187,186,185,185,184,183,183,184,184,182,182,183,184,184,184,185,186,190,188,189,191,192,194,196,199,201,201,204,206,208,213,212,214,216,218,221,222,222,224,226,229,230,230,232,233,238,238,239,241,241,244,244,245,245,247,248,246,248,248,248,250,248,248,248,247,247,246,245,245,244,244,243,241,241,239,236,234,234,232,231,228,226,224,222,220,218,217,215,214,211,209,207,205,203,201,198,196,194,194,191,190,189,188,187,186,184,184,184,183,184,183,182,182,182,179,183,185,185,185,188,187,189,191,191,193,194,197,198,200,202,207,209,209,213,214,215,215,217,220,223,223,225,229,228,229,231,233,235,235,238,239,241,241,242,244,245,246,248,250,246,248,248,248,248,248,248,247,247,245,246,246,246,247,243,245,242,239,238,237,235,233,232,231,228,227,223,223,220,218,217,215,213,211,209,207,205,203,201,197,197,195,193,191,190,189,187,187,185,185,184,184,184,183,181,182,183,182,183,183,184,186,185,186,189,188,190,191,193,195,196,198,201,202,204,208,209,211,215,215,217,219,220,221,224,225,227,227,229,231,235,235,239,237,239,240,245,243,243,246,247,246,246,248,250,248,248,248,247,248,246,247,247,245,245,244,244,243,243,242,241,239,240,237,234,231,229,227,226,224,222,220,218,217,215,213,210,209,207,204,202,199,198,196,194,192,191,189,189,188,186,185,185,185,184,183,183,181,182,181,181,183,183,184,185,185,188,187,190,192,191,194,195,199,198,200,204,205,207,208,209,211,213,215,218,220,222,223,225,227,229,229,232,232,235,236,238,239,240,241,243,244,245,245,246,247,248,248,248,247,247,247,250,246,246,246,248,245,247,244,243,243,241,241,239,237,235,233,232,230,229,227,225,222,222,218,215,215,212,210,208,205,204,201,200,198,196,194,193,191,189,190,190,186,186,185,184,184,182,180,182,182,181,181,182,183,183,184,186,188,188,189,192,190,193,195,195,199,201,202,207,207,209,211,212,214,215,218,220,222,225,225,226,229,228,231,234,235,239,238,239,241,243,244,245,248,246,246,247,246,247,247,247,247,247,248,248,246,246,247,246,245,245,244,242,241,238,238,235,233,233,231,229,227,225,225,224,220,217,217,213,214,209,207,205,203,201,199,197,195,194,194,190,188,188,187,188,185,185,184,184,182,182,183,182,182,182,182,182,185,185,186,188,189,188,190,194,193,196,197,198,202,202,205,207,210,212,214,215,216,217,221,222,224,225,227,228,230,231,236,234,237,239,241,242,246,244,247,245,245,247,246,247,250,248,247,248,248,246,249,246,246 };

    int stepsPerRev103 = 103;
    double revolutionHeight = 10.0 / 6.0; // 0.8...mm
    double stepAngleDegrees = 360.0 / stepsPerRev103;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    int maxLayers = (int)measurements.size() / stepsPerRev103;

    int nLayers = maxLayers;
    int layerOffset = 0;

    VTKBodyArray bodySlices;

    for (int rev_i = 0; rev_i < nLayers; rev_i++)
    {
        VecArrayArray layerPerims;
        CarveSlice(rev_i + layerOffset, measurements, layerPerims, revolutionHeight * (rev_i + layerOffset));


        // Make sheet from each perim and sweep into body
        for (VecArray layer : layerPerims)
        {
            int nPos = (int)layer.size();
            PK_VECTOR_t* pkVectors = new PK_VECTOR_t[nPos];
            for (int t = 0; t < nPos; t++)
            {
                pkVectors[t] = layer[t].PkVector();
            }

            PK_BODY_create_sheet_planar_o_t pkSheetOpt;
            PK_BODY_create_sheet_planar_o_m(pkSheetOpt);
            pkSheetOpt.plane.location = pkVectors[0];
            pkSheetOpt.plane.axis = Dir::ZDir().GetVec().PkVector();
            PK_BODY_t       fenceBody = 0;
            PK_ERROR_code_t pkError = PK_BODY_create_sheet_planar(nPos, pkVectors, &pkSheetOpt, &fenceBody);
            //CheckM(PK_ERROR_no_errors == pkError);
            if (PK_ERROR_no_errors == pkError)
            {


                VTKBody vb(fenceBody);
                IntIntPairArray ignore;
                double layerHeight = revolutionHeight * 1.1;
                bool sweptOk = vb.Sweep(Dir::ZDir(), layerHeight, ignore);
                //CheckM(sweptOk);
                if (sweptOk)
                {
                    bodySlices.push_back(vb);
                }

            }
            delete[] pkVectors;
        }

    }
    //DoCreateCylinder(bodySlices[0]);
    VTKBody::TransmitBodies(VTK::Version_25_00, bodySlices, "D:\\slices.x_t");


    PK_BODY_boolean_o_t boolOptions = { 0 };
    PK_BODY_boolean_o_m(boolOptions);
    boolOptions.function = PK_boolean_unite_c;
    boolOptions.merge_imprinted = PK_LOGICAL_false; // PK_LOGICAL_true; we don't want to merge this leaves our tag to disappear
    PKTopolTrackResults tracking;
    PK_boolean_r_t boolRes = { 0 };
    PK_ERROR_t pkError = 0;
    for (int b = 1; b < bodySlices.size(); b++)
    {
        pkError = bodySlices[0].Boolean(bodySlices[b], &boolOptions, &tracking, &boolRes);
        CheckM(PK_ERROR_no_errors == pkError);
        tracking.Clear();
        PK_boolean_r_f(&boolRes);
    }

    bodySlices[0].Transmit(VTK::Version_25_00, "D:\\temp.x_t");
    


    return true;
}
VTK_TEST_FN(TestPKUtils999_SpaceCarveSortingHat001)
{
    // Full scan

    IntArray measurements = { 142,145,153,34,156,153,150,33,34,158,165,175,32,188,186,192,201,210,240,241,247,243,225,207,195,182,183,184,180,180,180,179,180,182,182,181,178,178,175,178,144,171,169,172,155,166,165,166,168,169,168,168,168,167,166,167,166,165,165,163,162,157,156,155,155,154,153,151,150,145,145,142,141,138,136,134,130,128,126,126,124,122,120,118,116,114,112,112,110,108,108,115,150,168,171,163,137,130,128,134,174,173,145,142,146,158,164,162,161,159,159,162,169,182,191,196,202,205,208,210,213,233,245,263,266,262,184,184,179,178,179,325,329,332,332,332,334,334,334,335,336,337,339,340,340,344,325,324,328,333,334,332,333,335,334,331,330,330,328,327,324,322,321,317,318,316,315,311,309,307,306,304,301,299,146,293,293,292,136,283,132,282,280,278,276,270,268,118,116,114,112,109,109,106,107,107,108,115,117,128,127,133,138,176,180,148,144,153,188,179,180,182,186,186,189,193,198,207,209,212,213,215,212,212,220,247,266,181,178,176,176,322,320,323,326,328,327,327,329,329,331,330,334,333,333,334,334,337,341,324,324,326,331,333,334,331,331,332,331,329,327,326,323,322,320,319,318,316,313,310,310,306,305,303,302,298,298,293,293,294,293,287,286,283,280,279,277,276,272,272,271,270,268,267,266,266,267,111,110,109,108,110,112,124,136,148,177,197,135,152,190,189,182,185,191,193,196,199,199,207,212,210,209,208,205,207,211,218,253,181,177,176,178,322,324,324,325,324,327,326,327,328,331,332,332,332,334,335,336,336,336,335,339,328,327,329,332,332,335,332,333,330,331,329,327,325,326,321,319,318,316,314,310,311,308,307,302,303,300,297,297,296,294,291,289,286,284,282,284,280,278,275,272,270,270,270,270,268,266,267,267,266,271,270,113,112,115,117,119,128,207,127,151,154,177,167,176,181,183,186,189,192,194,197,202,203,200,204,206,210,178,180,174,176,325,330,330,331,330,330,333,333,335,336,337,336,337,337,340,338,342,340,340,340,340,338,341,333,335,340,337,339,337,337,337,336,337,335,334,332,330,328,326,320,319,322,319,319,307,307,306,303,302,305,301,301,296,296,291,291,289,286,285,282,278,277,279,276,274,273,271,273,271,268,273,270,270,275,277,116,116,120,122,129,128,140,142,146,155,155,161,166,168,173,177,179,186,194,203,204,205,181,178,178,175,178,323,328,327,330,329,330,333,333,335,335,336,335,338,337,339,339,339,340,339,339,340,341,341,342,341,340,341,341,340,339,339,338,336,338,334,332,331,330,329,327,325,325,322,322,318,320,317,313,308,306,303,301,298,297,293,290,288,287,287,284,282,280,278,277,275,273,271,270,271,270,269,270,270,273,272,274,276,277,124,124,126,128,131,137,140,143,150,154,156,160,165,169,171,177,185,180,178,176,176,175,178,321,319,322,324,325,327,329,331,331,332,334,333,334,334,336,339,340,339,340,342,344,342,343,344,344,345,346,345,345,345,346,345,343,341,342,341,339,339,336,332,330,328,323,321,322,320,317,312,308,306,305,301,299,297,294,291,283,281,281,279,280,278,276,274,270,271,270,268,267,268,267,266,265,266,269,270,273,274,273,275,277,279,285,133,138,137,140,143,147,151,154,157,162,166,167,169,171,173,174,176,176,323,322,321,321,323,324,326,329,330,331,331,334,336,334,334,336,339,340,339,340,341,345,347,348,349,348,350,350,351,350,351,351,351,350,347,347,344,344,343,340,334,331,328,324,321,318,316,315,312,310,308,306,304,298,295,291,288,284,281,282,280,279,275,274,270,271,268,269,268,267,266,265,265,265,267,269,270,271,272,272,274,274,277,280,281,284,287,285,287,148,151,152,154,158,160,163,164,168,171,174,175,178,322,322,324,323,325,324,327,328,330,331,333,334,335,338,337,338,337,340,341,343,347,347,349,350,351,353,354,355,354,355,357,356,354,356,356,355,352,351,348,343,338,337,332,329,325,322,321,318,317,316,314,315,309,301,297,292,290,289,290,285,283,280,280,278,274,274,270,270,269,268,265,265,265,266,268,272,272,273,275,273,273,273,275,278,279,281,282,283,282,283,286,288,293,297,304,311,167,168,172,323,323,324,326,329,328,328,328,327,328,328,330,333,333,334,336,335,337,338,341,340,344,346,347,351,352,353,357,357,357,358,357,359,357,357,360,359,359,359,357,357,353,352,347,345,338,336,334,330,327,325,324,324,324,317,313,311,303,302,300,299,310,307,304,295,289,286,284,283,284,281,281,283,275,270,269,269,269,271,273,276,277,278,277,276,276,275,278,283,281,282,281,281,284,289,309,309,311,313,319,319,321,323,326,325,327,328,329,328,328,328,328,328,329,331,334,335,335,337,336,338,341,343,346,347,350,352,353,356,359,358,361,363,362,364,363,362,362,363,362,362,361,360,357,355,353,350,345,341,340,339,336,333,331,329,326,325,320,316,318,320,320,318,314,308,305,303,296,292,290,286,285,283,281,281,282,283,284,279,274,274,273,276,277,279,278,280,279,278,278,283,284,282,283,287,297,303,304,306,309,313,314,316,317,319,321,322,323,325,326,326,325,324,327,328,327,329,330,332,335,336,337,339,341,343,346,346,350,355,354,359,360,361,363,363,365,365,367,367,366,366,365,364,364,365,362,358,354,351,349,343,341,340,339,338,335,332,333,330,328,328,326,325,322,319,316,313,311,307,304,297,295,290,288,284,283,282,281,282,284,282,282,281,279,279,279,280,281,281,281,282,284,283,287,287,291,291,294,297,300,305,309,310,312,313,316,317,319,321,322,325,328,327,328,329,326,326,327,327,329,331,334,334,337,339,340,341,345,348,350,352,354,356,360,360,362,365,366,369,368,368,368,369,367,366,364,361,359,358,354,354,349,347,345,343,341,341,339,335,336,334,333,332,331,330,328,325,322,319,318,310,308,304,295,292,291,286,283,280,280,279,280,284,283,280,280,280,278,277,276,276,278,280,283,285,287,289,292,293,295,297,301,306,308,311,314,314,318,320,322,324,326,334,334,335,334,335,333,327,327,327,329,330,332,334,335,338,339,340,344,344,346,350,352,354,356,359,360,363,364,364,366,366,367,365,363,364,360,357,354,353,350,351,351,348,347,346,342,341,340,339,338,337,334,334,331,329,329,328,324,323,319,316,313,309,305,299,295,291,286,282,278,279,276,278,281,285,282,279,277,278,276,276,277,280,282,285,287,293,291,298,303,308,311,311,314,316,316,319,322,324,325,326,326,329,329,331,332,334,331,330,329,330,330,330,332,334,336,337,339,339,341,342,343,347,348,352,353,356,356,359,360,361,360,359,361,361,357,355,354,352,352,353,353,352,351,350,350,348,344,343,343,341,340,339,337,336,335,333,331,328,327,323,323,318,317,314,311,306,302,295,291,288,284,279,278,277,276,280,283,285,286,280,279,281,280,282,286,294,296,299,301,302,304,305,306,308,309,311,317,317,322,323,325,324,327,326,328,329,329,328,328,329,328,330,330,331,329,333,336,336,338,340,339,340,340,342,344,346,347,349,352,353,354,356,353,352,354,353,354,354,352,352,352,353,353,353,352,352,350,353,349,348,345,342,343,339,340,340,337,335,334,333,328,327,324,323,322,317,313,311,308,300,294,294,288,283,280,277,276,275,277,281,280,283,285,285,287,289,290,293,296,297,299,299,300,300,301,303,304,310,312,314,318,321,324,324,326,327,329,327,328,327,327,327,329,328,332,330,331,332,333,335,336,338,338,339,340,342,341,342,344,346,346,347,351,351,353,355,352,355,354,355,355,354,354,356,355,355,356,356,357,356,356,357,355,353,351,349,346,345,344,339,338,336,332,331,329,327,323,321,319,316,314,310,304,299,294,291,286,281,278,278,278,278,280,278,279,280,281,284,286,287,288,292,293,293,294,295,297,299,303,305,310,311,317,318,321,324,326,329,329,329,330,330,329,330,331,330,330,330,332,334,338,338,338,340,340,343,341,344,343,343,344,346,347,349,350,350,352,352,354,354,355,356,357,357,357,358,358,359,360,360,360,361,361,362,361,361,359,358,357,354,351,348,344,342,340,337,335,329,328,326,323,319,318,317,311,306,299,296,293,291,286,281,280,280,280,278,277,277,275,278,279,280,282,285,288,288,291,292,295,298,301,305,309,311,315,317,318,322,325,327,329,330,332,332,333,334,333,335,335,336,337,338,340,341,343,346,348,349,351,351,350,351,349,350,350,351,351,352,353,356,358,358,359,359,360,362,363,362,363,365,365,366,365,365,366,367,366,370,365,364,365,362,359,358,354,351,349,346,341,338,335,332,328,326,324,319,316,314,308,301,300,297,293,288,285,283,282,286,281,278,275,273,273,272,276,277,278,286,288,290,294,297,300,303,306,308,312,316,318,321,323,327,328,329,331,334,334,335,338,336,338,339,341,341,343,344,345,347,349,350,351,351,351,354,354,354,354,355,355,357,358,361,361,363,365,364,366,367,368,367,369,369,369,371,370,370,370,371,370,369,368,367,366,366,363,362,360,358,354,352,351,344,339,337,332,333,328,327,323,321,317,314,310,303,299,297,293,290,288,287,286,284,285,278,274,274,273,273,277,282,283,289,293,297,298,302,306,310,312,316,319,321,322,324,327,331,331,334,336,336,340,339,340,341,343,344,344,344,345,348,350,352,352,352,355,355,355,356,357,357,358,361,361,363,366,366,367,370,370,372,372,371,372,373,375,373,373,374,373,374,372,371,371,371,367,368,369,365,363,363,361,358,355,351,347,341,336,333,334,330,331,329,327,322,320,313,308,302,300,298,295,288,287,286,286,288,285,276,273,273,274,278,283,288,292,293,300,303,306,310,312,315,317,318,320,322,325,327,330,332,335,335,336,339,340,341,342,342,344,344,345,349,350,352,354,355,356,360,359,359,361,360,363,363,365,368,368,371,373,371,372,374,375,376,375,376,376,376,376,376,378,378,383,371,371,372,376,367,367,365,364,361,359,359,358,354,349,345,341,337,335,335,333,332,332,334,324,320,311,307,304,302,298,297,294,292,292,290,291,293,281,277,276,278,281,283,288,293,298,302,306,306,310,311,313,317,318,319,326,328,329,331,333,331,336,338,338,341,342,341,344,344,345,351,353,356,358,358,361,364,365,365,366,368,368,370,373,372,373,374,375,377,377,377,380,380,380,381,381,381,381,381,377,378,382,375,363,365,367,368,369,367,370,362,361,358,358,354,352,349,344,342,337,336,337,337,338,337,337,328,322,317,310,306,305,302,301,300,300,294,294,294,297,299,280,278,279,281,287,290,296,300,304,305,307,310,312,317,318,319,321,325,326,331,334,336,336,338,339,340,341,342,343,347,347,346,353,354,357,361,363,365,367,368,369,371,371,374,375,375,377,379,382,381,382,383,383,385,386,387,387,390,389,392,392,396,399,403,409,413,365,367,368,367,371,373,376,362,361,358,356,354,351,348,346,343,341,341,340,341,341,340,342,332,322,313,311,309,308,305,303,302,302,299,300,303,304,313,283,284,284,290,296,299,302,304,312,316,321,320,325,323,325,325,327,331,333,338,339,338,339,340,340,341,342,345,346,349,351,354,357,360,363,366,368,369,371,374,374,377,377,379,381,381,382,385,387,387,388,390,391,392,395,398,401,407,396,398,399,399,402,406,408,490,490,490,490,372,375,371,373,377,378,476,392,394,396,398,390,391,387,384,374,362,358,355,331,331,317,317,313,313,310,312,313,312,311,310,311,312,313,292,291,292,295,298,304,308,311,314,318,319,323,325,329,330,330,332,335,334,335,335,336,339,338,341,340,341,344,347,351,352,356,359,363,364,367,369,372,373,377,378,379,381,383,386,385,388,391,392,392,394,396,395,399,398,401,403,403,406,408,409,411,416,423,426,438,490,422,490,490,478,479,481,482,473,388,390,394,468,386,387,388,461,456,453,377,355,342,336,333,328,322,316,315,313,314,314,315,317,319,317,315,313,301,295,293,295,298,300,307,310,313,315,318,321,323,325,326,326,327,329,331,331,332,335,335,337,339,341,343,345,348,350,353,355,358,362,366,367,371,373,376,377,379,382,382,385,386,388,389,390,392,395,395,396,398,400,401,402,404,405,408,409,412,412,416,419,417,490,490,490,490,490,490,490,480,490,490,472,470,466,465,461,456,453,451,449,449,448,356,349,344,335,330,326,317,315,315,316,316,317,319,318,316,314,317,287,288,286,289,289,294,301,306,308,310,312,316,317,317,318,321,322,323,325,330,331,333,334,338,340,343,344,346,348,351,353,357,361,363,366,369,371,373,375,379,381,382,384,386,386,388,390,392,393,395,396,397,398,399,402,403,405,408,410,410,412,415,417,420,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,462,453,451,450,451,450,451,490,490,490,490,490,490,421,317,317,316,316,316,315,314,314,289,285,286,285,287,288,290,289,290,292,295,300,302,305,308,311,314,315,320,324,325,328,331,333,336,339,342,344,347,349,352,355,360,370,372,372,373,374,375,377,380,381,383,385,386,388,390,390,394,394,397,397,399,400,402,403,403,406,409,410,413,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,411,410,319,318,315,313,313,313,287,285,284,284,287,291,291,290,288,289,288,288,290,292,296,301,305,309,314,319,322,327,330,332,336,340,342,345,348,352,354,359,365,367,373,376,381,384,386,392,396,398,400,404,406,409,412,416,419,421,424,427,432,433,437,442,446,451,456,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,407,407,408,316,313,311,289,287,287,286,287,289,292,291,291,290,289,288,290,289,290,291,295,302,303,310,316,321,325,329,334,337,341,343,347,349,354,356,362,365,369,373,377,381,383,388,390,394,395,398,402,404,408,410,414,415,419,421,424,425,429,434,434,435,441,451,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,407,409,318,313,306,297,291,289,289,290,296,294,291,290,291,284,290,292,294,297,303,307,314,321,324,326,329,333,333,335,338,343,345,348,352,355,359,362,365,371,375,378,383,386,388,389,393,397,400,401,405,407,410,412,415,418,420,424,426,428,429,433,436,440,447,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,404,407,318,317,316,332,316,308,304,300,297,294,291,290,289,290,291,292,295,299,304,312,317,320,326,330,334,337,340,344,345,350,352,354,358,361,366,372,375,377,381,384,386,389,393,397,396,400,404,406,408,411,414,417,419,421,422,424,428,430,433,438,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,418,419,421,326,311,307,305,301,297,294,290,290,289,292,291,293,300,303,309,315,322,325,330,333,336,343,344,348,352,354,357,360,366,372,375,380,382,384,387,390,392,396,398,400,402,404,407,410,413,413,417,419,422,423,428,446,448,452,455,460,465,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,423,422,422,421,322,318,315,314,302,301,296,291,292,291,295,299,306,306,311,316,320,324,328,333,334,339,342,346,349,352,355,360,364,367,372,376,379,383,388,391,395,397,401,402,406,409,411,415,417,420,423,426,430,435,438,438,441,444,447,449,455,466,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,436,423,421,419,420,322,319,314,310,307,304,297,294,293,298,300,304,307,310,313,316,320,324,327,332,335,338,343,346,351,352,358,363,367,368,371,376,380,383,386,391,395,396,401,404,406,407,412,414,418,419,422,425,428,432,435,437,440,442,445,451,456,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,419,417,417,321,316,312,309,308,300,298,298,298,300,301,301,303,306,309,312,319,322,327,330,334,337,343,346,350,355,358,361,366,369,373,377,381,384,386,390,393,398,399,401,406,410,411,414,417,419,423,426,429,433,434,436,438,443,445,449,455,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,423,414,414,414,320,315,308,304,300,301,298,296,295,296,296,298,300,304,307,312,317,321,328,331,338,342,347,349,353,358,363,366,370,373,377,381,384,388,392,393,398,402,404,406,410,412,416,417,420,423,427,430,432,435,437,441,445,449,453,465,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,417,411,412,414,318,315,306,302,300,298,295,295,294,296,298,300,303,305,311,315,320,325,331,335,340,346,351,354,359,364,368,373,376,380,384,386,390,394,397,400,404,407,409,412,416,418,420,424,426,429,432,434,439,441,444,447,451,456,468,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,414,411,411,323,319,310,305,301,298,296,297,302,301,302,305,308,307,311,316,321,324,330,335,342,345,351,355,360,363,369,374,379,384,386,390,392,395,399,401,406,409,413,417,417,421,423,427,427,432,436,438,441,443,447,449,453,459,473,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,410,407,405,317,309,305,301,302,306,306,307,307,311,309,313,315,315,317,321,327,329,338,342,349,352,359,363,368,372,376,382,385,390,393,396,400,402,406,409,412,416,417,421,424,426,429,433,434,439,442,445,446,452,455,461,469,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,411,405,409,413,319,311,314,309,307,307,306,306,307,307,312,317,318,322,324,329,332,338,344,350,355,361,369,374,380,382,388,390,394,399,402,405,409,412,415,418,421,423,427,429,431,436,437,440,442,445,450,454,459,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,410,413,412,319,316,311,308,304,301,301,304,302,304,309,313,319,325,328,340,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,450,446,446,448,451,453,457,460,464,467,471,475,480,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,409,409,409,313,310,306,301,300,300,299,299,303,306,309,315,320,334,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,452,452,452,454,457,459,461,464,468,473,476,482,488,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,404,405,405,309,303,304,300,299,299,301,301,304,308,315,321,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,456,457,457,459,461,466,467,472,475,478,483,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,405,400,403,307,304,303,304,301,303,305,308,306,309,314,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,466,466,468,471,475,478,483,489,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,403,403,490,387,307,305,301,300,302,303,308,311,313,317,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,379,382,300,294,295,298,300,309,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,393,374,379,295,287,288,292,303,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,392,367,370,376,280,282,288,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,361,365,278,276,280,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,352,360,268,270,490,490,490,490,490,490,490,490 };

    for (size_t s = 0; s < measurements.size(); s++)
    {
        if (measurements[s] < 50)
        {
            measurements[s] = measurements[s - 1];
        }
    }


    int stepsPerRev103 = 103;
    double revolutionHeight = 10.0 / 6.0; // 0.8...mm
    double stepAngleDegrees = 360.0 / stepsPerRev103;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    int maxLayers = (int)measurements.size() / stepsPerRev103;

    int nLayers = maxLayers;
    int layerOffset = 0;

    VTKBodyArray bodySlices;
    nLayers = 1;
    for (int rev_i = 0; rev_i < nLayers; rev_i++)
    {
        VecArrayArray layerPerims;
        CarveSlice(rev_i + layerOffset, measurements, layerPerims, revolutionHeight * (rev_i + layerOffset));


        // Make sheet from each perim and sweep into body
        for (VecArray layer : layerPerims)
        {
            int nPos = (int)layer.size();
            PK_VECTOR_t* pkVectors = new PK_VECTOR_t[nPos];
            for (int t = 0; t < nPos; t++)
            {
                pkVectors[t] = layer[t].PkVector();
            }

            PK_BODY_create_sheet_planar_o_t pkSheetOpt;
            PK_BODY_create_sheet_planar_o_m(pkSheetOpt);
            pkSheetOpt.plane.location = pkVectors[0];
            pkSheetOpt.plane.axis = Dir::ZDir().GetVec().PkVector();
            PK_BODY_t       fenceBody = 0;
            PK_ERROR_code_t pkError = PK_BODY_create_sheet_planar(nPos, pkVectors, &pkSheetOpt, &fenceBody);
            //CheckM(PK_ERROR_no_errors == pkError);
            DEB_assert(PK_ERROR_no_errors == pkError);
            if (PK_ERROR_no_errors == pkError)
            {


                VTKBody vb(fenceBody);
                IntIntPairArray ignore;
                double layerHeight = revolutionHeight * 1.1;
                bool sweptOk = vb.Sweep(Dir::ZDir(), layerHeight, ignore);
                //CheckM(sweptOk);
                if (sweptOk)
                {
                    bodySlices.push_back(vb);
                }

            }
            delete[] pkVectors;
        }

    }
    //DoCreateCylinder(bodySlices[0]);
    VTKBody::TransmitBodies(VTK::Version_25_00, bodySlices, "D:\\hat_slices.x_t");


    PK_BODY_boolean_o_t boolOptions = { 0 };
    PK_BODY_boolean_o_m(boolOptions);
    boolOptions.function = PK_boolean_unite_c;
    boolOptions.merge_imprinted = PK_LOGICAL_false; // PK_LOGICAL_true; we don't want to merge this leaves our tag to disappear
    PKTopolTrackResults tracking;
    PK_boolean_r_t boolRes = { 0 };
    PK_ERROR_t pkError = 0;
    for (int b = 1; b < bodySlices.size(); b++)
    {
        pkError = bodySlices[0].Boolean(bodySlices[b], &boolOptions, &tracking, &boolRes);
        CheckM(PK_ERROR_no_errors == pkError);
        tracking.Clear();
        PK_boolean_r_f(&boolRes);
    }

    bodySlices[0].Transmit(VTK::Version_25_00, "D:\\hat_bool.x_t");



    return true;
}



VTK_TEST_FN(TestPKUtils999_SpaceCarveShortWall001)
{
    
    IntArray measurements = { 377, 370, 35, 377, 55, 35, 35, 389, 395, 395, 34, 410, 411, 409, 411, 416, 417, 422, 425, 36, 432, 443, 443, 35, 450, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 408, 411, 412, 320, 320, 320, 317, 320, 318, 490, 406, 393, 394, 390, 390, 391, 393, 394, 399, 403, 404, 409, 409, 414, 415, 421, 418, 427, 429, 435, 435, 438, 442, 445, 449, 455, 461, 469, 471, 471, 478, 476, 483, 483, 490, 490, 490, 490, 490, 490, 490, 411, 410, 414, 319, 321, 320, 320, 321, 323, 490, 490, 450, 402, 388, 372, 370, 369, 372, 374, 378, 383, 380, 390, 388, 396, 395, 401, 403, 405, 412, 409, 416, 418, 421, 422, 427, 428, 435, 437, 443, 450, 475, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 490, 410, 408, 321, 317, 320, 318, 317, 316, 317, 490, 415, 398, 396, 390, 388, 388, 393, 395, 400, 402, 403, 408, 410, 412, 417, 420, 422, 425, 426, 433, 433, 441, 440, 445, 449, 453, 458, 464, 471, 474, 477, 479, 481, 482, 489, 490, 490, 490, 490, 490, 490, 490, 408, 412, 319, 320, 320, 321, 320, 321, 317, 439, 395 };
    double stepstoCOR = 416.87;
    double factor = -6.202;

    int stepsPerRev103 = 103;
    double revolutionHeight = 10.0 / 6.0; // 0.8...mm
    double distanceToCOR = 67.21; //# mm
    
    double xHome = 0;
    double yHome = 0;
    double stepAngleDegrees = 360.0 / stepsPerRev103;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    int maxLayers = (int)measurements.size() / stepsPerRev103;

    int rev_i = 0;
    int nLayers = maxLayers;
    int layerOffset = 0;

    VTKBodyArray bodySlices;

    for (int rev_i = 0; rev_i < nLayers; rev_i++)
    {
        VecArrayArray layerPerims;
        CarveSlice(rev_i + layerOffset, measurements, layerPerims, revolutionHeight * (rev_i + layerOffset));


        // Make sheet from each perim and sweep into body
        for (VecArray layer : layerPerims)
        {
            int nPos = (int)layer.size();
            PK_VECTOR_t* pkVectors = new PK_VECTOR_t[nPos];
            for (int t = 0; t < nPos; t++)
            {
                pkVectors[t] = layer[t].PkVector();
            }

            PK_BODY_create_sheet_planar_o_t pkSheetOpt;
            PK_BODY_create_sheet_planar_o_m(pkSheetOpt);
            pkSheetOpt.plane.location = pkVectors[0];
            pkSheetOpt.plane.axis = Dir::ZDir().GetVec().PkVector();
            PK_BODY_t       fenceBody = 0;
            PK_ERROR_code_t pkError = PK_BODY_create_sheet_planar(nPos, pkVectors, &pkSheetOpt, &fenceBody);
            //CheckM(PK_ERROR_no_errors == pkError);
            if (PK_ERROR_no_errors == pkError)
            {


                VTKBody vb(fenceBody);
                IntIntPairArray ignore;
                double layerHeight = revolutionHeight * 1.1;
                bool sweptOk = vb.Sweep(Dir::ZDir(), layerHeight, ignore);
                //CheckM(sweptOk);
                if (sweptOk)
                {
                    bodySlices.push_back(vb);
                }

            }
            delete[] pkVectors;
        }

    }

    VTKBody::TransmitBodies(VTK::Version_25_00, bodySlices, "D:\\slices.x_t");


    PK_BODY_boolean_o_t boolOptions = { 0 };
    PK_BODY_boolean_o_m(boolOptions);
    boolOptions.function = PK_boolean_unite_c;
    boolOptions.merge_imprinted = PK_LOGICAL_false; // PK_LOGICAL_true; we don't want to merge this leaves our tag to disappear
    PKTopolTrackResults tracking;
    PK_boolean_r_t boolRes = { 0 };
    PK_ERROR_t pkError = 0;
    for (int b = 1; b < bodySlices.size(); b++)
    {
        pkError = bodySlices[0].Boolean(bodySlices[b], &boolOptions, &tracking, &boolRes);
        CheckM(PK_ERROR_no_errors == pkError);
        tracking.Clear();
        PK_boolean_r_f(&boolRes);
    }

    bodySlices[0].Transmit(VTK::Version_25_00, "D:\\temp.x_t");


    return true;
}

VTK_TEST_FN(TestPKUtils999_SpaceCarve001)
{
    //IntArray measurements = { 292,286,284,277,275,268,265,259,257,254,249,247,244,243,240,240,242,241,242,243,248,248,253,257,262,265,268,273,276,281,285,290,296,297,302,303,308,307,310,311,313,310,312,311,311,309,307,304,304,299,295,293,286,285,278,276,267,267,260,256,252,251,247,247,243,241,242,241,241,245,245,249,249,255,257,261,264,270,275,277,283,288,290,294,298,303,304,307,308,311,311,312,313,313,312,310,310,307,304,302,301,297,291,288,283,279,274,271,266,262,257,254,249,248,244,245,243,242,241,243,243,245,248,252,255,257,261,267,269,275,277,284,287,291,295,299,301,304,307,310,310,312,311,313,311,313,312,310,306,305,303,299,294,292,287,283,277,275,268,266,261,258,254,249,248,245,244,243,240,243,242,244,245,249,250,255,259,263,267,270,275,279,283,288,291,296,297,302,304,307,308,311,312,313,312,313,312,311,309,308,305,301,299,295,291,286,282,279,275,269,264,262,256,252,250,248,246,244,242,242,241,242,244,247,248,252,254,259,261,267,271,276,278,284,287,291,293,299,303,305,306,308,311,310,311,313,313,312,310,308,307,304,302,300,295 };
    IntArray measurements = { 206,203,201,270,202,182,153,237,259,445,179,183,191,201,201,203,204,205,210,212,218,220,223,223,223,223,225,223,225,223,223,220,222,220,222,213,224,280,224,226,226,225,226,226,223,218,217,215,213,208,352,355,346,345,348,346,310,182,233,347,346,345,348,349,349,352,356,354,354,360,365,221,223,226,227,227,226,229,227,228,229,226,226,224,380,385,385,227,228,333,335,338,344,355,365,351,259,335,348,359,350,348,350,347,346,343,341,184,173,248,461,183,346,344,347,349,352,351,355,215,359,358,223,230,225,228,375,375,374,376,378,379,378,381,382,383,381,224,227,362,352,352,355,358,363,370,370,353,347,351,352,212,352,350,346,344,346,344,344,346,332,345,343,344,345,347,345,348,351,355,355,217,221,224,224,224,372,374,372,375,377,378,377,378,379,379,380,383,229,371,372,363,364,366,357,348,327,379,325,343,350,355,219,364,352,347,342,343,341,343,345,338,342,342,342,344,345,349,350,354,351,354,358,362,221,370,374,373,372,372,375,376,378,379,380,381,380,381,387,229,318,330,331,379,379,383,380,381,380,378,376,371,367,362,358,356,354,352,349,343,342,345,341,345,344,344,343,345,348,350,351,356,355,362,364,366,369,371,374,376,377,382,381,383,382,383,384,385,386,384,382,384,382,383,383,235,236,234,233,378,375,377,372,367,362,356,354,351,349,348,349,347,346,343,343,344,345,347,349,351,352,353,357,360,364,366,373,375,376,378,381,381,382,383,383,382,380,382,381,384,384,385,383,382,379,379,378,381,380,381,374,372,370,367,364,358,357,355,350,348,348,347,348,348,349,350,350,351,354,350,351,354,355,358,368,370,373,375,377,380,381,380,381,383,380,381,380,382,380,380,381,385,382,383,383,384,383,382,383,374,376,377,376,373,370,367,362,357,357,353,352,350,350,353,354,351,351,352,354,354,356,359,361,365,368,371,373,374,375,376,378,381,381,381,379,378,377,376,377,379,381,383,382,379,380,380,382,383,379,372,372,374,381,383,386,383,379,364,359,357,352,354,355,358,358,357,358,359,359,358,357,358,359,360,362,363,365,368,369,368,367,369,372,373,373,375,376,376,377,377,382,385,388,388,388,380,383,383,377,374,379,389,392,391,389,387,383,381,363,361,359,355,356,359,361,363,359,364,364,361,358,356,355,357,358,365,363,359,360,364,368,373,381,381,381,381,386,387,388,383,384,386,389,390,390,390,387,386,387,380,387,391,392,391,391,389,384,376,371,362,360,357,358,360,364,365,366,366,367,367,364,361,359,359,360,361,369,367,366,365,368,374,383,382,382,382,383,383,388,392,393,385,388,390,393,388,389,391,388,390,387,391,390,390,388,387,383,380,376,374,360,359,359,360,364,368,370,370,370,369,369,366,363,365,366,368,369,368,369,381,384,385,384,382,382,383,387,386,392,392,393,393,396,395,395,394,392,392,393,391,392,391,393,395,399,398,396,394,392,388,385,361,361,360,362,366,369,372,373,372,372,370,366,365,367,368,371,371,378,382,383,386,386,386,384,385,385,385,383,387,390,392,393,392,391,389,387,389,391,391,393,397,405,403,403,403,401,397,394,391,387,383,363,362,361,363,367,370,372,372,372,370,367,368,367,368,370,373,378,381,384,386,387,387,387,386,383,383,383,382,383,389,393,393,389,388,388,390,391,398,404,407,406,406,402,402,401,398,394,393,387,376,368,360,360,360,363,365,372,373,373,369,367,368,368,369,370,376,376,380,384,386,388,388,388,388,386,385,384,383,384,382,387,391,397,398,401,399,404,402,405,405,405,403,399,398,399,398,397,396,392,385,374,368,363,362,358,358,361,366,369,367,365,366,367,369,370,371,374,380,380,384,386,388,389,392,390,388,387,386,383,381,381,385,389,394,396,398,399,399,401,403,403,406,396,398,393,396,395,395,393,389,385,370,367,362,360,356,357,361,363,365,366,365,367,371,371,375,375,379,386,386,391,391,394,395,394,394,392,391,391,388,388,384,384,389,390,392,393,395,394,395,395,394,392,393,395,396,396,397,395,395,392,387,381,375,370,370,369,357,359,361,363,364,365,368,372,373,375,377,380,383,385,388,389,391,394,395,395,394,392,390,388,387,384,387,390,388,387,387,388,390,391,391,392,395,397,398,399,399,398,397,396,392,388,380,377,374,374,370,366,362,364,365,367,370,374,379,380,381,382,382,385,388,390,390,392,395,396,395,395,394,392,390,390,388,389,391,391,387,384,382,383,389,390,394,399,400,400,401,401,399,399,398,394,391,385,379,376,375,371,369,369,371,372,372,377,380,382,384,386,387,389,390,391,392,394,397,398,399,399,401,396,396,395,395,394,395,393,396,397,385,382,383,387,391,396,399,401,402,403,402,399,399,398,395,384,379,374,375,371,370,369,372,377,377,379,383,385,388,390,393,393,395,397,398,399,400,402,403,404,404,404,405,402,400,402,399,398,393,395,399,405,382,382,387,391,395,399,401,401,400,398,398,397,395,392,388,383,381,382,383,380,378,378,378,380,381,384,385,386,389,392,395,396,396,398,398,399,402,405,407,408,410,408,401,402,401,399,398,395,400,400,403,385,386,390,394,399,402,401,404,404,400,402,399,397,393,387,385,385,383,384,381,380,381,382,384,386,387,389,391,394,395,399,400,401,402,402,405,407,412,416,416,412,410,405,404,405,406,406,407,403,407,406,422,390,392,396,399,402,405,411,411,412,402,403,405,400,387,386,387,389,388,388,387,387,387,387,386,385,388,389,392,395,397,399,399,400,402,405,409,414,414,412,412,410,407,406,407,409,410,412,406,413,416,426,393,395,402,402,410,412,412,413,412,411,405,405,408,386,386,387,391,390,391,392,394,395,396,402,387,380,388,392,394,397,398,400,402,404,409,415,421,420,417,415,413,411,411,415,413,421,421,422,424,425,425,425,402,404,408,410,411,412,412,412,413,409,399,390,388,386,390,392,393,394,394,397,400,400,405,412,411,392,396,402,408,423,425,426,428,428,428,426,425,422,422,420,422,415,419,421,422,426,429,430,429,429,433,404,400,404,407,409,410,410,408,404,396,393,391,387,387,390,393,394,395,397,400,401,407,409,412,414,419,417,419,421,424,424,426,427,427,428,428,427,426,426,425,423,422,419,421,425,428,431,430,430,431,394,393,392,395,403,404,404,403,398,394,393,392,391,390,390,394,395,395,396,398,401,404,408,411,414,418,425,420,424,420,427,426,426,427,427,430,429,429,427,428,426,427,423,421,423,426,428,428,429,431,395,393,395,392,387,385,386,389,389,390,393,393,394,395,394,398,404,408,416,400,400,401,406,408,413,418,421,425,432,435,432,433,433,432,431,432,434,434,437,438,437,435,434,490,425,425,426,426,427,430,435,393,397,395,392,389,383,381,382,387,390,394,396,398,400,399,403,407,414,419,423,426,432,408,412,415,420,424,428,431,438,442,447,456,473,475,478,480,483,487,490,490,490,490,490,490,490,427,426,427,432,395,397,399,395,391,389,390,394,400,405,408,407,401,402,403,404,407,413,417,420,425,428,431,434,438,441,444,445,449,449,456,462,466,469,467,474,476,479,486,490,490,490,490,490,490,490,490,490,430,429,398,413,407,403,394,390,389,391,394,399,405,408,411,412,411,410,411,412,415,418,423,426,430,434,438,442,445,448,452,455,461,465,449,452,455,452,454,456,457,458,461,467,490,490,490,490,490,425,427,429,422,418,413,410,403,395,393,393,396,400,405,410,412,414,415,416,416,419,421,424,421,424,427,431,435,438,441,445,449,452,454,457,461,463,466,469,472,476,485,490,490,490,490,490,490,490,490,490,490,490,490,434,439,420,413,406,393,394,397,396,401,408,410,412,413,414,416,417,422,426,429,429,428,432,433,437,441,444,448,451,455,457,460,463,466,468,471,474,479,487,490,490,490,490,490,490,490,490,490,490,490,490,490,435,432,427,412,395,396,403,406,407,409,411,412,414,415,415,419,421,425,430,435,438,438,437,440,445,447,451,454,457,460,463,466,468,471,474,477,490,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,424,416,413,400,400,400,397,396,401,404,406,408,410,414,418,419,427,429,436,441,441,444,448,452,457,460,464,467,471,474,464,467,469,472,478,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,421,423,408,402,401,396,393,394,396,400,403,406,408,412,415,419,423,429,432,436,441,447,451,455,459,464,467,472,473,477,480,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,423,408,406,399,392,392,393,397,401,406,409,412,415,418,422,425,429,434,438,443,448,452,456,461,465,469,473,475,480,485,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,413,401,400,400,400,397,394,396,399,404,407,412,416,421,425,430,435,440,445,452,456,461,466,471,474,476,481,485,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,415,407,405,401,399,399,396,400,401,403,408,413,419,423,428,431,438,443,450,456,461,468,472,473,474,471,475,479,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,426,423,416,416,411,410,408,411,413,412,417,419,424,428,432,436,439,443,443,446,449,454,457,458,463,467,473,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,425,420,414,404,400,403,404,408,409,414,417,422,441,444,452,456,463,469,474,480,484,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,430,420,409,397,397,398,402,404,410,416,421,426,434,440,446,454,463,468,474,479,482,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,399,384,384,387,392,397,403,414,423,433,440,448,456,462,469,477,481,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,395,392,388,389,396,398,406,414,425,435,442,451,459,466,474,482,487,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,400,401,403,403,407,412,417,426,433,442,448,455,462,468,477,483,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,394,394,399,405,416,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,484,382,386,394,403,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,384,386,391,397,410,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,471,370,372,378,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,457,362,366,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,459,359,365,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,349,367,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490 };
    int stepstoCOR = 478;
    int stepsPerRev51 = 51;
    double revolutionHeight = 10.0 / 6.0; // 0.8...mm
    double distanceToCOR = 87.3; //# mm
    double factor = 6.17669;

    double xHome = 0;
    double yHome = 0;
    double stepAngleDegrees = 360.0 / stepsPerRev51;
    double stepAngleRads = DegreesToRadians * stepAngleDegrees;

    int maxLayers = (int)measurements.size() / stepsPerRev51;

    int rev_i = 0;
    int nLayers = maxLayers;
    int layerOffset =0;

    VTKBodyArray bodySlices;

    for (int rev_i = 0; rev_i < nLayers; rev_i++)
    {
        VecArrayArray layerPerims;
        CarveSlice(rev_i+ layerOffset,measurements, layerPerims, revolutionHeight* (rev_i+ layerOffset));


        // Make sheet from each perim and sweep into body
        for (VecArray layer : layerPerims)
        {
            int nPos = (int)layer.size();
            PK_VECTOR_t* pkVectors = new PK_VECTOR_t[nPos];
            for (int t = 0; t < nPos; t++)
            {
                pkVectors[t] = layer[t].PkVector();
            }

            PK_BODY_create_sheet_planar_o_t pkSheetOpt;
            PK_BODY_create_sheet_planar_o_m(pkSheetOpt);
            pkSheetOpt.plane.location = pkVectors[0];
            pkSheetOpt.plane.axis = Dir::ZDir().GetVec().PkVector();
            PK_BODY_t       fenceBody = 0;
            PK_ERROR_code_t pkError = PK_BODY_create_sheet_planar(nPos, pkVectors, &pkSheetOpt, &fenceBody);
            //CheckM(PK_ERROR_no_errors == pkError);
            if (PK_ERROR_no_errors == pkError)
            {


                VTKBody vb(fenceBody);
                IntIntPairArray ignore;
                double layerHeight = revolutionHeight * 1.1;
                bool sweptOk = vb.Sweep(Dir::ZDir(), layerHeight, ignore);
                //CheckM(sweptOk);
                if (sweptOk)
                {
                 bodySlices.push_back(vb);
                }
               
            }
            delete[] pkVectors;
        }

    }

    VTKBody::TransmitBodies(VTK::Version_25_00, bodySlices, "D:\\slices.x_t");


    PK_BODY_boolean_o_t boolOptions = { 0 };
    PK_BODY_boolean_o_m(boolOptions);
    boolOptions.function = PK_boolean_unite_c;
    boolOptions.merge_imprinted = PK_LOGICAL_false; // PK_LOGICAL_true; we don't want to merge this leaves our tag to disappear
    PKTopolTrackResults tracking;
    PK_boolean_r_t boolRes = { 0 };
    PK_ERROR_t pkError = 0;
    for (int b = 1; b < bodySlices.size(); b++)
    {
        pkError = bodySlices[0].Boolean(bodySlices[b], &boolOptions, &tracking, &boolRes);
        CheckM(PK_ERROR_no_errors == pkError);
        tracking.Clear();
        PK_boolean_r_f(&boolRes);
    }

    bodySlices[0].Transmit(VTK::Version_25_00, "D:\\temp.x_t");


    return true;
}

bool TestPKUtils()
{
    TestM(TestPKUtils001);
    TestM(TestPKUtils002);
    TestM(TestPKUtils003_blending);
    TestM(TestPKUtils004_EntityGetBody);
    TestM(TestPKUtils005_TopolGetGeomClass);
    TestM(TestPKUtils006);
    TestM(TestPKUtils007_GeomCompare);
    TestFailM(TestPKUtils008_GeomIsCoincident);
    TestM(TestPKUtils009);
    TestM(TestPKUtils010);
    TestM(TestPKUtils011);
    TestM(TestPKUtils012);
    TestM(TestPKUtils013);
    TestM(TestPKUtils014);
    TestM(TestPKUtils015);
    TestM(TestPKUtils016);
    TestM(TestPKUtils017a);
    TestM(TestPKUtils017b);
    TestM(TestPKUtils017c);
    TestM(TestPKUtils017d);
    TestM(TestPKUtils017e);
    TestM(TestPKUtils017f);
    TestM(TestPKUtils018);
    TestM(TestPKUtils019);

    TestM(TestPKUtils020);
    TestM(TestPKUtils021);
    TestM(TestPKUtils022);

    TestM(TestPKUtils023_EdgeGrouping01);

    QuarantinedTestFailM(TestPKUtils024_EdgeDeleteFromWireHealAuto_PR6786955_Case01);
    TestM(TestPKUtils024_EdgeDeleteFromWireHealAuto_PR6786955_Case02);

    TestM(TestPKUtils025_CombinePKTopolTrackResults);

    TestM(TestPKUtils026_AreWireBodyEdgesConnected);

    TestM(TestPKUtils_FaceGetAdjacentFace);
    TestM(TestPKUtils_FaceGetOuterLoopEdges);
    TestM(TestPKUtils_EdgeGetFacesByVertices);

    TestM(TestPKUtils027);

    TestM(TestPKUtils_VectorDifference);
    TestM(TestPKUtils_VectorIntersection);

    TestM(TestPKUtils028_WireBodyEdgeSplitAndKnit);
    TestM(TestPKUtils028_WireBodyEdgeSplitAndCombine);

    TestM(TestPKUtils029_VertexIsSmooth);
    TestM(TestPKUtils030_EdgesGetCoincidentVertices);
    TestM(TestPKUtils031_EdgesGetCoincidentVertices);

    TestM(TestPKUtils031_DetectionOfEdgesWithOpenEndVertex);
    TestM(TestPKUtils032_DetectionOfEdgesWithOpenEndVertex);

    TestM(TestPKUtils033_BcurveGetGeo01);
    TestM(TestPKUtils033_BcurveGetGeo02);
    TestM(TestPKUtils033_BcurveGetGeo03);

    TestM(TestPKUtils034_Bcurve01);
    TestM(TestPKUtils034_Bcurve02);

    TestM(TestPKUtils035_Mirroring_01);
    TestM(TestPKUtils035_Mirroring_02);
    TestM(TestPKUtils035_Mirroring_03);

    TestM(TestPKUtils036_CircularEdgeReplaceCurve);

    TestM(TestPKUtils037_DetectScalingTransform);

    TestM(TestPKUtils038_GenerateTranslationFromScalingTransform01);
    TestM(TestPKUtils038_GenerateTranslationFromScalingTransform02);
    TestM(TestPKUtils038_GenerateTranslationFromScalingTransform03);

    TestM(TestPKUtils039_FaceIsSheet01);
    TestM(TestPKUtils039_FaceIsSheet02);
    TestM(TestPKUtils039_FaceIsSheet03);

    TestM(TestPKUtils040_FaceGetPeriodicRange01);
    TestM(TestPKUtils041_WireBodyCreation);

    TestM(TestPKUtils042_MakeWirebodyFromCurves1);
    TestM(TestPKUtils042_MakeWirebodyFromCurves2);
    TestM(TestPKUtils042_MakeWirebodyFromCurves3);
    TestM(TestPKUtils042_MakeWirebodyFromCurves4);
    TestM(TestPKUtils042_MakeWirebodyFromCurves5);
    TestM(TestPKUtils042_MakeWirebodyFromCurves6_ZeroLengthBaseEdge);

    TestM(TestPKUtils043_SplitWirebodyIntoDisjointBodies);
    TestM(TestPKUtils044_CurveHasAtLeastG1Continuity);

    TestM(TestPKUtils045_EllipseData01);
    TestM(TestPKUtils045_EllipseData02);
    TestM(TestPKUtils045_EllipseData03);
    TestM(TestPKUtils045_EllipseData04);
    TestM(TestPKUtils045_EllipseData05);
    TestM(TestPKUtils045_EllipseData06);

    TestM(TestPKUtils046_CylinderRangeAndMidpoint_bad_inputs);
    TestM(TestPKUtils046_CylinderRangeAndMidpoint_points_across_cut);

    TestM(TestPKUtils047_InterpolatedBcurve01);
    TestM(TestPKUtils047_InterpolatedBcurve02);
    TestM(TestPKUtils047_InterpolatedBcurve03);

    TestM(TestPKUtils048_PointIsAtEndOfEdgeCurve01);
    TestM(TestPKUtils048_VertexIsAtEndOfEdgeCurve01);

    TestM(TestPKUtils047_InterpolatedBcurveWithDers01);
    TestM(TestPKUtils047_InterpolatedBcurveWithDers02);
    TestM(TestPKUtils047_InterpolatedBcurveWithDers03);

    TestM(TestPKUtils048_PerformanceTest_FindNABox);

    TestM(TestPKUtils049);

    TestM(TestEllipseDataUnit001);
        
    TestM(TestPKUtils049_ZeroRadiusCircle01);
    TestM(TestPKUtils049_ZeroRadiusCircle02);
    TestM(TestPKUtils050_ZeroLengthEdge01);
    TestM(TestPKUtils050_ZeroLengthEdge02);
    TestM(TestPKUtils050_ZeroLengthEdge03);
    TestM(TestPKUtils050_ZeroLengthEdge04);

    TestM(TestPKUtils051_IdentifySharpTangent);

    TestM(TestPKUtils052_WirebodyMakeFromCurves);

    TestM(TestPKUtils053_ComputeNonEmptyInterval);

    TestM(TestPKUtils054_CurveOutputVectors_OutOfRange);
    TestM(TestPKUtils055_OffsetCurveExtension);
    TestM(TestPKUtils056_AttachDetachOffsetCurve);

    TestM(TestPKUtils057_EllipseEllipseTangent);

    TestM(TestPKUtils058_BCurveEllipseNearlyTAETangentPositions);
    TestM(TestPKUtils059_BCurveEllipseTAETangentPositions);
    TestFailM(TestPKUtils060_BCurveBCurveTangentPositions); //PR 9232933
    TestM(TestPKUtils061_BCurveBCurveTangentPositions);
    TestM(TestPKUtils062_EllipseEllipseTangentPositions);
    TestFailM(TestPKUtils063_LineBCurveTangentPositions); //See PR 9232933
    TestM(TestPKUtils064_EllipseBCurveTangentPositions);
    TestM(TestPKUtils065_EllipseBCurveTangentPositions);
    TestM(TestPKUtils066_CircleBCurveTangentPositions);
    TestM(TestPKUtils067_EllipseBCurveTangentPositions);

    TestM(TestPKUtils068_OffsetCurveFindParamForPos);
    TestM(TestPKUtils068_OffsetCurveFindParamForPos02);
    TestM(TestPKUtils068_OffsetCurveFindParamForPos03);
    TestM(TestPKUtils068_OffsetCurveFindParamForPos04);
    TestM(TestPKUtils068_OffsetCurveFindParamForPos05);
    TestM(TestPKUtils068_OffsetCurveFindParamForPos06); //Test to show inaccurate output from PK_GEOM_range_local_vector. PR 9288756 has been created.
    TestM(TestPKUtils069_OffsetCurveEdgeContainsPoint);
    TestM(TestPKUtils070_OffsetCurveEdgeGetTangentDirAtParam);
    TestM(TestPKUtils071_OffsetCurveEdgeFindEndTangents);
    TestM(TestPKUtils072_OffsetCurveGetClosestParamsForPos);
    TestM(TestPKUtils073_CompareFacetBodies);

    TestM(TestPKUtils074_ChamferEdges);

    TestM(TestPKUtils075_CurveMakeWirebody_AvoidTolerantVertices);

    TestM(TestPKUtils076_MergeNonMergeableVertexAttributes);
    TestM(TestPKUtils077a_CheckMergingAttributeOnNonMergeableVertex);
    TestM(TestPKUtils077b_CheckMergingAttributeOnNonMergeableVertex);
    TestM(TestPKUtils077c_CheckMergingAttributeOnNonMergeableVertex);

    PerfTestM(TestPKUtils078_TestPKPerformanceHammer);
    PerfTestM(TestPKUtils079_TestPVMPerformanceHammer);

    TestM(TestPKUtils080a_CheckGetContinuityChecking001);
    TestM(TestPKUtils080b_CheckGetContinuityChecking002);
    TestM(TestPKUtils080c_CheckSetContinuityChecking001);
    TestM(TestPKUtils081_CurveIsClosed001);
    TestM(TestPKUtils082_RepairSplitEdgesForWirebodyCreation001);
    TestM(TestPKUtils082_RepairSplitEdgesForWirebodyCreation002);
    TestM(TestPKUtils082_RepairSplitEdgesForWirebodyCreation003);
    TestM(TestPKUtils082_RepairSplitEdgesForWirebodyCreation004);
    TestM(TestPKUtils083_EdgeIsPeriodic001);
    TestM(TestPKUtils083_EdgeIsPeriodic002);
    TestM(TestPKUtils084_EdgeIsClosedNonPeriodic001);
    TestM(TestPKUtils084_EdgeIsClosedNonPeriodic002);

    TestFailM(TestPKUtils085_PR_9497136_compare);

    TestM(TestPKUtils086_CurveEvaluateAtParameter_EllipseParameterConversion01);
    TestM(TestPKUtils086_CurveEvaluateAtParameter_EllipseParameterConversion02);

    TestM(TestPKUtils087_Parameter_Can_Be_Found_On_Linear_Extension_Of_OffsetCurve_Outside_Curve_Interval);

    TestM(TestPKUtils_EntityGetClass_SingleSegmentPLine);

    TestM(TestPKUtils088_SplitEdgesOfG0Curve_With_Missing_Vertex_Can_Be_Merged);

    TestM(TestPKUtils088_CircleEdgeMidpoints);
    TestM(TestPKUtils089_CircleEdgeMidpoints);

    TestM(TestPKUtils090_ZeroRadiusArc_UpdateCurveData);

    TestM(TestPKUtils091_CheckEdgeEdgeIntersectionFaultFiltering);

    TestM(TestPKUtils092a_CurveFindPositionsExactDistanceFromGivenPositionInPlane_ZeroIntersections);
    TestM(TestPKUtils092b_CurveFindPositionsExactDistanceFromGivenPositionInPlane_OneIntersection);
    TestM(TestPKUtils092c_CurveFindPositionsExactDistanceFromGivenPositionInPlane_MultipleIntersections);
    TestM(TestPKUtils092d_CurveFindPositionsExactDistanceFromGivenPositionInPlane_InfiniteIntersections);
    TestM(TestPKUtils092e_CurveFindPositionsExactDistanceFromGivenPositionInPlane_DistanceResZero);
    TestM(TestPKUtils092f_CurveFindPositionsExactDistanceFromGivenPositionInPlane_CurveNotInSamePlaneAsPoint);

    TestM(EdgeIsSingleSegmentPline_Works);
    TestM(CageEdgeGetDir_WorksForLineAndSingleSegmentPlineGeoms);
    TestM(EntityIsBsurf_Works);
    TestM(EntityIsCurve_WorksWith2DModel);
    TestM(EntityIsCurve_WorksWith3DModel);
    TestM(EdgeGetFaces_SetVariantWorksWithLinearEdge);
    TestM(EdgeGetFaces_SetVariantWorksWithSingleSegmentPlineEdge);
    TestM(EdgeGetFaces_SetVariantWorksWithCircularEdge);

    TestM(TestPKUtils093a_MinimisePKEntityIsCallsDuringConfigure_ConfigureCase);

    TestM(TestPKUtils094_CreateWirebody_Finds_Accurate_Intersection_Between_Line_And_OffsetCurve);

    TestM(TestPKUtils095_GeomCheck_Ignores_SelfIntersectingState_If_CheckSelfIntSessionOptionIsOff);
    TestM(TestPKUtils999_SortingHatPointCloudToSolid);
    TestM(TestPKUtils999_SpaceCarveShortWall001);
    TestM(TestPKUtils999_SpaceCarveSpriteCan001);
    TestM(TestPKUtils999_SpaceCarveSpriteCan002);
    TestM(TestPKUtils999_SpaceCarveSortingHat001);
    TestM(TestPKUtils999_SpaceCarve001);
    

    return true;
}
