#include "Isis.h"
#include "ProcessExport.h"

#include "UserInterface.h"
#include "Filename.h"
#include <QImageWriter>
#include <QImage>

using namespace std;
using namespace Isis;
QImage *qimage;

// Line-by-Line qimage output prototype
void checkDataSize (Isis::BigInt line, Isis::BigInt samp, iString mode);
void toGreyscaleImage (Buffer &in);
void toRGBImage (vector<Buffer *> &in);
void toARGBImage (vector<Buffer *> &in);

// Main program
void IsisMain() {
  // See if qts support the users desired output format
  UserInterface &ui = Application::GetUserInterface();
  iString format = ui.GetString("FORMAT");
  format.DownCase();

  iString mode = ui.GetString("MODE");

  QList<QByteArray> list = QImageWriter::supportedImageFormats();
  QList<QByteArray>::Iterator it = list.begin();
  bool supported = false;
  while (it != list.end()) {
    if (*it == QString(format.c_str())) supported = true;
    ++it;
  }

  if (!supported) {
    string msg = "The installation of Trolltech/Qt does not support ";
    msg += "your selected format ["+format+"]";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Setup the required extension and world file
  string extension(""),world("");
  if (format == "png") {
    extension = "png";
    world = "pgw";
  }
  else if (format == "jpeg") {
    extension = "jpg";
    world = "jgw";
  }
  else if (format == "tiff") {
    extension = "tif";
    world = "tfw";
  }
  else if (format == "gif") {
    extension = "gif";
    world = "gfw";
  }
  else if (format == "bmp") {
    extension = "bmp";
    world = "bpw";
  }

  // Open the input cube
  ProcessExport p;

  // Cubes with less than three bands will be greyscale
  if (mode == "GRAYSCALE") {
    Cube *icube = p.SetInputCube("FROM", Isis::OneBand);
    checkDataSize(icube->Lines(),icube->Samples(),mode);
    qimage = new QImage(icube->Samples(),icube->Lines(), QImage::Format_Indexed8);
    qimage->setNumColors(256);
    QVector<QRgb> colors;
    //  create the color table (black = 0 to white = 255)
    for (int i = 0; i<256; i++) {
      colors.push_back(qRgb(i,i,i));
    }
    qimage->setColorTable(colors);
    p.SetInputRange();
    p.SetOutputRange(1.0,255.0);
    p.SetOutputNull(0.0);
    p.StartProcess(toGreyscaleImage);
  }
  //  If RGB, create a 24-bit color image
  //     (1st band -> red, 2nd band -> green, 3rd band -> blue)
  else if (mode == "RGB") {
    Cube *redcube = p.SetInputCube("RED", Isis::OneBand);
    checkDataSize(redcube->Lines(),redcube->Samples(),mode);
    p.SetInputCube("GREEN", Isis::OneBand);
    p.SetInputCube("BLUE", Isis::OneBand);
    qimage = new QImage(redcube->Samples(),redcube->Lines(),QImage::Format_RGB32);
    if (ui.GetString("STRETCH") == "MANUAL") {
      p.SetInputRange(ui.GetDouble("RMIN"), ui.GetDouble("RMAX"), 0);
      p.SetInputRange(ui.GetDouble("GMIN"), ui.GetDouble("GMAX"), 1);
      p.SetInputRange(ui.GetDouble("BMIN"), ui.GetDouble("BMAX"), 2);
    }
    else {
      p.SetInputRange();
      ui.Clear("MINIMUM");
      ui.Clear("MAXIMUM");
      ui.PutDouble("RMIN", p.GetInputMinimum(0));
      ui.PutDouble("RMAX", p.GetInputMaximum(0));
      ui.PutDouble("GMIN", p.GetInputMinimum(1));
      ui.PutDouble("GMAX", p.GetInputMaximum(1));
      ui.PutDouble("BMIN", p.GetInputMinimum(2));
      ui.PutDouble("BMAX", p.GetInputMaximum(2));
    }
    p.SetOutputRange(1.0,255.0);
    p.SetOutputNull(0.0);
    p.StartProcess(toRGBImage);
  }
  else if (mode == "ARGB") {
    Cube *alpha = p.SetInputCube("ALPHA", Isis::OneBand);
    checkDataSize(alpha->Lines(),alpha->Samples(),mode);
    p.SetInputCube("RED", Isis::OneBand);
    p.SetInputCube("GREEN", Isis::OneBand);
    p.SetInputCube("BLUE", Isis::OneBand);
    qimage = new QImage(alpha->Samples(),alpha->Lines(),QImage::Format_ARGB32);
    if (ui.GetString("STRETCH") == "MANUAL") {
      p.SetInputRange(ui.GetDouble("AMIN"), ui.GetDouble("AMAX"), 0);
      p.SetInputRange(ui.GetDouble("RMIN"), ui.GetDouble("RMAX"), 1);
      p.SetInputRange(ui.GetDouble("GMIN"), ui.GetDouble("GMAX"), 2);
      p.SetInputRange(ui.GetDouble("BMIN"), ui.GetDouble("BMAX"), 3);
    }
    else {
      p.SetInputRange();
      ui.Clear("MINIMUM");
      ui.Clear("MAXIMUM");
      ui.PutDouble("AMIN", p.GetInputMinimum(0));
      ui.PutDouble("AMAX", p.GetInputMaximum(0));
      ui.PutDouble("RMIN", p.GetInputMinimum(1));
      ui.PutDouble("RMAX", p.GetInputMaximum(1));
      ui.PutDouble("GMIN", p.GetInputMinimum(2));
      ui.PutDouble("GMAX", p.GetInputMaximum(2));
      ui.PutDouble("BMIN", p.GetInputMinimum(3));
      ui.PutDouble("BMAX", p.GetInputMaximum(3));
    }
    p.SetOutputRange(1.0,255.0);
    p.SetOutputNull(0.0);
    p.StartProcess(toARGBImage);
  }

  // Get the name of the file and write it
  Filename fname(ui.GetFilename("TO"));
  fname.AddExtension(extension);
  string filename(fname.Expanded());

  // The return status is wrong for JPEG images, so the code will always
  //   continue.
  if(!qimage->save(filename.c_str(),format.c_str(),100)) {
    delete qimage;
    iString err = "Unable to save [";
    err += filename.c_str();
    err += "] to the disk";
    throw iException::Message(iException::User, err, _FILEINFO_);
  }

  // Create a world file if it has a map projection
  fname.RemoveExtension();
  fname.AddExtension(world);
  p.CreateWorldFile(fname.Expanded());
  p.EndProcess();

  delete qimage;
}

// Write a line of data to the greyscale qimage object
void toGreyscaleImage (Buffer &in) {
  // Loop for each column and load the pixel from in[i] which will
  // be in the range of [0,255]
  for (int i=0; i<in.size(); i++) {
    int dn = (int) in[i];
    if (dn < 0) dn = 0;
    if (dn > 255) dn = 255;
    qimage->setPixel(i, in.Line()-1, dn);
  }
}

// Check to see if the QImage will be larger than 4GB
// error if it will be
void checkDataSize (Isis::BigInt line, Isis::BigInt samp, iString mode){
  Isis::BigInt maxSize = (Isis::BigInt)4*1024*1024*1024;
  Isis::BigInt size = 0;
  if(mode == "GRAYSCALE") {
    size = line*samp;
  } else if(mode == "RGB") {
    size = (line*samp)*3;
  } else if(mode == "ARGB") {
    size = (line*samp)*4;
  }
  if (size >= maxSize) {
    double inGB = (double)size / (1024*1024*1024);
    string msg = "Cube exceeds max size of 4GB. Qimage cannot support ";
    msg += "that much raw data. Your cube is "+(iString)inGB+" GB.";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }
  return;
}

// Write a line of data to the rgb qimage object
void toRGBImage (vector<Buffer *> &in) {
  Buffer &red = *in[0];
  Buffer &green = *in[1];
  Buffer &blue = *in[2];

  // Set magick pointer to the desired row and construct a Color
  QRgb *line = (QRgb *) qimage->scanLine(red.Line()-1);

  // Loop for each column and load the pixel from in[i] which will
  // be in the range of [0,255]
  for (int i=0; i<red.size(); i++) {
    int redDN = (int)red[i];
    if (redDN < 0) redDN = 0;
    else if (redDN > 255) redDN = 255;

    int greenDN = (int)green[i];
    if (greenDN < 0) greenDN = 0;
    else if (greenDN > 255) greenDN = 255;

    int blueDN = (int)blue[i];
    if (blueDN < 0) blueDN = 0;
    else if (blueDN > 255) blueDN = 255;

    line[i] = qRgb(redDN, greenDN, blueDN);
  }
}

// Write a line of data to the argb qimage object
void toARGBImage (vector<Buffer *> &in) {
  Buffer &alpha = *in[0];
  Buffer &red = *in[1];
  Buffer &green = *in[2];
  Buffer &blue = *in[3];

  // Set magick pointer to the desired row and construct a Color
  QRgb *line = (QRgb *) qimage->scanLine(red.Line()-1);

  // Loop for each column and load the pixel from in[i] which will
  // be in the range of [0,255]
  for (int i=0; i<red.size(); i++) {
    int redDN = (int)red[i];
    if (redDN < 0) redDN = 0;
    else if (redDN > 255) redDN = 255;

    int greenDN = (int)green[i];
    if (greenDN < 0) greenDN = 0;
    else if (greenDN > 255) greenDN = 255;

    int blueDN = (int)blue[i];
    if (blueDN < 0) blueDN = 0;
    else if (blueDN > 255) blueDN = 255;

    int alphaDN = (int)alpha[i];
    if (alphaDN < 0) alphaDN = 0;
    else if (alphaDN > 255) alphaDN = 255;

    line[i] = qRgba(redDN, greenDN, blueDN, alphaDN);
  }
}