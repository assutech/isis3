#include "CubeManager.h"
#include "Cube.h"
#include "Filename.h"

namespace Isis {
  CubeManager CubeManager::p_instance;

  /**
   * This method opens a cube. If the cube is already opened, this method will 
   * return the cube from memory. The CubeManager class retains ownership of this 
   * cube pointer, so do not close the cube, destroy the pointer, or otherwise 
   * modify the cube object or pointer such that another object using them would 
   * fail. This method does not guarantee you are the only one with this pointer, 
   * nor is it recommended to keep this pointer out of a local (method) scope. 
   * 
   * @param cubeFilename The filename of the cube you wish to open
   * 
   * @return Cube* A pointer to the cube object that CubeManager retains ownership 
   *         to and may delete at any time
   */
  Cube *CubeManager::OpenCube(const std::string &cubeFilename) {
    QString fileName((iString)Filename(cubeFilename).Expanded());
    QMap<QString, Cube *>::iterator searchResult = p_cubes.find(fileName);

    if(searchResult == p_cubes.end()) {
      p_cubes.insert(fileName, new Cube());
      searchResult = p_cubes.find(fileName);
      (*searchResult)->Open(fileName.toStdString());
    }

    return (*searchResult);
  }

  /**
   * This method removes a cube from memory, if it exists. If the cube is not 
   * loaded into memory, nothing happens. This will cause any pointers to this 
   * cube, obtained via OpenCube, to be invalid. 
   * 
   * @param cubeFilename The filename of the cube to remove from memory
   */
  void CubeManager::CleanCubes(const std::string &cubeFilename) {
    QString fileName((iString)Filename(cubeFilename).Expanded());
    QMap<QString, Cube *>::iterator searchResult = p_cubes.find(fileName);

    if(searchResult == p_cubes.end()) {
      return;
    }

    (*searchResult)->Close();
    delete *searchResult;
    p_cubes.erase(searchResult);
  }

  /**
   * This method removes all cubes from memory. All pointers returned via OpenCube
   * will be invalid.
   */
  void CubeManager::CleanCubes() {
    QMap<QString, Cube *>::iterator pos = p_cubes.begin();

    while(pos != p_cubes.end()) {
      (*pos)->Close();
      delete *pos;
      pos ++;
    }

    p_cubes.clear();
  }

  /**
   * This is the CubeManager destructor. This method calls CleanCubes().
   * 
   */
  CubeManager::~CubeManager() {
    CleanCubes();
  }
}