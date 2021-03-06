#include "Calculator.h"
#include "Preference.h"
#include <float.h>

using namespace std;
using namespace Isis;

int main(int argc, char *argv[])
{
  Isis::Preference::Preferences(true);
  Isis::Calculator c;
  vector<double> v1;
  vector<double> v2;
  vector<double> v3;
  vector<double> v4;
  double scalar = 2.0;
  double big = DBL_EPSILON;
  
  v1.push_back(1);
  v1.push_back(2);
  v1.push_back(3);

  v2.push_back(4);
  v2.push_back(5);
  v2.push_back(6);

  v4.push_back(2);
  v4.push_back(2);
  v4.push_back(2);

  cout << "-------------------------------------------------------" << endl;

  c.Push(v1);
  cout << "v1: ";
  c.PrintTop();
  c.Push(v2);
  cout << "v2: ";
  c.PrintTop();
  c.Push(v4);
  cout << "v4: ";
  c.PrintTop();
  c.Push(scalar);
  cout << "Scalar: ";
  c.PrintTop();
  c.Push(big);
  cout << "Big: ";
  c.PrintTop();
  c.Clear();
  
  cout << "-------------------------------------------------------" << endl;
  
  c.Push(v1);
  c.Negative();
  cout << "Negative (-v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v2);
  c.Multiply();
  cout << "Multiply (v1*v2): ";
  c.PrintTop();
  c.Clear();
  
  c.Push(v1);
  c.Push(scalar);
  c.Multiply();
  cout << "Multiply (v1*scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(big);
  c.Push(big);
  c.Multiply();
  cout << "Multiply (big*big): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v2);
  c.Add();
  cout << "Add (v1+v2): ";
  c.PrintTop();
  c.Clear();
  
  c.Push(v1);
  c.Push(scalar);
  c.Add();
  cout << "Add (v1+scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v2);
  c.Subtract();
  cout << "Subtract (v1-v2): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(scalar);
  c.Subtract();
  cout << "Subtract (v1-scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v2);
  c.Divide();
  cout << "Divide (v1/v2): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(scalar);
  c.Divide();
  cout << "Divide (v1/scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(1.0);
  c.Push(0.0);
  c.Divide();
  cout << "Divide (1/0): ";
  c.PrintTop();
  c.Clear();
  
  c.Push(-1.0);
  c.Push(0.0);
  c.Divide();
  cout << "Divide (-1/0): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(scalar);
  c.Exponent();
  cout << "Exponent (v1^scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.SquareRoot();
  cout << "Square Root (sqrt[v1]): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Negative();
  c.AbsoluteValue();
  cout << "Absolute value (-v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Log();
  cout << "Log (log[v1]): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Log10();
  cout << "Log10 (log10[v1]): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(scalar);
  c.LeftShift();
  cout << "LeftShift (v1<<scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(scalar);
  c.RightShift();
  cout << "RightShift (v1>>scalar): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Minimum();
  cout << "Min (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Maximum();
  cout << "Max (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.GreaterThan();
  cout << "GreaterThan (v1>v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.LessThan();
  cout << "LessThan (v1<v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.Equal();
  cout << "Equal (v1==v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.LessThanOrEqual();
  cout << "LessThanOrEqual (v1<=v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.GreaterThanOrEqual();
  cout << "GreaterThanOrEqual (v1>=v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.NotEqual();
  cout << "NotEqual (v1!=v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Sine();
  cout << "Sine (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Cosine();
  cout << "Cosine (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Tangent();
  cout << "Tangent (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Secant();
  cout << "Secant (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Cosecant();
  cout << "Cosecant (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Cotangent();
  cout << "Cotangent (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Arcsine();
  cout << "Arcsine (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Arccosine();
  cout << "Arccosine (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Arctangent();
  cout << "Arctangent (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.SineH();
  cout << "SineH (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.CosineH();
  cout << "CosineH (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.TangentH();
  cout << "TangentH (v1): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.Modulus();
  cout << "Modulus (v1%v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.And();
  cout << "Bitwise And (v1,v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(v1);
  c.Push(v4);
  c.Or();
  cout << "Bitwise Or (v1,v4): ";
  c.PrintTop();
  c.Clear();

  c.Push(1.0);
  c.Negative();
  c.SquareRoot();
  cout << "Square Root(-1): ";
  c.PrintTop();
  c.Clear();

  c.Push(0.0);
  c.Log();
  cout << "Log(of 0): ";
  c.PrintTop();
  c.Clear();

  std::cout << "-------------------------------------------------------" << std::endl;
  try {
    c.Push(v1);
    c.Push(4);
    c.LeftShift();
    cout << "LeftShift(too far): ";
    c.PrintTop();
    c.Clear();
  }
  catch(Isis::iException &e) {
    e.Report(false);
  }

  try {
    c.Push(v1);
    c.Push(4);
    c.RightShift();
    cout << "RightShift(too far): ";
    c.PrintTop();
    c.Clear();
  }
  catch(Isis::iException &e) {
    e.Report(false);
  }

  try {
    c.Push(v1);
    c.Push(v3);
    c.Add();
  }
  catch(Isis::iException &e) {
    e.Report(false);
  }

  try {
    c.Push(v1);
    c.Add();
    c.Clear();
  }
  catch(Isis::iException &e) {
    e.Report(false);
  }
}
