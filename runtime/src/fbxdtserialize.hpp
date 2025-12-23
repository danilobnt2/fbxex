# pragma once
/* FBX Data Type Serialization Utility
 *
 * FbxDataType to SerializedDataType mapping (trivial JSON-friendly forms only).
 * Use <unsupported> when the FBX type needs custom handling.
  * FbxDataType FbxUndefinedDT, SerializedDataType              ;
  * FbxDataType      FbxNullDT, SerializedDataType          null;
  * FbxDataType      FbxBoolDT, SerializedDataType          bool;
  * FbxDataType      FbxCharDT, SerializedDataType        number;
  * FbxDataType     FbxUCharDT, SerializedDataType        number;
  * FbxDataType     FbxShortDT, SerializedDataType        number;
  * FbxDataType    FbxUShortDT, SerializedDataType        number;
  * FbxDataType       FbxIntDT, SerializedDataType        number;
  * FbxDataType      FbxUIntDT, SerializedDataType        number;
  * FbxDataType  FbxLongLongDT, SerializedDataType        number;
  * FbxDataType FbxULongLongDT, SerializedDataType        number;
  * FbxDataType     FbxFloatDT, SerializedDataType        number;
  * FbxDataType FbxHalfFloatDT, SerializedDataType        number;
  * FbxDataType    FbxDoubleDT, SerializedDataType        number;
  * FbxDataType   FbxDouble2DT, SerializedDataType array<number>;
  * FbxDataType   FbxDouble3DT, SerializedDataType array<number>;
  * FbxDataType   FbxDouble4DT, SerializedDataType array<number>;
  * FbxDataType FbxDouble4x4DT, SerializedDataType array<number>;
  * FbxDataType      FbxEnumDT, SerializedDataType <unsupported>;
  * FbxDataType    FbxStringDT, SerializedDataType        string;
  * FbxDataType      FbxTimeDT, SerializedDataType <unsupported>;
  * FbxDataType FbxReferenceDT, SerializedDataType <unsupported>;
  * FbxDataType      FbxBlobDT, SerializedDataType <unsupported>;
  * FbxDataType  FbxDistanceDT, SerializedDataType <unsupported>;
  * FbxDataType  FbxDateTimeDT, SerializedDataType <unsupported>;
  * 
 * FbxDataType                 FbxColor3DT, SerializedDataType        object; // {r: number, g: number, b: number};
  * FbxDataType                FbxColor4DT, SerializedDataType        object; // {r: number, g: number, b: number, a: number};
  * FbxDataType              FbxCompoundDT, SerializedDataType <unsupported>;
  * FbxDataType       FbxReferenceObjectDT, SerializedDataType <unsupported>;
  * FbxDataType     FbxReferencePropertyDT, SerializedDataType <unsupported>;
  * FbxDataType            FbxVisibilityDT, SerializedDataType <unsupported>;
  * FbxDataType FbxVisibilityInheritanceDT, SerializedDataType <unsupported>;
  * FbxDataType                   FbxUrlDT, SerializedDataType        string;
  * FbxDataType               FbxXRefUrlDT, SerializedDataType <unsupported>;
  * 
  * FbxDataType       FbxTranslationDT, SerializedDataType               object; // {x: number, y: number, z: number}
  * FbxDataType          FbxRotationDT, SerializedDataType               object; // {x: number, y: number, z: number}
  * FbxDataType           FbxScalingDT, SerializedDataType               object; // {x: number, y: number, z: number}
  * FbxDataType        FbxQuaternionDT, SerializedDataType               object; // {w: number, x: number, y: number, z: number}
  * FbxDataType  FbxLocalTranslationDT, SerializedDataType               object; // {x: number, y: number, z: number}
  * FbxDataType     FbxLocalRotationDT, SerializedDataType               object; // {x: number, y: number, z: number}
  * FbxDataType      FbxLocalScalingDT, SerializedDataType               object; // {x: number, y: number, z: number}
  * FbxDataType   FbxLocalQuaternionDT, SerializedDataType               object; // {w: number, x: number, y: number, z: number}
  * FbxDataType   FbxTransformMatrixDT, SerializedDataType array<array<number>>; // 4x4 matrix rows
  * FbxDataType FbxTranslationMatrixDT, SerializedDataType array<array<number>>; // 4x4 matrix rows
  * FbxDataType    FbxRotationMatrixDT, SerializedDataType array<array<number>>; // 4x4 matrix rows
  * FbxDataType     FbxScalingMatrixDT, SerializedDataType array<array<number>>; // 4x4 matrix rows
  * 
  * FbxDataType           FbxMaterialEmissiveDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType     FbxMaterialEmissiveFactorDT, SerializedDataType        number;
  * FbxDataType            FbxMaterialAmbientDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType      FbxMaterialAmbientFactorDT, SerializedDataType        number;
  * FbxDataType            FbxMaterialDiffuseDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType      FbxMaterialDiffuseFactorDT, SerializedDataType        number;
  * FbxDataType               FbxMaterialBumpDT, SerializedDataType        number;
  * FbxDataType          FbxMaterialNormalMapDT, SerializedDataType        number;
  * FbxDataType   FbxMaterialTransparentColorDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType FbxMaterialTransparencyFactorDT, SerializedDataType        number;
  * FbxDataType           FbxMaterialSpecularDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType     FbxMaterialSpecularFactorDT, SerializedDataType        number;
  * FbxDataType          FbxMaterialShininessDT, SerializedDataType        number;
  * FbxDataType         FbxMaterialReflectionDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType   FbxMaterialReflectionFactorDT, SerializedDataType        number;
  * FbxDataType       FbxMaterialDisplacementDT, SerializedDataType        number;
  * FbxDataType FbxMaterialVectorDisplacementDT, SerializedDataType        object; // {r: number, g: number, b: number}
  * FbxDataType       FbxMaterialCommonFactorDT, SerializedDataType        number;
  * FbxDataType      FbxMaterialCommonTextureDT, SerializedDataType <unsupported>;
  * 
  * FbxDataType    FbxLayerElementUndefinedDT, SerializedDataType <unsupported>;
  * FbxDataType       FbxLayerElementNormalDT, SerializedDataType <unsupported>;
  * FbxDataType     FbxLayerElementBinormalDT, SerializedDataType <unsupported>;
  * FbxDataType      FbxLayerElementTangentDT, SerializedDataType <unsupported>;
  * FbxDataType     FbxLayerElementMaterialDT, SerializedDataType <unsupported>;
  * FbxDataType      FbxLayerElementTextureDT, SerializedDataType <unsupported>;
  * FbxDataType FbxLayerElementPolygonGroupDT, SerializedDataType <unsupported>;
  * FbxDataType           FbxLayerElementUVDT, SerializedDataType <unsupported>;
  * FbxDataType  FbxLayerElementVertexColorDT, SerializedDataType <unsupported>;
  * FbxDataType    FbxLayerElementSmoothingDT, SerializedDataType <unsupported>;
  * FbxDataType       FbxLayerElementCreaseDT, SerializedDataType <unsupported>;
  * FbxDataType         FbxLayerElementHoleDT, SerializedDataType <unsupported>;
  * FbxDataType     FbxLayerElementUserDataDT, SerializedDataType <unsupported>;
  * FbxDataType   FbxLayerElementVisibilityDT, SerializedDataType <unsupported>;
  * 
  * FbxDataType        FbxAliasDT, SerializedDataType        string;
  * FbxDataType      FbxPresetsDT, SerializedDataType <unsupported>;
  * FbxDataType   FbxStatisticsDT, SerializedDataType <unsupported>;
  * FbxDataType     FbxTextLineDT, SerializedDataType        string;
  * FbxDataType        FbxUnitsDT, SerializedDataType <unsupported>;
  * FbxDataType      FbxWarningDT, SerializedDataType        string;
  * FbxDataType          FbxWebDT, SerializedDataType <unsupported>;
  * 
  * FbxDataType             FbxActionDT, SerializedDataType <unsupported>;
  * FbxDataType        FbxCameraIndexDT, SerializedDataType        number;
  * FbxDataType            FbxCharPtrDT, SerializedDataType        number;
  * FbxDataType          FbxConeAngleDT, SerializedDataType        number;
  * FbxDataType              FbxEventDT, SerializedDataType <unsupported>;
  * FbxDataType        FbxFieldOfViewDT, SerializedDataType        number;
  * FbxDataType       FbxFieldOfViewXDT, SerializedDataType        number;
  * FbxDataType       FbxFieldOfViewYDT, SerializedDataType        number;
  * FbxDataType                FbxFogDT, SerializedDataType        number;
  * FbxDataType                FbxHSBDT, SerializedDataType        object; // {h: number, s: number, b: number}
  * FbxDataType FbxIKReachTranslationDT, SerializedDataType <unsupported>;
  * FbxDataType    FbxIKReachRotationDT, SerializedDataType        number;
  * FbxDataType          FbxIntensityDT, SerializedDataType        number;
  * FbxDataType             FbxLookAtDT, SerializedDataType <unsupported>;
  * FbxDataType          FbxOcclusionDT, SerializedDataType        number;
  * FbxDataType     FbxOpticalCenterXDT, SerializedDataType        number;
  * FbxDataType     FbxOpticalCenterYDT, SerializedDataType        number;
  * FbxDataType        FbxOrientationDT, SerializedDataType <unsupported>;
  * FbxDataType               FbxRealDT, SerializedDataType        number;
  * FbxDataType               FbxRollDT, SerializedDataType        number;
  * FbxDataType          FbxScalingUVDT, SerializedDataType        object; // {u: number, v: number}
  * FbxDataType              FbxShapeDT, SerializedDataType <unsupported>;
  * FbxDataType         FbxStringListDT, SerializedDataType array<string>;
  * FbxDataType    FbxTextureRotationDT, SerializedDataType        object; // {u: number, v: number}
  * FbxDataType           FbxTimeCodeDT, SerializedDataType <unsupported>;
  * FbxDataType           FbxTimeWarpDT, SerializedDataType <unsupported>;
  * FbxDataType      FbxTranslationUVDT, SerializedDataType        object; // {u: number, v: number}
  * FbxDataType             FbxWeightDT, SerializedDataType        number;
  * 
*/

#include <fbxsdk.h>
#include <nlohmann/json.hpp>
#include <string>

// Convert a generic FbxProperty into a simple JSON representation:
// { name: string, type: string, value: supported JSON value }
// Unsupported property types throw (caller decides how to handle).
nlohmann::json SerializeFbxProperty(const FbxProperty& property);
