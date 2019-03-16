#pragma once
#include <Eigen/Core>

#include "g2o/core/base_vertex.h"
#include "g2o/core/base_unary_edge.h"

using Eigen::MatrixXd;
using namespace std;
#include <iostream>
/**
* \brief the params, a, b, and lambda for a * exp(-lambda * t) + b
*/
class VertexParams : public g2o::BaseVertex<3, Eigen::Vector3d>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
	VertexParams()
	{
	}

	virtual bool read(std::istream& /*is*/)
	{
		cerr << __PRETTY_FUNCTION__ << " not implemented yet" << endl;
		return false;
	}

	virtual bool write(std::ostream& /*os*/) const
	{
		cerr << __PRETTY_FUNCTION__ << " not implemented yet" << endl;
		return false;
	}

	virtual void setToOriginImpl()
	{
		cerr << __PRETTY_FUNCTION__ << " not implemented yet" << endl;
	}

	virtual void oplusImpl(const double* update)
	{
		Eigen::Vector3d::ConstMapType v(update);
		_estimate += v;
	}
};


/**
* \brief measurement for a point on the curve
*
* Here the measurement is the point which is lies on the curve.
* The error function computes the difference between the curve
* and the point.
*/
class EdgePointOnCurve : public g2o::BaseUnaryEdge<1, Eigen::Vector2d, VertexParams>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		EdgePointOnCurve()
	{
	}
	virtual bool read(std::istream& /*is*/)
	{
		cerr << __PRETTY_FUNCTION__ << " not implemented yet" << endl;
		return false;
	}
	virtual bool write(std::ostream& /*os*/) const
	{
		cerr << __PRETTY_FUNCTION__ << " not implemented yet" << endl;
		return false;
	}

	void computeError()
	{
		const VertexParams* params = static_cast<const VertexParams*>(vertex(0));
		const double& a = params->estimate()(0);
		const double& b = params->estimate()(1);
		const double& lambda = params->estimate()(2);
		double fval = a * exp(-lambda * measurement()(0)) + b;
		_error(0) = fval - measurement()(1);
	}
};
