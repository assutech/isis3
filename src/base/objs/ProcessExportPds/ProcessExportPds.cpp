/**
 *   Unless noted otherwise, the portions of Isis written by the
 *   USGS are public domain. See individual third-party library
 *   and package descriptions for intellectual property
 *   information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.
 *
 *   For additional information, launch 
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website, 
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */
#include <iostream>
#include <sstream>
#include <cmath>

#include "Pvl.h"
#include "PvlFormat.h"
#include "iException.h"
#include "PixelType.h"
#include "iString.h"
#include "PvlTranslationManager.h"
#include "Filename.h"
#include "Endian.h"
#include "PvlFormatPds.h"
#include "SpecialPixel.h"
#include "ProjectionFactory.h"

#include "ProcessExportPds.h"

using namespace std;

namespace Isis {
  ProcessExportPds::ProcessExportPds(){
    p_label = NULL;
    p_formatter = NULL;
    p_exportType = Stream;
  }

  ProcessExportPds::~ProcessExportPds(){
    if (p_label != NULL) {
      delete p_label;
      delete p_formatter;
    }
  }


  /** 
   * Create a standard PDS label of the type specified
   * 
   * Create a standard PDS label of the type specified. The type must be Image.
   * Qube and SpectralQube are not supported. 
   * 
   * NOTE: This method should not be called until all setting have been
   * finished. For example ProcessExport::SetOutputEndian
   * 
   * @param type The type of PDS label to be created.
   * 
   * @throws Isis::iException::Message
   */
  Pvl& ProcessExportPds::StandardPdsLabel(ProcessExportPds::PdsFileType type){
    p_label = new Pvl;

    p_formatter = new PvlFormatPds("$base/translations/pdsExportRootGen.typ");
    p_label->SetFormat(p_formatter);
    p_label->SetTerminator("END");

    if (type == ProcessExportPds::Image) {
      CreateImageLabel();
    }
    else {
      string msg = "Unsupported PDS output type";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    return *p_label;
  }


  /** 
   * Create a standard PDS label for type IMAGE
   */
  void ProcessExportPds::CreateImageLabel() {

    Pvl &mainPvl = *p_label;

    if ( p_exportType == Stream ) {
      StreamImageRoot (mainPvl);
    }
    else if ( p_exportType == Fixed ) {
      FixedImageRoot (mainPvl);
    }
    else {
      string msg = "Invalid PDS export type";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    StandardImageImage (mainPvl);

    // The IMAGE_MAP_PROJECTION group is located in the ROOT for PDS IMAGEs. The
    // standard routines will add the IMAGE_MAP_PROJECTION correctly
    StandardAllMapping (mainPvl);
    mainPvl.GetFormat()->Add("$base/translations/pdsExportAllMapping.typ");
  }


  /** 
   * Create a standard PDS label for type QUBE
   */
  void ProcessExportPds::CreateQubeLabel() {
    Pvl &mainPvl = *p_label;

//    StandardQubeRoot (mainPvl);
//    StandardQubeQube (mainPvl);

    // The IMAGE_MAP_PROJECTION group is located inside the QUBE object for PDS
    // QUBEs. Create a temporary PVL so the StandardAllMapping member can add an
    // IMAGE_MAP_PROJECTION group then later it can be extracted and added to
    // the output PDS label.
    Pvl mapTmp;
    StandardAllMapping (mapTmp);
    if (mapTmp.HasObject("IMAGE_MAP_PROJECTION")) {
      mainPvl.FindObject("QUBE").AddObject(mapTmp.FindObject("IMAGE_MAP_PROJECTION"));      
    }
  }


  /** 
   * Create a standard PDS label for type SPECTRAL_QUBE
   */
  void ProcessExportPds::CreateSpectralQubeLabel() {
    Pvl &mainPvl = *p_label;

//    StandardSpectralQubeRoot (mainPvl);
//    StandardSpectralQubeSpectralQube (mainPvl);

    // The IMAGE_MAP_PROJECTION group is located inside the SPECTRAL_QUBE object
    // for PDS SPECTRAL_QUBEs. Create a temporary PVL so the StandardAllMapping
    // member can add an IMAGE_MAP_PROJECTION group then later it can be
    // extracted and added to the output PDS label.
    Pvl mapTmp;
    StandardAllMapping (mapTmp);
    if (mapTmp.HasObject("IMAGE_MAP_PROJECTION")) {
      mainPvl.FindObject("QUBE").AddObject(mapTmp.FindObject("IMAGE_MAP_PROJECTION"));      
    }
  }


  /** 
   * Create the standard keywords for the ROOT object in a PDS IMAGE file 
   */
  void ProcessExportPds::StreamImageRoot(Pvl &mainPvl) {
    // Create standard ROOT object keywords
    mainPvl += PvlKeyword("PDS_VERSION_ID", "PDS3");
    mainPvl += PvlKeyword("RECORD_TYPE","UNDEFINED");
    // NOTE: WARNING: If the number of "?"s in the next few lines changes, you
    // must also changes the corresponding lines in the OutputLabel member
    mainPvl += PvlKeyword("LABEL_RECORDS", "???????","BYTES");
    mainPvl += PvlKeyword("^IMAGE", "???????","BYTES");
  }


  /**
   * Create the fixed keywords for the ROOT object in a PDS IMAGE file 
   */
  void ProcessExportPds::FixedImageRoot(Pvl &mainPvl) {
    //Create fixed ROOT object keywords
    mainPvl += PvlKeyword("PDS_VERSION_ID", "PDS3");
    mainPvl += PvlKeyword("RECORD_TYPE","FIXED_LENGTH");
    // NOTE: WARNING: If the number of "?"s in the next few lines changes, you 
    // must also changes the corresponding lines in the OutputLabel member
    mainPvl += PvlKeyword("RECORD_BYTES", "???????");
    mainPvl += PvlKeyword("FILE_RECORDS", "???????");
    mainPvl += PvlKeyword("LABEL_RECORDS", "????");
    mainPvl += PvlKeyword("^IMAGE", "???");
  }


  /**
   * Create the standard keywords for an IMAGE object in a PDS IMAGE file.
   * This should not be called until after all settings have been made. The
   * labels may contain the wrong data if it is.
   * 
   * @throws Isis::iException::Message
   */
  void ProcessExportPds::StandardImageImage(Pvl & mainPvl) {
    mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImage.typ");
    // Build up an IMAGE object:
    // Auto translate standard keywords for the IMAGE object
    Pvl *inputLabel = InputCubes[0]->Label();
    Filename transfile("$base/translations/pdsExportImageImage.trn");
    PvlTranslationManager Xlator(*inputLabel, transfile.Expanded());
    Xlator.Auto(mainPvl);

    // Calculate the core base/mult for this cube
    double base = 0.0;
    double multiplier = 1.0;
    double x1,x2;

    double minimum = (p_inputMinimum.size())? p_inputMinimum[0] : 0.0;
    double maximum = (p_inputMaximum.size())? p_inputMaximum[0] : 0.0;

    for(unsigned int i = 0; i < p_inputMinimum.size(); i ++) {
      minimum = std::min(minimum, p_inputMinimum[i]);
      maximum = std::max(maximum, p_inputMaximum[i]);
    }

    x1 = p_outputMinimum;
    x2 = p_outputMaximum;

    if (p_inputMinimum.size() && p_pixelType == Isis::UnsignedByte) {
      multiplier = (maximum - minimum) / (x2 - x1);
      base = minimum - multiplier * x1;
    }
    else if (p_inputMinimum.size() && p_pixelType == Isis::SignedWord) {
      multiplier = (maximum - minimum) / (x2 - x1);
      base = minimum - multiplier * x1;
    }
    else if (p_inputMinimum.size() && p_pixelType == Isis::UnsignedWord) {
      multiplier = (maximum - minimum) / (x2 - x1);
      base = minimum - multiplier * x1;
    }

    // Manually set the keyword for the number of bits in a pixel
    // NOTE: this is dependent on settings in ProcessExport and not the cube
    PvlObject &imgObj = mainPvl.FindObject("IMAGE");

    imgObj += PvlKeyword("BAND_STORAGE_TYPE", "BAND_SEQUENTIAL");
    imgObj += PvlKeyword("OFFSET", base);
    imgObj += PvlKeyword("SCALING_FACTOR", multiplier);

    // Manually set the keyword for pixel type and special pixels
    if (p_pixelType == Isis::UnsignedByte) {
      imgObj += PvlKeyword("SAMPLE_BITS", "8");
      imgObj += PvlKeyword("SAMPLE_BIT_MASK", 0xff);
      imgObj += PvlKeyword("SAMPLE_TYPE", "MSB_UNSIGNED_INTEGER");
      imgObj += PvlKeyword("CORE_NULL", (int)OutputNull());
      imgObj += PvlKeyword("CORE_LOW_REPR_SATURATION", (int)OutputLrs());
      imgObj += PvlKeyword("CORE_LOW_INSTR_SATURATION", (int)OutputLis());
      imgObj += PvlKeyword("CORE_HIGH_REPR_SATURATION", (int)OutputHrs());
      imgObj += PvlKeyword("CORE_HIGH_INSTR_SATURATION", (int)OutputHis());
      mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImagePixel8.typ");
    }
    else if ((p_pixelType == Isis::UnsignedWord) && (p_endianType == Isis::Msb)) {
      imgObj += PvlKeyword("SAMPLE_BITS", "16");
      imgObj += PvlKeyword("SAMPLE_BIT_MASK", 0xffff);
      imgObj += PvlKeyword("SAMPLE_TYPE", "MSB_UNSIGNED_INTEGER");
      imgObj += PvlKeyword("CORE_NULL", (int)OutputNull());
      imgObj += PvlKeyword("CORE_LOW_REPR_SATURATION", (int)OutputLrs());
      imgObj += PvlKeyword("CORE_LOW_INSTR_SATURATION", (int)OutputLis());
      imgObj += PvlKeyword("CORE_HIGH_REPR_SATURATION", (int)OutputHrs());
      imgObj += PvlKeyword("CORE_HIGH_INSTR_SATURATION", (int)OutputHis());
      mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImagePixel16.typ");
    }
    else if ((p_pixelType == Isis::UnsignedWord) && (p_endianType == Isis::Lsb)) {
      imgObj += PvlKeyword("SAMPLE_BITS", "16");
      imgObj += PvlKeyword("SAMPLE_BIT_MASK", 0xffff);
      imgObj += PvlKeyword("SAMPLE_TYPE", "LSB_UNSIGNED_INTEGER");
      imgObj += PvlKeyword("CORE_NULL", (int)OutputNull());
      imgObj += PvlKeyword("CORE_LOW_REPR_SATURATION",  (int)OutputLrs());
      imgObj += PvlKeyword("CORE_LOW_INSTR_SATURATION", (int)OutputLis());
      imgObj += PvlKeyword("CORE_HIGH_REPR_SATURATION", (int)OutputHrs());
      imgObj += PvlKeyword("CORE_HIGH_INSTR_SATURATION",(int)OutputHis());
      mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImagePixel16.typ");
    }
    else if ((p_pixelType == Isis::SignedWord) && (p_endianType == Isis::Msb)) {
      imgObj += PvlKeyword("SAMPLE_BITS", "16");
      imgObj += PvlKeyword("SAMPLE_BIT_MASK", 0xffff);
      imgObj += PvlKeyword("SAMPLE_TYPE", "MSB_INTEGER");
      imgObj += PvlKeyword("CORE_NULL", (int)OutputNull());
      imgObj += PvlKeyword("CORE_LOW_REPR_SATURATION",  (int)OutputLrs());
      imgObj += PvlKeyword("CORE_LOW_INSTR_SATURATION", (int)OutputLis());
      imgObj += PvlKeyword("CORE_HIGH_REPR_SATURATION", (int)OutputHrs());
      imgObj += PvlKeyword("CORE_HIGH_INSTR_SATURATION",(int)OutputHis());
      mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImagePixel16.typ");
    }
    else if ((p_pixelType == Isis::SignedWord) && (p_endianType == Isis::Lsb)) {
      imgObj += PvlKeyword("SAMPLE_BITS", "16");
      imgObj += PvlKeyword("SAMPLE_BIT_MASK", (BigInt)0xffff);
      imgObj += PvlKeyword("SAMPLE_TYPE", "LSB_INTEGER");
      imgObj += PvlKeyword("CORE_NULL", (int)OutputNull());
      imgObj += PvlKeyword("CORE_LOW_REPR_SATURATION",  (int)OutputLrs());
      imgObj += PvlKeyword("CORE_LOW_INSTR_SATURATION", (int)OutputLis());
      imgObj += PvlKeyword("CORE_HIGH_REPR_SATURATION", (int)OutputHrs());
      imgObj += PvlKeyword("CORE_HIGH_INSTR_SATURATION",(int)OutputHis());
      mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImagePixel16.typ");
    }
    else if (p_pixelType == Isis::Real) {
      imgObj += PvlKeyword("SAMPLE_BITS", "32");
      imgObj += PvlKeyword("SAMPLE_BIT_MASK", (BigInt)0xffffffff);

      if (p_endianType == Isis::Msb) {
        imgObj += PvlKeyword("SAMPLE_TYPE", "IEEE_REAL");
      }
      else {
        imgObj += PvlKeyword("SAMPLE_TYPE", "PC_REAL");
      }
      imgObj += PvlKeyword("CORE_NULL", Isis::INULL4);
      imgObj += PvlKeyword("CORE_LOW_REPR_SATURATION", Isis::ILOW_REPR_SAT4);
      imgObj += PvlKeyword("CORE_LOW_INSTR_SATURATION", Isis::ILOW_INSTR_SAT4);
      imgObj += PvlKeyword("CORE_HIGH_REPR_SATURATION", Isis::IHIGH_REPR_SAT4);
      imgObj += PvlKeyword("CORE_HIGH_INSTR_SATURATION", Isis::IHIGH_INSTR_SAT4);
      mainPvl.GetFormat()->Add("$base/translations/pdsExportImageImagePixel32.typ");
    }
    else {
      string msg = "Unsupported PDS pixel type or sample size";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
  }


  /** 
   * Create the standard keywords for the IMAGE_MAP_PROJECTION group in a PDS
   * label
   * 
   * @throws Isis::iException::Message
   */
  void ProcessExportPds::StandardAllMapping(Pvl &outputPvl) {

    // Get the input Isis cube label and find the Mapping group if it has one
    Pvl *inputLabel = InputCubes[0]->Label();
    if (inputLabel->HasObject("IsisCube") &&
        !(inputLabel->FindObject("IsisCube").HasGroup("Mapping"))) return;
    PvlGroup &inputMapping = inputLabel->FindGroup("Mapping",Pvl::Traverse);
    // Translate the common keywords for a PDS IMAGE_MAP_PROJECTION
    PvlTranslationManager xlatGenProj(*inputLabel,
                                      "$base/translations/pdsExportAllMapping.trn");
    xlatGenProj.Auto(outputPvl);

    // Translate the projection specific keywords for a PDS IMAGE_MAP_PROJECTION
    iString projName = ProjectionName (*inputLabel);
    PvlTranslationManager xlatSpecProj(*inputLabel,
                                       "$base/translations/pdsExport"+ projName+".trn");
    xlatSpecProj.Auto(outputPvl);

    // Translate the target name
    PvlTranslationManager xlatTarget(*inputLabel,
                                     "$base/translations/pdsExportTarget.trn");
    xlatTarget.Auto(outputPvl);

    // Add keywords to the PDS labels that could not be handled automatically
    PvlObject &pdsMapObj = outputPvl.FindObject("IMAGE_MAP_PROJECTION");

    // Add the projection name
//    pdsMapObj += PvlKeyword ("MAP_PROJECTION_TYPE", projName.UpCase());

    // Modify the radii to be km
    PvlKeyword &aRadius = pdsMapObj["A_AXIS_RADIUS"];
    iString unit = aRadius.Unit();
    if (unit.UpCase() == "METERS") {
      aRadius.SetValue((double)aRadius/1000.0, "KM");
    }
    PvlKeyword &bRadius = pdsMapObj["B_AXIS_RADIUS"];
    unit = bRadius.Unit();
    if (unit.UpCase() == "METERS") {
      bRadius.SetValue((double)bRadius/1000.0, "KM");
    }
    PvlKeyword &cRadius = pdsMapObj["C_AXIS_RADIUS"];
    unit = cRadius.Unit();
    if (unit.UpCase() == "METERS") {
      cRadius.SetValue((double)cRadius/1000.0, "KM");
    }

    // Modify the units on MAP_SCALE and MAP_RESOLUTION
    PvlKeyword &mapScale = pdsMapObj["MAP_SCALE"];
    unit = mapScale.Unit();
    if ((unit.UpCase() == "METERS/PIX") || (unit.UpCase() == "METERS/PIXEL")) {
      mapScale.SetValue((double)mapScale, "METERS/PIXEL");
    }
    PvlKeyword &mapRes = pdsMapObj["MAP_RESOLUTION"];
    unit = mapRes.Unit();
    if (unit.UpCase() == "PIXELS/DEGREE") {
      mapRes.SetValue((string)mapRes, "PIX/DEG");
    }


    // Add the EASTERNMOST AND WESTERNMOST LONGITUDE keywords
    PvlKeyword &isisLonDir = inputMapping.FindKeyword("LongitudeDirection");
    iString lonDir = isisLonDir[0];
    lonDir.UpCase();
    if (lonDir == "POSITIVEEAST") {
      double maxLon = inputMapping.FindKeyword("MaximumLongitude");
      pdsMapObj += PvlKeyword("EASTERNMOST_LONGITUDE", maxLon);
      double minLon = inputMapping.FindKeyword("MinimumLongitude");
      pdsMapObj += PvlKeyword("WESTERNMOST_LONGITUDE", minLon);
    }
    else {
      double minLon = inputMapping.FindKeyword("MinimumLongitude");
      pdsMapObj += PvlKeyword("EASTERNMOST_LONGITUDE", minLon);
      double maxLon = inputMapping.FindKeyword("MaximumLongitude");
      pdsMapObj += PvlKeyword("WESTERNMOST_LONGITUDE", maxLon);
    }

    // Add the LINE_PROJECTION_OFFSET and SAMPLE_PROJECTION_OFFSET keywords
    // These keywords are the distance from the origin of the image to the
    // origin of the projection. The units are line or samples. The image origin
    // is the middle of pixel (1,1)
    double lineOffset = inputMapping.FindKeyword("UpperLeftCornerY");
    lineOffset /= (double)inputMapping.FindKeyword("PixelResolution");
    lineOffset *= 1.0;
    lineOffset += 0.5; // Add half a line to get to the center of (1,1)
    pdsMapObj += PvlKeyword("LINE_PROJECTION_OFFSET", lineOffset,"PIXEL");
    double sampleOffset = inputMapping.FindKeyword("UpperLeftCornerX");
    sampleOffset /= (double)inputMapping.FindKeyword("PixelResolution");
    sampleOffset *= -1.0;
    sampleOffset += 0.5; // Add half a sample to get to the center of (1,1)
    pdsMapObj += PvlKeyword("SAMPLE_PROJECTION_OFFSET", sampleOffset,"PIXEL");

    // Add units to keywords already in the IMAGE_MAP_PROJECTION object as necessary
    if (pdsMapObj.HasKeyword("CENTER_LATITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("CENTER_LATITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
    if (pdsMapObj.HasKeyword("CENTER_LONGITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("CENTER_LONGITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
//    if (pdsMapObj.HasKeyword("REFERENCE_LATITUDE")) {
//      PvlKeyword &tempKey = pdsMapObj.FindKeyword("REFERENCE_LATITUDE");
//      tempKey.SetValue(tempKey[0], "DEG");
//    }
    if (pdsMapObj.HasKeyword("REFERENCE_LONGITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("REFERENCE_LONGITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
    if (pdsMapObj.HasKeyword("MAXIMUM_LATITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("MAXIMUM_LATITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
    if (pdsMapObj.HasKeyword("MINIMUM_LATITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("MINIMUM_LATITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
    if (pdsMapObj.HasKeyword("EASTERNMOST_LONGITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("EASTERNMOST_LONGITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
    if (pdsMapObj.HasKeyword("WESTERNMOST_LONGITUDE")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("WESTERNMOST_LONGITUDE");
      tempKey.SetValue(tempKey[0], "DEG");
    }
    if (pdsMapObj.HasKeyword("MAP_PROJECTION_ROTATION")) {
      PvlKeyword &tempKey = pdsMapObj.FindKeyword("MAP_PROJECTION_ROTATION");
      tempKey.SetValue(tempKey[0], "DEG");
    }

  }


  /** 
   * Return a projection name
   */
  string ProcessExportPds::ProjectionName(Pvl &inputLabel) {
    Projection *proj = ProjectionFactory::Create(inputLabel);
    string name = proj->Name();
    delete proj;
    return name;
  }


  /** 
   * Return the line bytes (record size) for the input cube, at present this is
   * based on the number of samples and the bytes per pixel. 
   */
  int ProcessExportPds::LineBytes(){
    Cube *cube = InputCubes[0];
    int a = SizeOf(cube->PixelType());
    int b = cube->Samples();
    return b * a ;
  }


  /** 
   * Return the size of the output PDS label.
   */
  int ProcessExportPds::LabelSize(){
    ostringstream temp;
    if( p_label->GetFormat() != NULL ) {
      temp << *p_label << p_label->GetFormat()->FormatEOL();
    }
    else {
      temp << *p_label << endl;
    }
    return temp.tellp();
  }


  /** 
   * Write the PDS label to the supplied stream. The PDS keywords that have place 
   * holder "?" for their values (such as LABEL_RECORDS and ^IMAGE) will
   * be updated to their correct values before they are written. 
   */
  void ProcessExportPds::OutputLabel(std::ofstream &os) {
    int labSize = LabelSize(); // labSize will be the old label size with "?"
    // NOTE: WARNING: If anything changes in the next two lines, you must also changes the
    // corresponding lines in the StandardImageRoot member
    if ( p_exportType == Stream ) {
      (*p_label)["LABEL_RECORDS"].SetValue(iString(labSize), "BYTES");
      (*p_label)["^IMAGE"].SetValue(iString(labSize+1), "BYTES");
      if( p_label->GetFormat() != NULL ) {
        os << *p_label << p_label->GetFormat()->FormatEOL();
      }
      else {
        os << *p_label << endl;
      }
      // Fill the difference between the old and new label size with nulls.
      for (int i=LabelSize(); i<labSize; ++i) os << '\0';
    }
    else if ( p_exportType == Fixed ) {
      int lineBytes = LineBytes();
      (*p_label)["RECORD_BYTES"].SetValue( iString(lineBytes) );

      // The number of label records is dependent on the number of label bytes
      // and the lint bytes
      int labelRecords = (int)ceil ( (double)labSize / (double)lineBytes);
      (*p_label)["LABEL_RECORDS"].SetValue( iString(labelRecords) );
      (*p_label)["FILE_RECORDS"].SetValue( iString(labelRecords + (InputCubes[0])->Lines() *
                                                                   InputCubes[0]->Bands()) );
      (*p_label)["^IMAGE"].SetValue( iString(labelRecords+1) );
      if( p_label->GetFormat() != NULL ) {
        os << *p_label << p_label->GetFormat()->FormatEOL();
      }
      else {
        os << *p_label << endl;
      }
      for (int i=LabelSize(); i<labelRecords*lineBytes; ++i) os << '\0';
    }
  }

} // End of Isis namespace
