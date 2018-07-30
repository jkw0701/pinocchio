//
// Copyright (c) 2015-2018 CNRS
//
// This file is part of Pinocchio
// Pinocchio is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// Pinocchio is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Lesser Public License for more details. You should have
// received a copy of the GNU Lesser General Public License along with
// Pinocchio If not, see
// <http://www.gnu.org/licenses/>.

#ifndef __se3_frames_hxx__
#define __se3_frames_hxx__

#include "pinocchio/algorithm/kinematics.hpp"
#include "pinocchio/algorithm/jacobian.hpp"
#include "pinocchio/algorithm/check.hpp"

namespace se3 
{
  
  
  template<typename JointCollection>
  inline void framesForwardKinematics(const ModelTpl<JointCollection> & model,
                                      DataTpl<JointCollection> & data)
  {
    assert(model.check(data) && "data is not consistent with model.");
    
    typedef ModelTpl<JointCollection> Model;
    typedef typename Model::FrameIndex FrameIndex;
    typedef typename Model::JointIndex JointIndex;
    
    // The following for loop starts by index 1 because the first frame is fixed
    // and corresponds to the universe.s
    for (FrameIndex i=1; i < (FrameIndex) model.nframes; ++i)
    {
      const Frame & frame = model.frames[i];
      const JointIndex & parent = frame.parent;
      if (frame.placement.isIdentity())
        data.oMf[i] = data.oMi[parent];
      else
        data.oMf[i] = data.oMi[parent]*frame.placement;
    }
  }
  
  template<typename JointCollection, typename ConfigVectorType>
  inline void framesForwardKinematics(const ModelTpl<JointCollection> & model,
                                      DataTpl<JointCollection> & data,
                                      const Eigen::MatrixBase<ConfigVectorType> & q)
  {
    assert(model.check(data) && "data is not consistent with model.");
    
    forwardKinematics(model, data, q);
    framesForwardKinematics(model, data);
  }
  
  template<typename JointCollection, typename Matrix6xLike>
  inline void getFrameJacobian(const ModelTpl<JointCollection> & model,
                               const DataTpl<JointCollection> & data,
                               const typename ModelTpl<JointCollection>::FrameIndex frame_id,
                               const ReferenceFrame rf,
                               const Eigen::MatrixBase<Matrix6xLike> & J)
  {
    assert(J.rows() == 6);
    assert(J.cols() == model.nv);
    assert(data.J.cols() == model.nv);
    assert(model.check(data) && "data is not consistent with model.");
    
    typedef ModelTpl<JointCollection> Model;
    typedef DataTpl<JointCollection> Data;
    typedef typename Model::JointIndex JointIndex;
    
    const Frame & frame = model.frames[frame_id];
    const JointIndex & joint_id = frame.parent;
    if(rf == WORLD)
    {
      getJointJacobian(model,data,joint_id,WORLD,EIGEN_CONST_CAST(Matrix6xLike,J));
      return;
    }
    
    if(rf == LOCAL)
    {
      Matrix6xLike & J_ = EIGEN_CONST_CAST(Matrix6xLike,J);
      const typename Data::SE3 & oMframe = data.oMf[frame_id];
      const int colRef = nv(model.joints[joint_id])+idx_v(model.joints[joint_id])-1;
      
      for(Eigen::DenseIndex j=colRef;j>=0;j=data.parents_fromRow[(size_t) j])
      {
        J_.col(j) = oMframe.actInv(Motion(data.J.col(j))).toVector(); // TODO: use MotionRef
      }
      return;
    }
  }
  
  template<typename JointCollection, typename Matrix6xLike>
  inline void getFrameJacobian(const ModelTpl<JointCollection> & model,
                               const DataTpl<JointCollection> & data,
                               const typename ModelTpl<JointCollection>::FrameIndex frame_id,
                               const Eigen::MatrixBase<Matrix6xLike> & J)
  {
    getFrameJacobian(model,data,frame_id,LOCAL,EIGEN_CONST_CAST(Matrix6xLike,J));
  }

} // namespace se3

#endif // ifndef __se3_frames_hxx__
