//
// Copyright (c) 2019 CNRS INRIA
//

#include "pinocchio/bindings/python/algorithm/algorithms.hpp"
#include "pinocchio/algorithm/cholesky.hpp"

namespace pinocchio
{
  namespace python
  {
    
    struct CholeskyScope {};
    
    void exposeCholesky()
    {
      using namespace Eigen;
      using namespace pinocchio::cholesky;
      
      bp::scope main_scope;
      std::string submodule_name(bp::extract<const char*>(main_scope.attr("__name__")));
      submodule_name.append(".cholesky");
      
      bp::object submodule(bp::borrowed(PyImport_AddModule(submodule_name.c_str())));
      main_scope.attr("cholesky") = submodule;

      {
        // using the cholesky scope
        bp::scope current_scope = submodule;
        
        bp::def("decompose",
                &decompose<double,0,JointCollectionDefaultTpl>,
                bp::args("Model","Data"),
                "Computes the Cholesky decomposition of the joint space inertia matrix M contained in data.\n"
                "The upper triangular part of data.M should have been filled first by calling crba, or any related algorithms.",
                bp::return_value_policy<bp::return_by_value>());
        
        bp::def("solve",
                &solve<double,0,JointCollectionDefaultTpl,VectorXd>,
                bp::args("Model","Data","v"),
                "Returns the solution \f$x\f$ of \f$ M x = y \f$ using the Cholesky decomposition stored in data given the entry \f$ y \f$.",
                bp::return_value_policy<bp::return_by_value>());
      }
      
    }
    
  } // namespace python
} // namespace pinocchio
